// ====================================================
// M2: Outlet Temperature Test (DS18B20 + EMA Filter)
// Purpose: Verify stable 100 ms read loop and Exponential Moving Average (EMA) smoothing
// Board: ESP32 (Control Module)
// Sensor: OUTLET DS18B20, resolution = 9-bit (~94 ms conversion)
// Bus: GPIO 4 with 4.7kΩ pull-up to 3V3
// ====================================================

#include <DallasTemperature.h>
#include <OneWire.h>
#include <math.h>

#include "../../control/config.h"  // Centralized configuration

OneWire oneWire(TEMP_PIN_ONEWIRE);
DallasTemperature sensors(&oneWire);

// === STATE ===
uint32_t t0_ms = 0;
uint32_t next_ms = 0;
bool have_outlet = false;
uint32_t last_req_ms = 0;

bool first_sample = true;
float T_out_raw = NAN;
float T_out_filt = NAN;

void setup() {
  Serial.begin(115200);
  delay(200);

  sensors.begin();
  sensors.setWaitForConversion(false);  // non-blocking
  sensors.setCheckForConversion(true);  // allow isConversionComplete()

  // Use the configured OUTLET sensor address directly (no scanning)
  if (sensors.isConnected(TEMP_OUT_ADDR)) {
    have_outlet = true;
    sensors.setResolution(TEMP_OUT_ADDR, TEMP_RESOLUTION);

    // Start first conversion immediately (if sensor present)
    sensors.requestTemperaturesByAddress(TEMP_OUT_ADDR);
    last_req_ms = millis();

    // Align cadence to start and print CSV header
    t0_ms = last_req_ms;
    next_ms = t0_ms;

    // CSV header (explicit units)
    Serial.println(F("t_ms,T_out_raw,T_out_filt"));
  } else {
    // Print only once if missing, then stay silent
    Serial.println(F("# ERROR: OUTLET DS18B20 (TEMP_OUT_ADDR) not found on bus"));
  }
}

void loop() {
  if (!have_outlet) return;  // silent if missing

  const uint32_t now = millis();

  // Pace at 100 ms
  if ((int32_t) (now - next_ms) < 0) return;
  next_ms += TEMP_LOOP_DT_MS;

  // Check conversion ready
  bool ready = sensors.isConversionComplete() || (now - last_req_ms >= TEMP_CONVERSION_TIME_MS);

  if (ready) {
    float Tc = sensors.getTempC(TEMP_OUT_ADDR);

    // Validate reading explicitly (skip NaN and DEVICE_DISCONNECTED_C)
    bool ok = (!isnan(Tc)) && (Tc != DEVICE_DISCONNECTED_C) && (Tc > TEMP_MIN_VALID_C) && (Tc < TEMP_MAX_VALID_C);

    if (ok) {
      // Convert to Fahrenheit
      T_out_raw = DallasTemperature::toFahrenheit(Tc);

      // EMA update (seed on first valid sample)
      if (first_sample || isnan(T_out_filt)) {
        T_out_filt = T_out_raw;
        first_sample = false;
      } else {
        T_out_filt += TEMP_EMA_ALPHA * (T_out_raw - T_out_filt);
      }

      // Print CSV line using RELATIVE time (t_ms)
      const uint32_t t_ms = now - t0_ms;
      Serial.print(t_ms);
      Serial.print(F(","));
      Serial.print(T_out_raw, 3);
      Serial.print(F(","));
      Serial.println(T_out_filt, 3);
    }

    // Always re-request the next conversion so we keep sampling
    sensors.requestTemperaturesByAddress(TEMP_OUT_ADDR);
    last_req_ms = now;
  }

  delay(1);  // small idle to reduce busy waiting
}
