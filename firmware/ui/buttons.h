/*
 * ================================================================
 *  Module: buttons
 *  Purpose: Handles user input for the UI unit with a custom
 *           debouncer and event generator.
 *           Provides debounced detection of clicks, double-clicks,
 *           long presses, and repeat presses.
 *
 *  Hardware:
 *    - 5 active-low pushbuttons: ▲, ▼, ●, A, B
 *
 *  Dependencies:
 *    - config.h        (GPIO pin definitions, timing constants)
 *    - Arduino core    (pinMode, digitalRead, millis)
 *
 *  Interface:
 *    void buttonsInit();
 *    bool buttonsPoll(ButtonsEvents& out);
 *
 *  Data Structures:
 *    enum ButtonId { BUTTON_UP, BUTTON_DOWN, BUTTON_OK, BUTTON_A, BUTTON_B, BUTTON_COUNT };
 *    struct ButtonsEvents {
 *      bool upClick,   upDblClick,   upLong,   upRepeat;
 *      bool downClick, downDblClick, downLong, downRepeat;
 *      bool okClick,   okDblClick,   okLong,   okRepeat;
 *      bool aClick,    aDblClick,    aLong,    aRepeat;
 *      bool bClick,    bDblClick,    bLong,    bRepeat;
 *      bool chordStepLong;
 *      bool chordFlowLong;
 *    };
 * ================================================================
 */

#pragma once

#include <Arduino.h>

#include "config.h"

// Logical button IDs
enum ButtonId : uint8_t {
  BUTTON_UP = 0,
  BUTTON_DOWN,
  BUTTON_OK,
  BUTTON_A,
  BUTTON_B,
  BUTTON_COUNT
};

// One-shot event bundle returned by buttonsPoll()
struct ButtonsEvents {
  // UP/DOWN
  bool upClick = false;
  bool upDblClick = false;
  bool upLong = false;
  bool upRepeat = false;

  bool downClick = false;
  bool downDblClick = false;
  bool downLong = false;
  bool downRepeat = false;

  // OK
  bool okClick = false;
  bool okDblClick = false;
  bool okLong = false;
  bool okRepeat = false;

  // Presets
  bool aClick = false;
  bool aDblClick = false;
  bool aLong = false;
  bool aRepeat = false;

  bool bClick = false;
  bool bDblClick = false;
  bool bLong = false;
  bool bRepeat = false;

  // Chords
  bool chordStepLong = false;
  bool chordFlowLong = false;
};

// Initializes all buttons (active-low with pull-ups)
void buttonsInit();

// Polls all buttons; fills 'out' with events, clears latches,
// and returns true if any event occurred this cycle.
bool buttonsPoll(ButtonsEvents& out);
