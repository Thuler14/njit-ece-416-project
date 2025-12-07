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
#include "flow_sensor.h"
#include "pid.h"
#include "temperature.h"
#include "valve_mix.h"

static constexpr uint16_t LOOP_DELAY_MS = 12;
static constexpr uint16_t LOGGER_PERIOD_MS = 100;
static PID pi(PID_KP, PID_KI, PID_KD, PID_OUT_MIN, PID_OUT_MAX);

static float setpointF = SETPOINT_DEFAULT_F;
static bool runFlag = false;
static uint32_t lastOutletSampleMs = 0;
static bool loggerHeaderPrinted = false;
static float lastRatio = 0.0f;
static float lastU = 0.0f;
static void logCsvIfDue(unsigned long nowMs, const TemperatureReading& outlet, bool linkOk);
static bool estopPressed();

enum class FaultCode : uint8_t {
  None = 0,
  EStop,
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
  lastOutletSampleMs = 0;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  if (!commInit())
    while (1) delay(1000);

  pinMode(ESTOP_PIN, INPUT_PULLUP);

  if (!flowSensorInit()) {
    Serial.println("FLOW WARN: Flow sensor init failed");
  }

  if (!temperatureInit()) {
    Serial.println("TEMP ERROR: No DS18B20 sensors detected");
  }

  valveMixInit();
  valveMixCloseAll();
}

void loop() {
  (void) temperatureService();
  (void) flowSensorUpdate();

  const unsigned long nowMs = millis();
  const unsigned long lastRxMs = commLastRxMs();
  const bool linkOk =
      (lastRxMs != 0) && ((unsigned long) (nowMs - lastRxMs) <= COMM_LINK_TIMEOUT_MS);

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

  const bool estop = estopPressed();
  const TemperatureReading& outlet = temperatureGetReading(TempSensor::OUTLET);
  if (estop) {
    detectedFault = FaultCode::EStop;
    faultMsg = "E-STOP: switch active → closing valves";
    runFlag = false;
  } else {
    if (runFlag &&
        (lastRxMs == 0 || (unsigned long) (nowMs - lastRxMs) > COMM_LINK_TIMEOUT_MS)) {
      detectedFault = FaultCode::LinkLoss;
      faultMsg = "LINK ERROR: No UI command for 2s → closing valves";
      runFlag = false;
      commMarkLinkLost();
    }

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
  }

  if (detectedFault != FaultCode::None) {
    if (activeFault != detectedFault || detectedFault == FaultCode::OutletOutOfBounds) {
      enterSafeState(faultMsg);
    } else {
      enterSafeState(nullptr);
    }
    activeFault = detectedFault;
    logCsvIfDue(nowMs, outlet, linkOk);
    delay(LOOP_DELAY_MS);
    return;
  }

  if (!runFlag) {
    activeFault = FaultCode::None;
    enterSafeState(nullptr);
    logCsvIfDue(nowMs, outlet, linkOk);
    Serial.printf("RUN=OFF | OUT=%.2fF | SET=%.2fF | link=%s | flow=%.2f L/min\n",
                  outlet.filteredF,
                  setpointF,
                  linkOk ? "OK" : "LOST",
                  flowSensorGet().lpm);
    delay(LOOP_DELAY_MS);
    return;
  }

  activeFault = FaultCode::None;
  const uint32_t sampleMs = outlet.sampleMs;
  if (sampleMs == 0 || sampleMs == lastOutletSampleMs) {
    logCsvIfDue(nowMs, outlet, linkOk);
    delay(LOOP_DELAY_MS);
    return;
  }

  const float dtSec = (lastOutletSampleMs == 0)
                          ? (TEMP_LOOP_DT_MS / 1000.0f)
                          : (sampleMs - lastOutletSampleMs) / 1000.0f;
  lastOutletSampleMs = sampleMs;

  const float outletTempF = outlet.filteredF;
  const float errorF = setpointF - outletTempF;
  const float ratio = pi.update(errorF, dtSec);
  lastU = pi.lastOutput();
  lastRatio = ratio;
  applyMixRatio(ratio);

  if (PID_LOG_CSV) {
    logCsvIfDue(sampleMs, outlet, linkOk);
  } else {
    const FlowReading flow = flowSensorGet();
    Serial.printf("RUN=ON | OUT=%.2fF / SET=%.2fF | error=%.2fF | ratio=%.2f | flow=%.2f L/min | link=%s\n",
                  outletTempF,
                  setpointF,
                  errorF,
                  ratio,
                  flow.lpm,
                  linkOk ? "OK" : "LOST");
  }

  delay(LOOP_DELAY_MS);
}

static bool estopPressed() {
  return digitalRead(ESTOP_PIN) == LOW;
}

static void logCsvIfDue(unsigned long nowMs, const TemperatureReading& outlet, bool linkOk) {
  if (!PID_LOG_CSV) return;

  static unsigned long lastLogMs = 0;
  if (lastLogMs != 0 && (nowMs - lastLogMs) < LOGGER_PERIOD_MS) {
    return;
  }

  if (!loggerHeaderPrinted) {
    Serial.println("ms,setF,T_out_raw,T_out_filt,ratio,u,Kp,Ki,flow_lpm,link_ok");
    loggerHeaderPrinted = true;
  }

  const FlowReading flow = flowSensorGet();

  Serial.printf("%lu,%.1f,%.2f,%.2f,%.3f,%.3f,%.3f,%.3f,%.3f,%d\n",
                (unsigned long) nowMs,
                setpointF,
                outlet.rawF,
                outlet.filteredF,
                lastRatio,
                lastU,
                pi.getKp(),
                pi.getKi(),
                flow.lpm,
                linkOk ? 1 : 0);

  lastLogMs = nowMs;
}
