/*
 * ================================================================
 *  Module: flow_sensor
 *  Purpose: Interrupt-driven reader for the YF-S201 flow sensor on
 *           the control unit. Counts pulses, converts to Hz and
 *           liters per minute, and provides a cached reading for
 *           the control loop logger.
 *  Interface:
 *    bool flowSensorInit();
 *    bool flowSensorUpdate();
 *    FlowReading flowSensorGet();
 * ================================================================
 */

#pragma once

#include <stdint.h>

// Snapshot of the most recent flow measurement (YF-S201)
struct FlowReading {
  float lpm;        // Filtered liters per minute (EMA)
  float lpmRaw;     // Instantaneous liters per minute for the last window
  float Hz;         // Filtered pulse frequency (EMA)
  float HzRaw;      // Instantaneous pulse frequency for the last window
  uint32_t sampleMs;
  bool fresh;  // true if updated on the latest window
};

// Initialize the flow sensor GPIO + interrupt handler
bool flowSensorInit();

// Update measurement if the sampling window elapsed (call each loop)
bool flowSensorUpdate();

// Return the last computed measurement (may be stale if no pulses)
FlowReading flowSensorGet();

// Convenience accessor for liters per minute
inline float flowSensorLpm() { return flowSensorGet().lpm; }
