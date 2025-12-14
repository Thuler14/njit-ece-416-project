#pragma once

#include <stdint.h>

using DeviceAddress = uint8_t[8];

// ====================================================
// Temperature Sensors (DS18B20)
// ====================================================

constexpr uint8_t TEMP_PIN_ONEWIRE = 4;           // DS18B20 data line (GPIO 4)
constexpr uint8_t TEMP_RESOLUTION = 9;            // 9-bit (~93.75 ms conversion, 0.5°C step)
constexpr unsigned TEMP_CONVERSION_TIME_MS = 94;  // Max conversion time at 9-bit resolution
constexpr unsigned TEMP_LOOP_DT_MS = 100;         // 10 Hz polling rate
constexpr float TEMP_MIN_VALID_C = -60.0f;        // Minimum valid temperature (°C)
constexpr float TEMP_MAX_VALID_C = 125.0f;        // Maximum valid temperature (°C)
constexpr float TEMP_EMA_ALPHA = 0.20f;           // EMA filter (τ ≈ 0.4 s @10 Hz)

// Plausibility window for outlet control logic (°F)
constexpr float OUTLET_MIN_PLAUSIBLE_F = 32.0f;
constexpr float OUTLET_MAX_PLAUSIBLE_F = 140.0f;

// Plausibility for individual lines (°F)
constexpr float HOT_MIN_PLAUSIBLE_F = 50.0f;
constexpr float HOT_MAX_PLAUSIBLE_F = 140.0f;
constexpr float COLD_MIN_PLAUSIBLE_F = 32.0f;
constexpr float COLD_MAX_PLAUSIBLE_F = 90.0f;

// Rapid change detection
constexpr float TEMP_RAPID_DELTA_F = 12.0f;          // Trigger if jump exceeds this delta
constexpr unsigned TEMP_RAPID_WINDOW_MS = 1000;      // Time window for rapid change check

// --- Sensor ROM Addresses ---
// Scanned using: firmware/tools/m1_temp_scan/m1_temp_scan.ino
const DeviceAddress TEMP_HOT_ADDR = {0x28, 0x9D, 0xE1, 0xBA, 0x00, 0x00, 0x00, 0x78};   // Hot line
const DeviceAddress TEMP_COLD_ADDR = {0x28, 0x84, 0xB3, 0xB4, 0x00, 0x00, 0x00, 0xB6};  // Cold line
const DeviceAddress TEMP_OUT_ADDR = {0x28, 0x4D, 0xF1, 0xBA, 0x00, 0x00, 0x00, 0xC0};   // Outlet (mixed) line

// ====================================================
// Servo Motors (MG996R)
// ====================================================

constexpr uint8_t SERVO_PIN_HOT = 19;   // Hot valve servo (GPIO 19)
constexpr uint8_t SERVO_PIN_COLD = 18;  // Cold valve servo (GPIO 18)

// --- Calibration Values ---
// Calibrated using: firmware/tools/servo_calibration/servo_calibration.ino

constexpr unsigned SERVO_HOT_MIN_US = 1180;   // µs at fully open position
constexpr unsigned SERVO_HOT_MAX_US = 2080;   // µs at fully closed position
constexpr unsigned SERVO_COLD_MIN_US = 1120;  // µs at fully open position
constexpr unsigned SERVO_COLD_MAX_US = 2080;  // µs at fully closed position
constexpr unsigned SERVO_GUARD_US = 15;       // Guard offset to avoid hard stops

// ====================================================
// Flow Sensor (YF-S201)
// ====================================================

constexpr uint8_t FLOW_PIN = 21;               // Flow sensor signal pin (GPIO 21)
constexpr float FLOW_K_PULSES_PER_ML = 7.5f;   // K-factor: pulses per mL of water (placeholder; calibrate later)
constexpr unsigned FLOW_WINDOW_MS = 500;       // Sampling window (ms)
constexpr unsigned FLOW_FILTER_TAU_MS = 1000;  // EMA time constant for flow smoothing (ms)

// ====================================================
// Emergency Stop Switch
// ====================================================

constexpr uint8_t ESTOP_PIN = 33;  // E-stop button input

// ====================================================
// PID Controller (Outlet Temperature)
// ====================================================

constexpr float PID_KP = 0.025f;     // Proportional gain
constexpr float PID_KI = 0.005f;     // Integral gain (per second)
constexpr float PID_KD = 0.0f;       // Derivative gain
constexpr float PID_OUT_MIN = 0.0f;  // Output lower bound (mix ratio)
constexpr float PID_OUT_MAX = 1.0f;  // Output upper bound (mix ratio)
constexpr float PID_ERROR_DEADBAND_F = 0.2f;      // No integrate/drive when |error| < deadband
constexpr float PID_OUTPUT_SLEW_PER_SEC = 0.8f;   // Slow slew (per second, 0-1 scale)
constexpr float PID_OUTPUT_SLEW_FAST_PER_SEC = 3.0f; // Fast slew when far from setpoint
constexpr float PID_SLEW_ERROR_THRESH_F = 3.0f;   // Error threshold to use fast slew
constexpr bool PID_LOG_CSV = true;   // Enable CSV logging (time_ms,out_f,set_f,error_f,ratio)

// ====================================================
// Safety / Communication
// ====================================================

constexpr unsigned COMM_LINK_TIMEOUT_MS = 2000;  // Link-loss timeout (ms)
