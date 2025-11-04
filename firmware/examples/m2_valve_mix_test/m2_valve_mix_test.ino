// ====================================================
// M2: Valve Mixing Test (Servo Hot/Cold + Ratio Sweep)
// Purpose: Verify synchronized servo actuation across
//          defined hot/cold mix ratios and confirm
//          valve_mix() timing stability for calibration.
// Board: ESP32 (Control Module)
// Actuators: MG996R Servos for Hot and Cold Valves
// ====================================================

#include "../../control/valve_mix.cpp"
#include "../../control/valve_mix.h"

static const float kRatios[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
static const uint8_t kN = sizeof(kRatios) / sizeof(kRatios[0]);

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("t_ms,ratio,cold_us,hot_us");
  valveMixBegin();
  delay(500);

  const unsigned hold_ms = 3000;  // ~15 s total for all 5 steps

  for (uint8_t i = 0; i < kN; ++i) {
    float r = kRatios[i];
    applyMixRatio(r);

    // Log a few times during each hold so you have steady data while filming
    const unsigned t0 = millis();
    while (millis() - t0 < hold_ms) {
      Serial.printf("%lu,%.2f,%d,%d\n",
                    (unsigned long) millis(),
                    r,
                    lastColdUs(),
                    lastHotUs());
      delay(250);
    }
  }

  // Safe shutdown
  valveMixCloseAll();
}

void loop() {}
