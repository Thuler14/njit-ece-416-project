// ====================================================
// M1: Servo Sweep Test (Cold / Hot Valves)
// Purpose: Verify smooth motion of both valves using calibrated µs limits
//          defined in firmware/control/config.h
// Board: ESP32 (Control Module)
// Pins: GPIO 18 = Cold valve, GPIO 19 = Hot valve
// Power: 6 V DC (≥3 A) shared by both servos, common GND with ESP32
// ====================================================

#include <ESP32Servo.h>

#include "../../control/config.h"

Servo sCold, sHot;

void setup() {
  Serial.begin(115200);
  sCold.setPeriodHertz(50);
  sHot.setPeriodHertz(50);

  // Clamp hardware outputs to the measured usable window
  sCold.attach(18, usCOLD_MIN, usCOLD_MAX);
  sHot.attach(19, usHOT_MIN, usHOT_MAX);

  // Start inside the guard band (avoid pressing hard stops)
  const int COLD_MIN_OP = usCOLD_MIN + SERVO_GUARD_US;
  const int HOT_MIN_OP = usHOT_MIN + SERVO_GUARD_US;
  sCold.writeMicroseconds(COLD_MIN_OP);
  sHot.writeMicroseconds(HOT_MIN_OP);
}

void loop() {
  // Sweep both servos within calibrated + guard range
  for (int pos = usCOLD_MIN + SERVO_GUARD_US; pos <= usCOLD_MAX - SERVO_GUARD_US; pos += 5) {
    int posHot = map(pos, usCOLD_MIN, usCOLD_MAX,
                     usHOT_MIN + SERVO_GUARD_US, usHOT_MAX - SERVO_GUARD_US);

    sCold.writeMicroseconds(pos);
    sHot.writeMicroseconds(posHot);

    delay(15);  // smooth motion (~0.75s per sweep)
  }

  for (int pos = usCOLD_MAX - SERVO_GUARD_US; pos >= usCOLD_MIN + SERVO_GUARD_US; pos -= 5) {
    int posHot = map(pos, usCOLD_MIN, usCOLD_MAX,
                     usHOT_MIN + SERVO_GUARD_US, usHOT_MAX - SERVO_GUARD_US);

    sCold.writeMicroseconds(pos);
    sHot.writeMicroseconds(posHot);

    delay(15);
  }
}
