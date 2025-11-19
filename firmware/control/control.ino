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

static constexpr float LOOP_DT_SEC = 0.012f;
static constexpr uint16_t LOOP_DELAY_MS = 12;
static PID pi(0.10f, 0.08f, 0.0f, 1.0f);

static float setpointF = SETPOINT_DEFAULT_F;
static bool runFlag = false;
static bool outletFaultActive = false;

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

  const float outletTempF = outlet.filteredF;
  const float errorF = setpointF - outletTempF;
  const float ratio = pi.update(errorF, LOOP_DT_SEC);
  applyMixRatio(ratio);

  Serial.printf("OUT=%.2fF / SET=%.2fF | error=%.2fF | ratio=%.2f\n",
                outletTempF, setpointF, errorF, ratio);

  delay(LOOP_DELAY_MS);
}
