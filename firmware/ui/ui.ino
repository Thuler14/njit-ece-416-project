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

// Map UI + comm status into DisplayState and draw on OLED
static void updateDisplay(const CommStatus& st) {
  DisplayState ds{};
  ds.setpointF = setpointF;
  ds.stepF = stepF;
  ds.runFlag = runFlag;
  ds.txDoneCount = st.txCount;
  ds.lastResultOk = st.lastOk;
  ds.pending = st.pending;
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
  updateDisplay(st);
}

void loop() {
  ButtonsEvents ev{};
  buttonsPoll(ev);

  bool stateChanged = false;
  bool displayChanged = false;

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
    stateChanged = true;
    displayChanged = true;
  } else if (ev.downClick || ev.downRepeat) {
    setpointF -= stepF;
    stateChanged = true;
    displayChanged = true;
  } else if (ev.aLong) {
    setpointF = SETPOINT_PRESET_A_F;
    stateChanged = true;
    displayChanged = true;
  } else if (ev.bLong) {
    setpointF = SETPOINT_PRESET_B_F;
    stateChanged = true;
    displayChanged = true;
  } else if (ev.okLong) {
    runFlag = !runFlag;
    stateChanged = true;
    displayChanged = true;
  }

  // Send new state (marks TX as pending in communication layer)
  if (stateChanged) {
    setpointF = constrain(setpointF, SETPOINT_MIN_F, SETPOINT_MAX_F);
    commSendSetpoint(setpointF, runFlag);
  }

  // Heartbeat: keep link alive while running
  commHeartbeatTick(millis());

  // Check if comm status changed (pending, ACK, or TX fail)
  CommStatus st{};
  bool statusChanged = commPollStatus(st);

  // Redraw and log when UI state or comm status changes
  if (stateChanged || displayChanged || statusChanged) {
    if (!statusChanged) commGetStatus(st);

    updateDisplay(st);

    if (stateChanged || statusChanged) {
      if (st.pending)
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
