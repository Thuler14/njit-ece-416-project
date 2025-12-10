#include "flow_sensor.h"

#include <Arduino.h>
#include <math.h>

#include "config.h"

static volatile uint32_t s_pulseCount = 0;
static uint32_t s_lastCount = 0;
static uint32_t s_lastWindowMs = 0;
static float s_filteredHz = 0.0f;
static float s_filteredLpm = 0.0f;
static FlowReading s_lastReading{0.0f, 0.0f, 0.0f, 0.0f, 0, false};
static bool s_initialized = false;

// ISR: increment pulse count on rising edge
static void IRAM_ATTR onFlowPulse() {
  s_pulseCount++;
}

bool flowSensorInit() {
  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), onFlowPulse, RISING);

  const uint32_t now = millis();
  s_lastWindowMs = now;
  s_lastReading.sampleMs = now;
  s_lastReading.fresh = false;
  s_initialized = true;
  return true;
}

bool flowSensorUpdate() {
  if (!s_initialized) return false;

  const uint32_t now = millis();
  const uint32_t windowMs = now - s_lastWindowMs;
  if (windowMs < FLOW_WINDOW_MS) {
    return false;
  }

  noInterrupts();
  const uint32_t count = s_pulseCount;
  interrupts();

  const uint32_t delta = count - s_lastCount;
  s_lastCount = count;
  s_lastWindowMs = now;

  const uint32_t dtMs = windowMs == 0 ? 1 : windowMs;
  const float hzRaw = (delta == 0) ? 0.0f : (1000.0f * delta) / (float) dtMs;
  const float lpmRaw = (delta == 0)
                           ? 0.0f
                           : (hzRaw * 60.0f) / (FLOW_K_PULSES_PER_ML * 1000.0f);

  if (s_lastReading.sampleMs == 0) {
    // Seed the filter on the first measurement to avoid startup bias.
    s_filteredHz = hzRaw;
    s_filteredLpm = lpmRaw;
  } else {
    float alpha = 1.0f - expf(-(float) dtMs / (float) FLOW_FILTER_TAU_MS);
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    s_filteredHz += alpha * (hzRaw - s_filteredHz);
    s_filteredLpm += alpha * (lpmRaw - s_filteredLpm);
  }

  FlowReading reading{};
  reading.sampleMs = now;
  reading.fresh = true;
  reading.Hz = s_filteredHz;
  reading.HzRaw = hzRaw;
  reading.lpm = s_filteredLpm;
  reading.lpmRaw = lpmRaw;

  s_lastReading = reading;
  return true;
}

FlowReading flowSensorGet() {
  return s_lastReading;
}
