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
static bool outletFaultActive = false;
static unsigned long lastLoopMs = 0;
static bool printedCsvHeader = false;

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

  if (!runFlag) {
    outletFaultActive = false;
    enterSafeState(nullptr);
    delay(LOOP_DELAY_MS);
    return;
  }

  const TemperatureReading& outlet = temperatureGetReading(TempSensor::OUTLET);
  if (!outlet.present || !outlet.valid) {
    if (!outletFaultActive) {
      const char* msg = !outlet.present
                            ? "TEMP ERROR: Outlet sensor missing → closing valves"
                            : "TEMP ERROR: Invalid outlet reading → closing valves";
      enterSafeState(msg);
    } else {
      enterSafeState(nullptr);
    }
    outletFaultActive = true;
    delay(LOOP_DELAY_MS);
    return;
  }
  if (outletFaultActive) {
    Serial.println("TEMP INFO: Outlet sensor recovered");
    outletFaultActive = false;
  }

  const unsigned long nowMs = millis();
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
