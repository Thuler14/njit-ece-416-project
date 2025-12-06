/*
 * ================================================================
 *  Module: control
 *  Purpose: Handles the control logic for the shower controller.
 *           Receives setpoint and run-state commands from the UI Unit,
 *           reads temperature sensors, and drives the servo valves
 *           to maintain the desired outlet temperature.
 *
 *  Hardware:
 *    - ESP32 Control Unit (WIFI_STA)
 *    - 3× DS18B20 sensors (Hot, Cold, Outlet)
 *    - 2× MG996R servos (Hot, Cold)
 *    - 1x YF-S201 flow sensor (Outlet)
 *
 *  Communication:
 *    - Receives setpoint and run-state data from UI Unit via ESP-NOW
 *    - Sends ACK/ERR responses
 *    - Optional encryption using PMK/LMK
 * ================================================================
 */

#include <stdio.h>

#include "../common/config.h"
#include "communication.h"
#include "config.h"
#include "pid.h"
#include "temperature.h"
#include "valve_mix.h"

static constexpr uint16_t LOOP_DELAY_MS = 12;
static PID pi(PID_KP, PID_KI, PID_KD, PID_OUT_MIN, PID_OUT_MAX);

static float setpointF = SETPOINT_DEFAULT_F;
static bool runFlag = false;
static unsigned long lastLoopMs = 0;
static bool printedCsvHeader = false;

enum class FaultCode : uint8_t {
  None = 0,
  LinkLoss,
  OutletSensorFault,
  OutletOutOfBounds,
};

static FaultCode activeFault = FaultCode::None;

static void enterSafeState(const char* reason) {
  if (reason != nullptr) {
    Serial.println(reason);
  }
  valveMixCloseAll();
  pi.reset();
}

void setup() {
  Serial.begin(115200);
  delay(100);

  if (!commInit())
    while (1) delay(1000);

  if (!temperatureInit()) {
    Serial.println("TEMP ERROR: No DS18B20 sensors detected");
  }

  valveMixInit();
  valveMixCloseAll();
}

void loop() {
  (void) temperatureService();

  const unsigned long nowMs = millis();

  CommCommand cmd{};
  if (commPollCommand(cmd)) {
    if (cmd.lastOk) {
      setpointF = constrain(cmd.setpointF, SETPOINT_MIN_F, SETPOINT_MAX_F);
      runFlag = cmd.runFlag;

      Serial.printf("CTRL<-UI setpoint=%.1fF run=%s seq=%lu\n",
                    setpointF,
                    runFlag ? "ON" : "OFF",
                    (unsigned long) cmd.lastSeq);
    } else {
      Serial.printf("CTRL<-UI RX failed\n");
    }
  }

  FaultCode detectedFault = FaultCode::None;
  const char* faultMsg = nullptr;
  char boundsMsg[96];

  const unsigned long lastRxMs = commLastRxMs();
  if (runFlag &&
      (lastRxMs == 0 || (unsigned long) (nowMs - lastRxMs) > COMM_LINK_TIMEOUT_MS)) {
    detectedFault = FaultCode::LinkLoss;
    faultMsg = "LINK ERROR: No UI command for 2s → closing valves";
    runFlag = false;
    commMarkLinkLost();
  }

  const TemperatureReading& outlet = temperatureGetReading(TempSensor::OUTLET);
  if (detectedFault == FaultCode::None && runFlag) {
    if (!outlet.present || !outlet.valid) {
      detectedFault = FaultCode::OutletSensorFault;
      faultMsg = "TEMP ERROR: Outlet sensor fault → closing valves";
      runFlag = false;
    } else if ((outlet.filteredF < OUTLET_MIN_PLAUSIBLE_F) ||
               (outlet.filteredF > OUTLET_MAX_PLAUSIBLE_F)) {
      detectedFault = FaultCode::OutletOutOfBounds;
      snprintf(boundsMsg,
               sizeof(boundsMsg),
               "TEMP ERROR: Outlet %.1fF out of bounds (%.0f-%.0fF) → closing valves",
               outlet.filteredF,
               OUTLET_MIN_PLAUSIBLE_F,
               OUTLET_MAX_PLAUSIBLE_F);
      faultMsg = boundsMsg;
      runFlag = false;
    }
  }

  if (detectedFault != FaultCode::None) {
    if (activeFault != detectedFault || detectedFault == FaultCode::OutletOutOfBounds) {
      enterSafeState(faultMsg);
    } else {
      enterSafeState(nullptr);
    }
    activeFault = detectedFault;
    delay(LOOP_DELAY_MS);
    return;
  }

  if (!runFlag) {
    activeFault = FaultCode::None;
    enterSafeState(nullptr);
    delay(LOOP_DELAY_MS);
    return;
  }

  activeFault = FaultCode::None;
  const float dtSec =
      lastLoopMs ? (nowMs - lastLoopMs) / 1000.0f : LOOP_DELAY_MS / 1000.0f;
  lastLoopMs = nowMs;

  const float outletTempF = outlet.filteredF;
  const float errorF = setpointF - outletTempF;
  const float ratio = pi.update(errorF, dtSec);
  applyMixRatio(ratio);

  if (PID_LOG_CSV) {
    if (!printedCsvHeader) {
      Serial.println("t_ms,out_f,set_f,error_f,ratio");
      printedCsvHeader = true;
    }
    Serial.printf("%lu,%.2f,%.2f,%.2f,%.2f\n",
                  (unsigned long) nowMs,
                  outletTempF,
                  setpointF,
                  errorF,
                  ratio);
  } else {
    Serial.printf("OUT=%.2fF / SET=%.2fF | error=%.2fF | ratio=%.2f\n",
                  outletTempF, setpointF, errorF, ratio);
  }

  delay(LOOP_DELAY_MS);
}
