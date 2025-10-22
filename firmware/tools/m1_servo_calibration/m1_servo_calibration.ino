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
//   [T] Toggle CSV logging

#include <ESP32Servo.h>

const int PIN_COLD = 18;
const int PIN_HOT = 19;

Servo sCold, sHot;
int ch = 0;  // 0 = Cold, 1 = Hot
int usCold = 1500, usHot = 1500;
bool autoSweep = false, csv = false;
int dirCold = +1, dirHot = -1;

// Marked limits (–1 means not set yet)
int usCOLD_MIN = -1, usCOLD_MAX = -1, usHOT_MIN = -1, usHOT_MAX = -1;

// Keep a small guard away from hard stops when jumping to MIN/MAX
const int GUARD_US = 15;

bool hasColdCal() { return usCOLD_MIN > 0 && usCOLD_MAX > 0; }
bool hasHotCal() { return usHOT_MIN > 0 && usHOT_MAX > 0; }

int clamp(int u) { return constrain(u, 500, 2500); }
int currentCold() { return usCold; }
int currentHot() { return usHot; }

// Calibrated-aware targets for SELECTED channel (ch: 0=Cold, 1=Hot)
int selMin() {
  if (ch == 0 && hasColdCal()) return clamp(usCOLD_MIN + GUARD_US);
  if (ch == 1 && hasHotCal()) return clamp(usHOT_MIN + GUARD_US);
  return 500;
}
int selMax() {
  if (ch == 0 && hasColdCal()) return clamp(usCOLD_MAX - GUARD_US);
  if (ch == 1 && hasHotCal()) return clamp(usHOT_MAX - GUARD_US);
  return 2500;
}
int selMid() {
  if (ch == 0 && hasColdCal()) return (usCOLD_MIN + usCOLD_MAX) / 2;
  if (ch == 1 && hasHotCal()) return (usHOT_MIN + usHOT_MAX) / 2;
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

void setup() {
  Serial.begin(115200);
  delay(200);

  sCold.setPeriodHertz(50);
  sHot.setPeriodHertz(50);
  sCold.attach(PIN_COLD, 500, 2500);
  sHot.attach(PIN_HOT, 500, 2500);
  sCold.writeMicroseconds(usCold);
  sHot.writeMicroseconds(usHot);

  banner();
}

void banner() {
  Serial.println(F("\n=== ESP32 Servo Calibration ==="));
  Serial.println(F("Keys: [H] Hot  [G] Cold  |  [R] AutoSweep  [T] CSV  [P] Print"));
  Serial.println(F("      [Z/X]=Cold MIN/MAX  [C/V]=Hot MIN/MAX"));
  Serial.println(F("      [A]=MIN  [S]=MID  [D]=MAX  (uses calibrated limits if set)"));
  Serial.println(F("      [Q/W/E] -5/-10/-25   [1/2/3] +5/+10/+25"));
  printState();
}

void loop() {
  if (Serial.available()) handleKey(Serial.read());

  if (autoSweep) {
    sweep(&usCold, &dirCold);
    sweep(&usHot, &dirHot);
    sCold.writeMicroseconds(usCold);
    sHot.writeMicroseconds(usHot);
    if (csv) printCSV();
    delay(20);
  }
}

void sweep(int* u, int* dir) {
  *u += (*dir) * 5;
  if (*u >= 2100) {
    *u = 2100;
    *dir = -1;
  }
  if (*u <= 900) {
    *u = 900;
    *dir = +1;
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
  // Ignore Serial Monitor line endings so each press prints once
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

    // Sweep / CSV
    case 'r':
    case 'R':
      autoSweep = !autoSweep;
      Serial.println(autoSweep ? "Auto ON" : "Auto OFF");
      break;
    case 't':
    case 'T':
      csv = !csv;
      Serial.println(csv ? "CSV ON" : "CSV OFF");
      break;

    // Position jumps (calibrated-aware)
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

    // Mark limits
    case 'z':
    case 'Z':
      usCOLD_MIN = currentCold();
      Serial.println("Marked Cold MIN");
      break;
    case 'x':
    case 'X':
      usCOLD_MAX = currentCold();
      Serial.println("Marked Cold MAX");
      break;
    case 'c':
    case 'C':
      usHOT_MIN = currentHot();
      Serial.println("Marked Hot MIN");
      break;
    case 'v':
    case 'V':
      usHOT_MAX = currentHot();
      Serial.println("Marked Hot MAX");
      break;

    // Print #defines
    case 'p':
    case 'P':
      printDefines();
      break;
  }

  printState();
}

void printState() {
  Serial.printf("COLD=%dus  HOT=%dus  sel=%s  auto=%d  csv=%d\n",
                usCold, usHot, (ch == 0 ? "Cold" : "Hot"), autoSweep, csv);
}

void printCSV() {
  static uint32_t t0 = millis();
  Serial.printf("t_ms=%lu,usCold=%d,usHot=%d\n", (unsigned long) (millis() - t0), usCold, usHot);
}

void printDefines() {
  Serial.println("\n// Paste into firmware/control/config.h");
  if (usCOLD_MIN > 0) Serial.printf("#define usCOLD_MIN %d\n", usCOLD_MIN);
  if (usCOLD_MAX > 0) Serial.printf("#define usCOLD_MAX %d\n", usCOLD_MAX);
  if (usHOT_MIN > 0) Serial.printf("#define usHOT_MIN  %d\n", usHOT_MIN);
  if (usHOT_MAX > 0) Serial.printf("#define usHOT_MAX  %d\n", usHOT_MAX);
  Serial.println();
}
