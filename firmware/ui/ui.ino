/*
 * ================================================================
 *  Module: ui
 *  Purpose: Handles the user interface for the shower controller.
 *           Reads button inputs, updates the OLED display, and
 *           transmits setpoint and run-state commands to the Control Unit.
 *
 *  Hardware:
 *    - SSD1309 128×64 OLED (SPI, U8g2 library)
 *    - 5 active-low pushbuttons (▲ ▼ ● A B)
 *
 *  Communication:
 *    - Sends setpoint and run-state data to Control Unit via ESP-NOW
 *    - Optional encryption using PMK/LMK
 * ================================================================
 */

#include "../common/config.h"
#include "buttons.h"
#include "communication.h"
#include "config.h"
#include "display.h"

static float setpointF = SETPOINT_DEFAULT_F;  // current setpoint
static float stepF = SETPOINT_STEP_F;         // current setpoint step
static bool runFlag = false;                  // true=ON, false=OFF
static bool setpointDirty = false;            // true when setpoint changed but not yet sent
static unsigned long lastSetpointEditMs = 0;  // last time the user adjusted setpoint
static bool flowOverlayLatched = false;       // true while flow overlay is active

// Map UI + comm status into DisplayState and draw on OLED
static void updateDisplay(const CommStatus& st, bool showingFlow) {
  const bool showingSetpoint = setpointDirty || st.pending;
  DisplayState ds{};
  ds.setpointF = setpointF;
  ds.outletTempF = st.outletTempF;
  ds.flowLpm = st.flowLpm;
  ds.stepF = stepF;
  ds.showingSetpoint = showingSetpoint;
  ds.showingFlow = showingFlow;
  ds.outletValid = st.outletValid;
  ds.flowValid = st.flowValid;
  ds.runFlag = runFlag;
  ds.txDoneCount = st.txCount;
  ds.lastResultOk = st.lastOk;
  ds.pending = st.pending || setpointDirty;  // show pending when unsent edits exist
  displayDraw(ds);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  if (!commInit())
    while (1) delay(1000);

  displayInit();
  buttonsInit();

  // Initial draw
  CommStatus st{};
  commGetStatus(st);
  updateDisplay(st, /*showingFlow=*/false);
}

void loop() {
  const unsigned long nowMs = millis();

  ButtonsEvents ev{};
  buttonsPoll(ev);

  bool displayChanged = false;
  bool txTriggered = false;
  bool sendNow = false;
  bool flowOverlayActive = flowOverlayLatched;

  auto markSetpointDirty = [&]() {
    setpointDirty = true;
    lastSetpointEditMs = nowMs;
  };

  if (ev.chordStepLong) {
    if (stepF <= 0.5f)
      stepF = 1.0f;
    else if (stepF < 1.5f)
      stepF = 2.0f;
    else
      stepF = 0.5f;
    displayChanged = true;
  } else if (ev.upClick || ev.upRepeat) {
    setpointF += stepF;
    markSetpointDirty();
    displayChanged = true;
  } else if (ev.downClick || ev.downRepeat) {
    setpointF -= stepF;
    markSetpointDirty();
    displayChanged = true;
  } else if (ev.aLong) {
    setpointF = SETPOINT_PRESET_A_F;
    markSetpointDirty();
    displayChanged = true;
  } else if (ev.bLong) {
    setpointF = SETPOINT_PRESET_B_F;
    markSetpointDirty();
    displayChanged = true;
  } else if (ev.okLong) {
    runFlag = !runFlag;
    sendNow = true;  // run/stop toggles send immediately
    displayChanged = true;
  } else if (ev.chordFlowLong) {
    flowOverlayLatched = true;
    flowOverlayActive = true;
    displayChanged = true;
  }

  auto anyNonFlowEvent = [&]() {
    return ev.chordStepLong || ev.upClick || ev.upDblClick || ev.upLong || ev.upRepeat ||
           ev.downClick || ev.downDblClick || ev.downLong || ev.downRepeat ||
           ev.okClick || ev.okDblClick || ev.okLong || ev.okRepeat ||
           ev.aClick || ev.aDblClick || ev.aLong || ev.aRepeat ||
           ev.bClick || ev.bDblClick || ev.bLong || ev.bRepeat;
  };

  if (flowOverlayLatched && anyNonFlowEvent()) {
    flowOverlayLatched = false;
    flowOverlayActive = false;
    displayChanged = true;
  }

  if (setpointDirty) {
    setpointF = constrain(setpointF, SETPOINT_MIN_F, SETPOINT_MAX_F);
  }

  // Heartbeat: keep link alive while running
  commHeartbeatTick(nowMs);

  // Send setpoint after a quiet period with no edits
  if (setpointDirty && (nowMs - lastSetpointEditMs) >= UI_SETPOINT_SEND_DELAY_MS) {
    sendNow = true;
  }

  if (sendNow) {
    bool ok = commSendSetpoint(setpointF, runFlag);
    txTriggered = true;
    if (setpointDirty) {
      setpointDirty = !ok;
      if (!ok) lastSetpointEditMs = nowMs;
    } else if (!ok) {
      // retry after a short pause if immediate send failed
      setpointDirty = true;
      lastSetpointEditMs = nowMs;
    }
  }

  // Check if comm status changed (pending, ACK, or TX fail)
  CommStatus st{};
  bool statusChanged = commPollStatus(st);

  // Redraw and log when UI state or comm status changes
  if (displayChanged || statusChanged || txTriggered || setpointDirty) {
    if (!statusChanged) commGetStatus(st);

    flowOverlayActive = flowOverlayLatched;
    updateDisplay(st, flowOverlayActive);

    if (txTriggered || statusChanged) {
      if (st.pending || setpointDirty)
        Serial.println("UI->CTRL TX pending");
      else if (!st.lastOk)
        Serial.println("UI<-CTRL TX failed");
      else
        Serial.printf("UI->CTRL setpoint=%.1fF run=%s seq=%lu\n",
                      setpointF,
                      runFlag ? "ON" : "OFF",
                      (unsigned long) st.lastSeq);
    }
  }

  delay(12);  // small delay to avoid busy loop
}
