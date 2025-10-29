#pragma once

// ====================================================
// Temperature Sensors (DS18B20)
// ====================================================

#define PIN_TEMP_ONEWIRE 4  // DS18B20 data line (GPIO 4)

// --- Sensor ROM Addresses ---
// Scanned using: firmware/tools/m1_temp_scan/m1_temp_scan.ino
#define ADDR_T_COLD {0x28, 0x2C, 0x4F, 0x12, 0x00, 0x00, 0x00, 0x6C}  // Cold line
#define ADDR_T_HOT {0x28, 0x8D, 0xAC, 0x12, 0x00, 0x00, 0x00, 0x93}   // Hot line
#define ADDR_T_OUT {0x28, 0x59, 0x4D, 0x12, 0x00, 0x00, 0x00, 0x9C}   // Outlet (mixed) line

// ====================================================
// Servo Motors (MG996R)
// ====================================================

#define PIN_SERVO_COLD 18  // Cold valve servo (GPIO 18)
#define PIN_SERVO_HOT 19   // Hot valve servo (GPIO 19)

// --- Calibration Values ---
// Calibrated using: firmware/tools/servo_calibration/servo_calibration.ino

#define usCOLD_MIN 1120  // µs at fully closed position
#define usCOLD_MAX 2080  // µs at fully open position
#define usHOT_MIN 1180   // µs at fully closed position
#define usHOT_MAX 2080   // µs at fully open position

#define SERVO_GUARD_US 15  // Guard offset to avoid hard stops
