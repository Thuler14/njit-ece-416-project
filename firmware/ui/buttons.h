/*
 * ================================================================
 *  Module: buttons
 *  Purpose: Handles user input for the UI unit.
 *           Provides debounced button reads and press detection.
 *
 *  Hardware:
 *    - 5 active-low pushbuttons (▲ ▼ ● A B)
 *
 *  Dependencies:
 *    - config.h  (GPIO pin assignments)
 *    - Arduino core (digitalRead, pinMode)
 *
 *  Interface:
 *    void buttonsInit();
 *    bool buttonIsDown(BtnIndex idx);
 *    bool buttonPressed(BtnIndex idx);
 *
 *  Data Structures:
 *    enum BtnIndex { BTN_UP, BTN_DOWN, BTN_OK, BTN_A, BTN_B, BTN_COUNT };
 *    struct Btn {
 *      const char* label;   // "▲", "▼", "●", "A", "B"
 *      uint8_t pin;         // GPIO number
 *      bool lastStable;     // last debounced state
 *      unsigned long t;     // last debounce timestamp (ms)
 *    };
 * ================================================================
 */

#pragma once

#include <Arduino.h>

#include "config.h"

// Button indices for BTN_LIST[]
enum BtnIndex : uint8_t {
  BTN_UP = 0,
  BTN_DOWN,
  BTN_OK,
  BTN_A,
  BTN_B,
  BTN_COUNT
};

// Button metadata and debounce state
struct Btn {
  const char* label;  // "▲", "▼", "●", "A", "B"
  uint8_t pin;        // GPIO
  bool lastStable;    // last debounced state
  unsigned long t;    // last debounce timestamp
};

// Global list of button descriptors (defined in buttons.cpp)
extern Btn BTN_LIST[BTN_COUNT];

// Initialize button GPIOs and debounce state
void buttonsInit();

// Return true while the button is physically pressed (active-low)
bool buttonIsDown(BtnIndex idx);

// Return true once per press after debounce
bool buttonPressed(BtnIndex idx);
