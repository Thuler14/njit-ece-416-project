#pragma once

#include <stdint.h>

// ====================================================
// OLED Display (SSD1309 SPI Blue)
// ====================================================

// --- Pin Configuration ---
constexpr uint8_t OLED_PIN_CS = 5;   // Chip Select (CS)
constexpr uint8_t OLED_PIN_DC = 22;  // Data/Command (DC)
constexpr uint8_t OLED_PIN_RST = 4;  // Reset (RST)

// ====================================================
// Push Buttons (▲ ▼ ● A B)
// ====================================================

constexpr uint8_t BTN_PIN_UP = 25;    // Up button (GPIO 25)
constexpr uint8_t BTN_PIN_DOWN = 26;  // Down button (GPIO 26)
constexpr uint8_t BTN_PIN_OK = 27;    // OK button (GPIO 27)
constexpr uint8_t BTN_PIN_A = 14;     // A button (GPIO 14)
constexpr uint8_t BTN_PIN_B = 13;     // B button (GPIO 13)

// --- Debounce & Gesture Timing (ms) ---
constexpr unsigned long BTN_DEBOUNCE_MS = 25;       // debounce filter time
constexpr unsigned long BTN_DBLCLICK_MS = 350;      // maximum gap between clicks for double-click
constexpr unsigned long BTN_LONGPRESS_MS = 800;     // time held to trigger a long press
constexpr unsigned long BTN_REPEAT_DELAY_MS = 500;  // delay before the first repeat event
constexpr unsigned long BTN_REPEAT_MS = 150;        // interval between subsequent repeats

// ====================================================
// Communication heartbeat
// ====================================================

constexpr unsigned long UI_HEARTBEAT_MS = 1000;  // UI -> Control heartbeat interval
