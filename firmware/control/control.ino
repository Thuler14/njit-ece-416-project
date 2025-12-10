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

#include <math.h>
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
static float lastHotRapidF = 0.0f;
static float lastColdRapidF = 0.0f;
static uint32_t lastHotRapidMs = 0;
static uint32_t lastColdRapidMs = 0;
static void logCsvIfDue(unsigned long nowMs, const TemperatureReading& outlet, bool linkOk);
static bool estopPressed();

enum class FaultCode : uint8_t {
  None = 0,
  EStop,
  LinkLoss,
  OutletSensorFault,
  OutletOutOfBounds,
  HotSensorFault,
  ColdSensorFault,
  HotOutOfBounds,
  ColdOutOfBounds,
  HotRapidChange,
  ColdRapidChange,
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
  const FlowReading flow = flowSensorGet();

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
  char hotBoundsMsg[96];
  char coldBoundsMsg[96];
  char rapidMsg[96];

  const bool estop = estopPressed();
  const TemperatureReading& outlet = temperatureGetReading(TempSensor::OUTLET);
  const TemperatureReading& hot = temperatureGetReading(TempSensor::HOT);
  const TemperatureReading& cold = temperatureGetReading(TempSensor::COLD);
  commUpdateOutletTemp(outlet.filteredF, outlet.present && outlet.valid, flow.lpm, flow.sampleMs != 0);
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
      if (!hot.present || !hot.valid) {
        detectedFault = FaultCode::HotSensorFault;
        faultMsg = "TEMP ERROR: Hot sensor fault → closing valves";
        runFlag = false;
      } else if (!cold.present || !cold.valid) {
        detectedFault = FaultCode::ColdSensorFault;
        faultMsg = "TEMP ERROR: Cold sensor fault → closing valves";
        runFlag = false;
      } else if (hot.filteredF < HOT_MIN_PLAUSIBLE_F || hot.filteredF > HOT_MAX_PLAUSIBLE_F) {
        detectedFault = FaultCode::HotOutOfBounds;
        snprintf(hotBoundsMsg,
                 sizeof(hotBoundsMsg),
                 "TEMP ERROR: Hot %.1fF out of bounds (%.0f-%.0fF) → closing valves",
                 hot.filteredF,
                 HOT_MIN_PLAUSIBLE_F,
                 HOT_MAX_PLAUSIBLE_F);
        faultMsg = hotBoundsMsg;
        runFlag = false;
      } else if (cold.filteredF < COLD_MIN_PLAUSIBLE_F || cold.filteredF > COLD_MAX_PLAUSIBLE_F) {
        detectedFault = FaultCode::ColdOutOfBounds;
        snprintf(coldBoundsMsg,
                 sizeof(coldBoundsMsg),
                 "TEMP ERROR: Cold %.1fF out of bounds (%.0f-%.0fF) → closing valves",
                 cold.filteredF,
                 COLD_MIN_PLAUSIBLE_F,
                 COLD_MAX_PLAUSIBLE_F);
        faultMsg = coldBoundsMsg;
        runFlag = false;
      } else {
        // Rapid change detection for hot/cold lines
        if (hot.sampleMs != 0 && lastHotRapidMs != 0) {
          const uint32_t dtMs = hot.sampleMs - lastHotRapidMs;
          if (dtMs > 0 && dtMs <= TEMP_RAPID_WINDOW_MS &&
              fabs(hot.filteredF - lastHotRapidF) >= TEMP_RAPID_DELTA_F) {
            detectedFault = FaultCode::HotRapidChange;
            snprintf(rapidMsg,
                     sizeof(rapidMsg),
                     "TEMP ERROR: Hot jump %.1fF in %lums → closing valves",
                     fabs(hot.filteredF - lastHotRapidF),
                     (unsigned long) dtMs);
            faultMsg = rapidMsg;
            runFlag = false;
          }
        }

        if (detectedFault == FaultCode::None && cold.sampleMs != 0 && lastColdRapidMs != 0) {
          const uint32_t dtMs = cold.sampleMs - lastColdRapidMs;
          if (dtMs > 0 && dtMs <= TEMP_RAPID_WINDOW_MS &&
              fabs(cold.filteredF - lastColdRapidF) >= TEMP_RAPID_DELTA_F) {
            detectedFault = FaultCode::ColdRapidChange;
            snprintf(rapidMsg,
                     sizeof(rapidMsg),
                     "TEMP ERROR: Cold jump %.1fF in %lums → closing valves",
                     fabs(cold.filteredF - lastColdRapidF),
                     (unsigned long) dtMs);
            faultMsg = rapidMsg;
            runFlag = false;
          }
        }

        if (detectedFault == FaultCode::None) {
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
                  flow.lpm);
    delay(LOOP_DELAY_MS);
    return;
  }

