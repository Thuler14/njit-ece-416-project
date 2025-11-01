// ====================================================
// M1: Servo Calibration Tool (Cold / Hot Valves)
// Purpose: Find fully closed and open µs limits for both servos.
// Board: ESP32 (Control Unit)
// Pins: GPIO 18 = Cold valve, GPIO 19 = Hot valve
// Power: 6 V DC supply (≥3 A) with common ground to ESP32
// ====================================================
//
// Keyboard layout:
//   [C] Hot MIN      [V] Hot MAX
//   [Z] Cold MIN     [X] Cold MAX
//   [A] MIN (calibrated or 500)   [S] MID (calibrated or 1500)   [D] MAX (calibrated or 2500)
//   [Q]/[W]/[E]  -5 / -10 / -25 µs
//   [1]/[2]/[3]  +5 / +10 / +25 µs
//   [G] Select Cold  [H] Select Hot
//   [P] Print saved limits (#define)
//   [R] Toggle auto sweep

#include <ESP32Servo.h>

#include "../../control/config.h"  // Centralized configuration

// Attach bounds (keep wide for calibration)
constexpr int ATTACH_MIN_US = 500;
constexpr int ATTACH_MAX_US = 2500;

Servo sCold, sHot;
int ch = 0;  // 0 = Cold, 1 = Hot

// Internal working copies of calibration values.
#ifdef SERVO_COLD_MIN_US
int SERVO_COLD_MIN_US_VAL = SERVO_COLD_MIN_US;
#else
int SERVO_COLD_MIN_US_VAL = -1;
#endif

#ifdef SERVO_COLD_MAX_US
int SERVO_COLD_MAX_US_VAL = SERVO_COLD_MAX_US;
#else
int SERVO_COLD_MAX_US_VAL = -1;
#endif

#ifdef SERVO_HOT_MIN_US
int SERVO_HOT_MIN_US_VAL = SERVO_HOT_MIN_US;
#else
int SERVO_HOT_MIN_US_VAL = -1;
#endif

#ifdef SERVO_HOT_MAX_US
int SERVO_HOT_MAX_US_VAL = SERVO_HOT_MAX_US;
#else
int SERVO_HOT_MAX_US_VAL = -1;
#endif

// Current commanded pulse widths (start centered; will recenter if cal present)
int usCold = 1500;
int usHot = 1500;

bool autoSweep = false;
int dirCold = +1, dirHot = -1;

// Helpers / State
bool hasColdCal() { return SERVO_COLD_MIN_US_VAL > 0 && SERVO_COLD_MAX_US_VAL > 0; }
bool hasHotCal() { return SERVO_HOT_MIN_US_VAL > 0 && SERVO_HOT_MAX_US_VAL > 0; }
bool fullyCalibrated() { return hasColdCal() && hasHotCal(); }

int clamp(int u) { return constrain(u, ATTACH_MIN_US, ATTACH_MAX_US); }
int currentCold() { return usCold; }
int currentHot() { return usHot; }

int selMin() {
  if (ch == 0 && hasColdCal()) return clamp(SERVO_COLD_MIN_US_VAL + SERVO_GUARD_US);
  if (ch == 1 && hasHotCal()) return clamp(SERVO_HOT_MIN_US_VAL + SERVO_GUARD_US);
  return ATTACH_MIN_US;
}
int selMax() {
  if (ch == 0 && hasColdCal()) return clamp(SERVO_COLD_MAX_US_VAL - SERVO_GUARD_US);
  if (ch == 1 && hasHotCal()) return clamp(SERVO_HOT_MAX_US_VAL - SERVO_GUARD_US);
  return ATTACH_MAX_US;
}
int selMid() {
  if (ch == 0 && hasColdCal()) return (SERVO_COLD_MIN_US_VAL + SERVO_COLD_MAX_US_VAL) / 2;
  if (ch == 1 && hasHotCal()) return (SERVO_HOT_MIN_US_VAL + SERVO_HOT_MAX_US_VAL) / 2;
  return 1500;
}

