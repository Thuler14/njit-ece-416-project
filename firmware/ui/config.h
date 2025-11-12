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

// --- Debounce Configuration ---
constexpr unsigned long BTN_DEBOUNCE_MS = 20;
