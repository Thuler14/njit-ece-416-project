#include "temperature.h"

#include <DallasTemperature.h>
#include <OneWire.h>
#include <math.h>
#include <string.h>

static OneWire g_oneWire(TEMP_PIN_ONEWIRE);
static DallasTemperature g_sensors(&g_oneWire);

static TemperatureReading g_readings[TEMP_SENSOR_COUNT];

static uint32_t g_nextLoopMs = 0;
static uint32_t g_lastRequestMs = 0;
static bool g_conversionPending = false;
static bool g_initialized = false;

static constexpr size_t sensorIndex(TempSensor sensor) {
  return static_cast<size_t>(sensor);
}

static const DeviceAddress& sensorAddr(TempSensor sensor) {
  switch (sensor) {
    case TempSensor::HOT:
      return TEMP_HOT_ADDR;
    case TempSensor::COLD:
      return TEMP_COLD_ADDR;
    case TempSensor::OUTLET:
    default:
      return TEMP_OUT_ADDR;
  }
}

static void scheduleNextLoop(uint32_t now) {
  do {
    g_nextLoopMs += TEMP_LOOP_DT_MS;
  } while ((int32_t) (now - g_nextLoopMs) >= 0);
}

static void clearFreshFlags() {
  for (auto& r : g_readings) {
    r.fresh = false;
  }
}

bool temperatureInit() {
  g_sensors.begin();
  g_sensors.setWaitForConversion(false);
  g_sensors.setCheckForConversion(true);

  bool anyPresent = false;
  memset(g_readings, 0, sizeof(g_readings));

  for (size_t idx = 0; idx < TEMP_SENSOR_COUNT; ++idx) {
    TempSensor sensor = static_cast<TempSensor>(idx);
    TemperatureReading& reading = g_readings[idx];

    reading.rawC = NAN;
    reading.rawF = NAN;
    reading.filteredC = NAN;
    reading.filteredF = NAN;
    reading.fresh = false;
    reading.valid = false;
    reading.sampleMs = 0;

    reading.present = g_sensors.isConnected(sensorAddr(sensor));
    if (reading.present) {
      anyPresent = true;
      g_sensors.setResolution(sensorAddr(sensor), TEMP_RESOLUTION);
    }
  }

  uint32_t now = millis();
  g_nextLoopMs = now;
  g_lastRequestMs = now;
  g_sensors.requestTemperatures();
  g_conversionPending = true;
  g_initialized = true;

  return anyPresent;
}

bool temperatureService() {
  if (!g_initialized) return false;

  uint32_t now = millis();

  if (!g_conversionPending) {
    if ((int32_t) (now - g_nextLoopMs) < 0) return false;

    g_sensors.requestTemperatures();
    g_lastRequestMs = now;
    g_conversionPending = true;
    return false;
  }

  bool ready = g_sensors.isConversionComplete() || ((now - g_lastRequestMs) >= TEMP_CONVERSION_TIME_MS);
  if (!ready) return false;
  if ((int32_t) (now - g_nextLoopMs) < 0) return false;

  bool anyFresh = false;
  clearFreshFlags();

  for (size_t idx = 0; idx < TEMP_SENSOR_COUNT; ++idx) {
    TempSensor sensor = static_cast<TempSensor>(idx);
    TemperatureReading& reading = g_readings[idx];

    if (!reading.present) {
      reading.valid = false;
      continue;
    }

    float tempC = g_sensors.getTempC(sensorAddr(sensor));

    bool ok = (!isnan(tempC)) && (tempC != DEVICE_DISCONNECTED_C) &&
              (tempC > TEMP_MIN_VALID_C) && (tempC < TEMP_MAX_VALID_C);

    if (!ok) {
      reading.valid = false;
      continue;
    }

    reading.sampleMs = now;
    reading.rawC = tempC;
    reading.rawF = DallasTemperature::toFahrenheit(tempC);

    if (!reading.valid || isnan(reading.filteredC)) {
      reading.filteredC = tempC;
    } else {
      reading.filteredC += TEMP_EMA_ALPHA * (tempC - reading.filteredC);
    }
    reading.filteredF = DallasTemperature::toFahrenheit(reading.filteredC);

    reading.valid = true;
    reading.fresh = true;
    anyFresh = true;
  }

  g_conversionPending = false;
  scheduleNextLoop(now);

  g_sensors.requestTemperatures();
  g_lastRequestMs = now;
  g_conversionPending = true;

  return anyFresh;
}

bool temperatureSensorPresent(TempSensor sensor) {
  size_t idx = sensorIndex(sensor);
  if (idx >= TEMP_SENSOR_COUNT) idx = 0;
  return g_readings[idx].present;
}

const TemperatureReading& temperatureGetReading(TempSensor sensor) {
  size_t idx = sensorIndex(sensor);
  if (idx >= TEMP_SENSOR_COUNT) idx = 0;
  return g_readings[idx];
}

bool temperatureAnyFault() {
  for (size_t idx = 0; idx < TEMP_SENSOR_COUNT; ++idx) {
    const TemperatureReading& reading = g_readings[idx];
    if (!reading.present || !reading.valid) {
      return true;
    }
  }
  return false;
}