void setSel(int target) {
  if (ch == 0) {
    usCold = clamp(target);
    sCold.writeMicroseconds(usCold);
  } else {
    usHot = clamp(target);
    sHot.writeMicroseconds(usHot);
  }
}

void applyInitialMids() {
  if (hasColdCal()) usCold = clamp((SERVO_COLD_MIN_US_VAL + SERVO_COLD_MAX_US_VAL) / 2);
  if (hasHotCal()) usHot = clamp((SERVO_HOT_MIN_US_VAL + SERVO_HOT_MAX_US_VAL) / 2);
}

// Sweep within provided [lo, hi] bounds (inclusive)
void sweepWithin(int* u, int* dir, int lo, int hi) {
  const int step = 5;
  *u += (*dir) * step;
  if (*u >= hi) {
    *u = hi;
    *dir = -1;
  }
  if (*u <= lo) {
    *u = lo;
    *dir = +1;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  sCold.setPeriodHertz(50);
  sHot.setPeriodHertz(50);
  sCold.attach(SERVO_PIN_COLD, ATTACH_MIN_US, ATTACH_MAX_US);
  sHot.attach(SERVO_PIN_HOT, ATTACH_MIN_US, ATTACH_MAX_US);

  applyInitialMids();
  sCold.writeMicroseconds(usCold);
  sHot.writeMicroseconds(usHot);

  banner();
}

void banner() {
  Serial.println(F("\n=== ESP32 Servo Calibration ==="));
  Serial.println(F("Keys: [H] Hot  [G] Cold  |  [R] AutoSweep  [P] Print"));
  Serial.println(F("      [Z/X]=Cold MIN/MAX  [C/V]=Hot MIN/MAX"));
  Serial.println(F("      [A]=MIN  [S]=MID  [D]=MAX  (uses config.h if present)"));
  Serial.println(F("      [Q/W/E] -5/-10/-25   [1/2/3] +5/+10/+25"));

  Serial.printf("Pins: COLD=%d, HOT=%d  |  GUARD=%d us\n",
                SERVO_PIN_COLD, SERVO_PIN_HOT, SERVO_GUARD_US);

  Serial.printf("Loaded from config.h?  Cold: %s  |  Hot: %s\n",
                hasColdCal() ? "YES" : "no",
                hasHotCal() ? "YES" : "no");

  if (hasColdCal()) Serial.printf("Cold limits: %d .. %d\n", SERVO_COLD_MIN_US_VAL, SERVO_COLD_MAX_US_VAL);
  if (hasHotCal()) Serial.printf("Hot  limits: %d .. %d\n", SERVO_HOT_MIN_US_VAL, SERVO_HOT_MAX_US_VAL);

  Serial.printf("Auto-sweep availability: %s (requires both servos calibrated)\n",
                fullyCalibrated() ? "READY" : "NOT READY");

  printState();
}

void loop() {
  if (Serial.available()) handleKey(Serial.read());

  // Safety: if calibration becomes invalid (e.g., user resets values), stop auto-sweep
  if (autoSweep && !fullyCalibrated()) {
    autoSweep = false;
    Serial.println("Auto-sweep disabled: both servos must be calibrated.");
  }

  if (autoSweep) {
    // Compute safe bounds from calibrated limits + guard
    const int cLo = clamp(SERVO_COLD_MIN_US_VAL + SERVO_GUARD_US);
    const int cHi = clamp(SERVO_COLD_MAX_US_VAL - SERVO_GUARD_US);
    const int hLo = clamp(SERVO_HOT_MIN_US_VAL + SERVO_GUARD_US);
    const int hHi = clamp(SERVO_HOT_MAX_US_VAL - SERVO_GUARD_US);

    sweepWithin(&usCold, &dirCold, cLo, cHi);
    sweepWithin(&usHot, &dirHot, hLo, hHi);

    sCold.writeMicroseconds(usCold);
    sHot.writeMicroseconds(usHot);

    printCSV();
    delay(20);
  }
}

void nudge(int delta) {
  if (ch == 0) {
    usCold = clamp(usCold + delta);
    sCold.writeMicroseconds(usCold);
  } else {
    usHot = clamp(usHot + delta);
    sHot.writeMicroseconds(usHot);
  }
}

void setAbs(int target) { setSel(target); }

void handleKey(int k) {
  if (k == '\n' || k == '\r') return;

  switch (k) {
    // Selection
    case 'g':
    case 'G':
      ch = 0;
      Serial.println("Selected: Cold");
      break;
    case 'h':
    case 'H':
      ch = 1;
      Serial.println("Selected: Hot");
      break;

    // Sweep toggle
    case 'r':
    case 'R':
      if (!fullyCalibrated()) {
        autoSweep = false;
        Serial.println("Auto-sweep blocked: define limits first (config.h or mark Z/X and C/V).");
      } else {
        autoSweep = !autoSweep;
        if (autoSweep) {
          Serial.println("Auto ON (CSV logging active)");
          printCSVHeader();
        } else {
          Serial.println("Auto OFF");
        }
      }
      break;

    // Position jumps (config-aware)
    case 'a':
    case 'A':
      setSel(selMin());
      break;
    case 's':
    case 'S':
      setSel(selMid());
      break;
    case 'd':
    case 'D':
      setSel(selMax());
      break;

    // Nudges
    case 'q':
    case 'Q':
      nudge(-5);
      break;
    case 'w':
    case 'W':
      nudge(-10);
      break;
    case 'e':
    case 'E':
      nudge(-25);
      break;
    case '1':
      nudge(+5);
      break;
    case '2':
      nudge(+10);
      break;
    case '3':
      nudge(+25);
      break;

    // Mark limits from current position
    case 'z':
    case 'Z':
      SERVO_COLD_MIN_US_VAL = currentCold();
      Serial.println("Marked Cold MIN");
      break;
    case 'x':
    case 'X':
      SERVO_COLD_MAX_US_VAL = currentCold();
      Serial.println("Marked Cold MAX");
      break;
    case 'c':
    case 'C':
      SERVO_HOT_MIN_US_VAL = currentHot();
      Serial.println("Marked Hot MIN");
      break;
    case 'v':
    case 'V':
      SERVO_HOT_MAX_US_VAL = currentHot();
      Serial.println("Marked Hot MAX");
      break;

    // Print #defines
    case 'p':
    case 'P':
      printDefines();
      break;
  }

  if (!autoSweep) printState();
}

void printState() {
  Serial.printf("COLD=%dus  HOT=%dus  sel=%s  auto=%d\n",
                usCold, usHot, (ch == 0 ? "Cold" : "Hot"), autoSweep);
}

void printCSVHeader() {
  Serial.println("t_ms,usCold,usHot");
}

void printCSV() {
  static uint32_t t0 = millis();
  uint32_t t = millis() - t0;
  Serial.printf("%lu,%d,%d\n", (unsigned long) t, usCold, usHot);
}

static void maybePrintDef(const char* name, int v) {
  if (v > 0) {
    Serial.print("#define ");
    Serial.print(name);
    Serial.print(" ");
    Serial.println(v);
  }
}

void printDefines() {
  Serial.println("\n// Paste into firmware/control/config.h");
  maybePrintDef("SERVO_COLD_MIN_US", SERVO_COLD_MIN_US_VAL);
  maybePrintDef("SERVO_COLD_MAX_US", SERVO_COLD_MAX_US_VAL);
  maybePrintDef("SERVO_HOT_MIN_US", SERVO_HOT_MIN_US_VAL);
  maybePrintDef("SERVO_HOT_MAX_US", SERVO_HOT_MAX_US_VAL);
  Serial.println();
}