  activeFault = FaultCode::None;
  if (hot.sampleMs != 0) {
    lastHotRapidF = hot.filteredF;
    lastHotRapidMs = hot.sampleMs;
  }
  if (cold.sampleMs != 0) {
    lastColdRapidF = cold.filteredF;
    lastColdRapidMs = cold.sampleMs;
  }
  const float outletTempF = outlet.filteredF;
  const uint32_t sampleMs = outlet.sampleMs;
  if (sampleMs == 0 || sampleMs == lastOutletSampleMs) {
    logCsvIfDue(nowMs, outlet, linkOk);
    delay(LOOP_DELAY_MS);
    return;
  }

  // Initial mixing logic using cold and hot sensors
  static bool initialMixingDone = false;

  // Only do initial mixing if not done and outlet is far from setpoint
  if (!initialMixingDone && hot.present && cold.present && hot.valid && cold.valid) {
    // Extra smoothing filter for initial mixing
    static float hotFiltered = 0.0f;
    static float coldFiltered = 0.0f;
    constexpr float alpha = 0.2f; // Smoothing factor (0.0-1.0)
    if (hotFiltered == 0.0f) hotFiltered = hot.filteredF;
    if (coldFiltered == 0.0f) coldFiltered = cold.filteredF;
    hotFiltered = alpha * hot.filteredF + (1.0f - alpha) * hotFiltered;
    coldFiltered = alpha * cold.filteredF + (1.0f - alpha) * coldFiltered;
    // Calculate mix ratio to get as close as possible to setpoint
    float hotF = hotFiltered;
    float coldF = coldFiltered;
    // Avoid division by zero
    if (fabs(hotF - coldF) > 0.1f) {
      float initialRatio = (setpointF - coldF) / (hotF - coldF);
      initialRatio = constrain(initialRatio, 0.0f, 1.0f);
      applyMixRatio(initialRatio);
      lastRatio = initialRatio;
      Serial.printf("Initial mixing (filtered): HOT=%.2fF, COLD=%.2fF, SET=%.2fF, ratio=%.2f\n", hotF, coldF, setpointF, initialRatio);
      // If outlet is within 1°F of setpoint, switch to PID
      if (fabs(outletTempF - setpointF) < 1.0f) {
        initialMixingDone = true;
        pi.reset(); // Reset PID for clean start
      }
      logCsvIfDue(sampleMs, outlet, linkOk);
      delay(LOOP_DELAY_MS);
      return;
    }
  }

  // PID control after initial mixing
  initialMixingDone = true; // fallback if sensors not present/valid
  const float dtSec = (lastOutletSampleMs == 0)
                          ? (TEMP_LOOP_DT_MS / 1000.0f)
                          : (sampleMs - lastOutletSampleMs) / 1000.0f;
  lastOutletSampleMs = sampleMs;

  const float errorF = setpointF - outletTempF;
  const float ratio = pi.update(errorF, dtSec);
  lastU = pi.lastOutput();
  lastRatio = ratio;
  applyMixRatio(ratio);

  if (PID_LOG_CSV) {
    logCsvIfDue(sampleMs, outlet, linkOk);
  } else {
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
