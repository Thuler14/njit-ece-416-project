/*
 * ================================================================
 *  Module: display
 *  Purpose: Provides OLED display functions for the UI unit.
 *           Renders current setpoint, run state, and communication status.
 *
 *  Hardware:
 *    - SSD1309 128×64 OLED (SPI, handled by U8g2 library)
 *
 *  Dependencies:
 *    - config.h      (display pin definitions and constants)
 *    - U8g2 library  (graphics rendering)
 *
 *  Interface:
 *    void displayInit();
 *    void displayDraw(const DisplayState& s);
 *
 *  Data Structures:
 *    struct DisplayState {
 *      float setpointF;       // current temperature setpoint (°F)
 *      float outletTempF;     // latest outlet temperature from Control (°F)
 *      float stepF;           // current step size for adjustments
 *      bool  showingSetpoint; // true when user is editing/pending
 *      bool  outletValid;     // outletTempF is valid
 *      bool  runFlag;         // true=ON, false=OFF
 *      uint32_t txDoneCount;  // number of completed transmissions
 *      bool  lastResultOk;    // true=ACK received, false=TX failed
 *      bool  pending;         // true=waiting for ACK or unsent edits
 *    };
 * ================================================================
 */

#pragma once

#include <U8g2lib.h>

#include "config.h"

// Structure representing what will be shown on the OLED
struct DisplayState {
  float setpointF;
  float outletTempF;
  float stepF;
  bool showingSetpoint;
  bool outletValid;
  bool runFlag;
  uint32_t txDoneCount;
  bool lastResultOk;
  bool pending;
};

// Global U8g2 display instance (defined in display.cpp)
extern U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI oledDisplay;

// Initialize and attach the OLED display
void displayInit();

// Draws current setpoint, run state, and TX status on the OLED
void displayDraw(const DisplayState& s);
