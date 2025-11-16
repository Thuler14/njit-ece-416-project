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
static void updateDisplay(float setpointF, bool runFlag, const CommStatus& st) {
  DisplayState ds{};
  ds.setpointF = setpointF;
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
  updateDisplay(setpointF, runFlag, st);
}

void loop() {
  ButtonsEvents ev{};
  const bool anyBtn = buttonsPoll(ev);  // returns true if any event latched
  bool updated = false;

  if (ev.chordStepLong) {
    if (stepF <= 0.5f)
      stepF = 1.0f;
    else if (stepF < 1.5f)
      stepF = 2.0f;
    else
      stepF = 0.5f;
    return;  // consume
  }

  if (ev.upClick || ev.upRepeat) setpointF += stepF, updated = true;
  if (ev.downClick || ev.downRepeat) setpointF -= stepF, updated = true;
  if (ev.aLong) setpointF = SETPOINT_PRESET_A_F, updated = true;
  if (ev.bLong) setpointF = SETPOINT_PRESET_B_F, updated = true;
  if (ev.okLong) runFlag = !runFlag, updated = true;

  // Send new state (marks TX as pending in communication layer)
  if (updated) {
    setpointF = constrain(setpointF, SETPOINT_MIN_F, SETPOINT_MAX_F);
    commSendSetpoint(setpointF, runFlag);
  }

  // Check if comm status changed (pending, ACK, or TX fail)
  CommStatus st{};
  bool statusChanged = commPollStatus(st);

  // Redraw and log when UI state or comm status changes
  if (updated || statusChanged) {
    if (!statusChanged) commGetStatus(st);

    updateDisplay(setpointF, runFlag, st);

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

  delay(12);  // small delay to avoid busy loop
}
