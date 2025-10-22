#pragma once

// ====================================================
// Temperature Sensors (DS18B20)
// Scanned: firmware/tools/m1_temp_scan/m1_temp_scan.ino
// ====================================================

// OneWire bus pin
#define PIN_TEMP_ONEWIRE 4  // DS18B20 data line (GPIO 4)

// Sensor ROM addresses (8 bytes each)
#define ADDR_T_HOT {0x28, 0x8D, 0xAC, 0x12, 0x00, 0x00, 0x00, 0x93}
#define ADDR_T_COLD {0x28, 0x2C, 0x4F, 0x12, 0x00, 0x00, 0x00, 0x6C}
#define ADDR_T_OUT {0x28, 0x59, 0x4D, 0x12, 0x00, 0x00, 0x00, 0x9C}

// ====================================================
// Servo Calibration (Cold / Hot Valves)
// Calibrated: firmware/tools/servo_calibration/servo_calibration.ino
// ====================================================

#define usCOLD_MIN  1120   // µs at fully closed position
#define usCOLD_MAX  2080   // µs at fully open position
#define usHOT_MIN   1180   // µs at fully closed position
#define usHOT_MAX   2080   // µs at fully open position

#define SERVO_GUARD_US  15 // Guard offset to avoid hard stops
