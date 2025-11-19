/*
 * ================================================================
 *  Module: temperature
 *  Purpose: Non-blocking manager for the three DS18B20 sensors
 *           (hot, cold, outlet) connected to the Control Unit.
 *           Handles address binding, 100 ms conversion cadence,
 *           validation, and EMA filtering used by the control loop.
 *
 *  Dependencies:
 *    - config.h            (sensor pin, ROM addresses, timing constants)
 *    - OneWire + DallasTemperature (implementation only)
 *
 *  Interface:
 *    bool temperatureInit();
 *    bool temperatureService();
 *    bool temperatureSensorPresent(TempSensor sensor);
 *    const TemperatureReading& temperatureGetReading(TempSensor sensor);
 *    float temperatureOutletFilteredF();
 *    bool temperatureAnyFault();
 * ================================================================
 */

#pragma once

#include <Arduino.h>

#include "config.h"

// Logical identifiers for the three DS18B20 sensors on the bus
enum class TempSensor : uint8_t {
  HOT = 0,
  COLD,
  OUTLET,
  COUNT,
};

constexpr size_t TEMP_SENSOR_COUNT = static_cast<size_t>(TempSensor::COUNT);

// Snapshot of the last captured reading for a sensor
struct TemperatureReading {
  bool present;         // true if the ROM address responded during init
  bool valid;           // last sample passed sanity checks
  bool fresh;           // a new sample has been captured since last read
  uint32_t sampleMs;    // millis() timestamp of the last sample
  float rawC;           // direct Celsius reading from the DS18B20
  float rawF;           // raw reading converted to °F
  float filteredC;      // EMA filtered value in °C
  float filteredF;      // EMA filtered value in °F (used by PI loop)
};

// Initialize the bus, bind sensors to ROM addresses, prime the EMA filter
bool temperatureInit();

// Service routine that should be called each loop iteration.
// Schedules conversions at TEMP_LOOP_DT_MS cadence and updates readings.
// Returns true when at least one sensor produced a fresh sample.
bool temperatureService();

// Helper to check if a specific sensor was detected on the bus
bool temperatureSensorPresent(TempSensor sensor);

// Access the latest reading for the requested sensor.
// The returned reference remains valid until the next temperatureService() call.
const TemperatureReading& temperatureGetReading(TempSensor sensor);

// Convenience helpers for the outlet sensor (mixed water)
inline float temperatureOutletFilteredF() {
  return temperatureGetReading(TempSensor::OUTLET).filteredF;
}

inline float temperatureOutletFilteredC() {
  return temperatureGetReading(TempSensor::OUTLET).filteredC;
}

// True if any required sensor is missing or currently invalid
bool temperatureAnyFault();
