// ====================================================
// M1: Servo Sweep Test (Cold / Hot Valves)
// Purpose: Verify smooth motion of both valves using calibrated µs limits
//          defined in firmware/control/config.h
// Board: ESP32 (Control Module)
// Pins: GPIO 18 = Cold valve, GPIO 19 = Hot valve
// Power: 6 V DC (≥3 A) shared by both servos, common GND with ESP32
// ====================================================

#include <ESP32Servo.h>

#define SERVO_PIN_HOT 19
#define SERVO_PIN_COLD 18

#define SERVO_HOT_MIN_US 1180
#define SERVO_HOT_MAX_US 2080
#define SERVO_COLD_MIN_US 1120
#define SERVO_COLD_MAX_US 2080
#define SERVO_GUARD_US 15

Servo sCold, sHot;

void setup() {
  Serial.begin(115200);
  sCold.setPeriodHertz(50);
  sHot.setPeriodHertz(50);

  // Clamp hardware outputs to the measured usable window
  sCold.attach(SERVO_PIN_COLD, SERVO_COLD_MIN_US, SERVO_COLD_MAX_US);
  sHot.attach(SERVO_PIN_HOT, SERVO_HOT_MIN_US, SERVO_HOT_MAX_US);

  // Start inside the guard band (avoid pressing hard stops)
  const int COLD_MIN_OP = SERVO_COLD_MIN_US + SERVO_GUARD_US;
  const int HOT_MIN_OP = SERVO_HOT_MIN_US + SERVO_GUARD_US;
  sCold.writeMicroseconds(COLD_MIN_OP);
  sHot.writeMicroseconds(HOT_MIN_OP);
}

void loop() {
  // Sweep both servos within calibrated + guard range
  for (int pos = SERVO_COLD_MIN_US + SERVO_GUARD_US; pos <= SERVO_COLD_MAX_US - SERVO_GUARD_US; pos += 5) {
    int posHot = map(pos, SERVO_COLD_MIN_US, SERVO_COLD_MAX_US,
                     SERVO_HOT_MIN_US + SERVO_GUARD_US, SERVO_HOT_MAX_US - SERVO_GUARD_US);

    sCold.writeMicroseconds(pos);
    sHot.writeMicroseconds(posHot);

    delay(15);  // smooth motion (~0.75s per sweep)
  }

  for (int pos = SERVO_COLD_MAX_US - SERVO_GUARD_US; pos >= SERVO_COLD_MIN_US + SERVO_GUARD_US; pos -= 5) {
    int posHot = map(pos, SERVO_COLD_MIN_US, SERVO_COLD_MAX_US,
                     SERVO_HOT_MIN_US + SERVO_GUARD_US, SERVO_HOT_MAX_US - SERVO_GUARD_US);

    sCold.writeMicroseconds(pos);
    sHot.writeMicroseconds(posHot);

    delay(15);
  }
}
