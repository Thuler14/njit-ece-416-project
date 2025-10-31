#pragma once
#include <DallasTemperature.h>

// ====================================================
// Temperature Sensors (DS18B20)
// ====================================================

#define TEMP_PIN_ONEWIRE 4          // DS18B20 data line (GPIO 4)
#define TEMP_RESOLUTION 9           // 9-bit (~93.75 ms conversion, 0.5°C step)
#define TEMP_CONVERSION_TIME_MS 94  // Max conversion time at 9-bit resolution
#define TEMP_LOOP_DT_MS 100         // 10 Hz polling rate
#define TEMP_MIN_VALID_C -60.0f     // Minimum valid temperature (°C)
#define TEMP_MAX_VALID_C 125.0f     // Maximum valid temperature (°C)
#define TEMP_EMA_ALPHA 0.20f        // EMA filter (τ ≈ 0.4 s @10 Hz)

// --- Sensor ROM Addresses ---
// Scanned using: firmware/tools/m1_temp_scan/m1_temp_scan.ino
const DeviceAddress TEMP_HOT_ADDR = {0x28, 0x8D, 0xAC, 0x12, 0x00, 0x00, 0x00, 0x93};   // Hot line
const DeviceAddress TEMP_COLD_ADDR = {0x28, 0x2C, 0x4F, 0x12, 0x00, 0x00, 0x00, 0x6C};  // Cold line
const DeviceAddress TEMP_OUT_ADDR = {0x28, 0x59, 0x4D, 0x12, 0x00, 0x00, 0x00, 0x9C};   // Outlet (mixed) line

// ====================================================
// Servo Motors (MG996R)
// ====================================================

#define PIN_SERVO_HOT 19   // Hot valve servo (GPIO 19)
#define PIN_SERVO_COLD 18  // Cold valve servo (GPIO 18)

// --- Calibration Values ---
// Calibrated using: firmware/tools/servo_calibration/servo_calibration.ino

#define usCOLD_MIN 1120  // µs at fully closed position
#define usCOLD_MAX 2080  // µs at fully open position
#define usHOT_MIN 1180   // µs at fully closed position
#define usHOT_MAX 2080   // µs at fully open position

#define SERVO_GUARD_US 15  // Guard offset to avoid hard stops

// ====================================================
// Flow Sensor (YF-S201)
// ====================================================

#define FLOW_PIN 21                // Flow sensor signal pin (GPIO 21)
#define FLOW_K_PULSES_PER_ML 7.5f  // K-factor: pulses per mL of water (placeholder; calibrate later)
#define FLOW_WINDOW_MS 500         // Sampling window (ms)
