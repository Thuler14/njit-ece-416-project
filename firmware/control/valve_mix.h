/*
 * ================================================================
 *  Module: valve_mix
 *  Purpose: Provides servo control functions for hot and cold valves.
 *           Maps blend ratio [0.0–1.0] to calibrated servo positions.
 *
 *  Dependencies:
 *    - config.h  (servo calibration constants and pin definitions)
 *    - ESP32Servo library
 *
 *  Interface:
 *    void valveMixInit();
 *    void applyMixRatio(float ratio);
 *    void valveMixCloseAll();
 *    int  lastColdUs();
 *    int  lastHotUs();
 * ================================================================
 */

#pragma once

// Initialize and attach servos using pins & calibration from config.h
void valveMixInit();

// Close both valves (safe state)
void valveMixCloseAll();

// ratio ∈ [0,1]: 0 → all COLD (Cold open, Hot closed)
//                1 → all HOT  (Hot open,  Cold closed)
void applyMixRatio(float ratio);

// Optional helpers (useful for logging/tests)
int lastColdUs();
int lastHotUs();
