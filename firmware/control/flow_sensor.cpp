#include "flow_sensor.h"

#include <Arduino.h>

#include "config.h"

static volatile uint32_t s_pulseCount = 0;
static uint32_t s_lastCount = 0;
static uint32_t s_lastWindowMs = 0;
static FlowReading s_lastReading{0.0f, 0.0f, 0, false};
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
  if ((now - s_lastWindowMs) < FLOW_WINDOW_MS) {
    return false;
  }

  noInterrupts();
  const uint32_t count = s_pulseCount;
  interrupts();

  const uint32_t delta = count - s_lastCount;
  s_lastCount = count;
  s_lastWindowMs = now;

  FlowReading reading{};
  reading.sampleMs = now;
  reading.fresh = true;

  if (delta == 0) {
    reading.Hz = 0.0f;
    reading.lpm = 0.0f;
  } else {
    reading.Hz = (1000.0f * delta) / (float) FLOW_WINDOW_MS;
    reading.lpm = (reading.Hz * 60.0f) / (FLOW_K_PULSES_PER_ML * 1000.0f);
  }

  s_lastReading = reading;
  return true;
}

FlowReading flowSensorGet() {
  return s_lastReading;
}
