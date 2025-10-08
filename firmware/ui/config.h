#pragma once

#include <U8g2lib.h>

// ====================================================
// OLED Display (SSD1309 SPI Blue)
// ====================================================

#define PIN_OLED_CS 5   // Chip Select (CS)
#define PIN_OLED_DC 22  // Data/Command (DC)
#define PIN_OLED_RST 4  // Reset (RST)

// ====================================================
// U8g2 Display Object
// ====================================================
U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI u8g2(
    U8G2_R0,  // rotation
    /* cs=*/PIN_OLED_CS,
    /* dc=*/PIN_OLED_DC,
    /* reset=*/PIN_OLED_RST);

// ====================================================
// Push Buttons (▲ ▼ ● A B)
// ====================================================

#define PIN_BTN_UP 25
#define PIN_BTN_DOWN 26
#define PIN_BTN_OK 27
#define PIN_BTN_A 14
#define PIN_BTN_B 13

// Debounce delay (ms)
constexpr unsigned long DEBOUNCE_MS = 20;

// Button structure
struct Btn {
  const char* label;  // "▲","▼","●","A","B"
  uint8_t pin;        // GPIO number
  bool lastStable;    // last debounced state
  unsigned long t;    // last debounce timestamp
};

// Button definitions
Btn buttons[] = {
    {"▲", PIN_BTN_UP, true, 0},
    {"▼", PIN_BTN_DOWN, true, 0},
    {"●", PIN_BTN_OK, true, 0},
    {"A", PIN_BTN_A, true, 0},
    {"B", PIN_BTN_B, true, 0},
};
