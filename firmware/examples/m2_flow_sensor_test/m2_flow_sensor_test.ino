// ====================================================
// M2: Flow Sensor Test (YF-S201 + Interrupt)
// Purpose: Verify accurate pulse counting → Hz → L/min (uncalibrated)
// Board: ESP32 (Control Module)
// Sensor: YF-S201 (open-collector; powered at 5V)
// Bus: GPIO 21 with 10kΩ pull-up to 3V3 (RISING-edge interrupt)
// ====================================================

#define FLOW_PIN 21
#define FLOW_K_PULSES_PER_ML 7.5f
#define FLOW_WINDOW_MS 500

volatile uint32_t g_pulseCount = 0;
static uint32_t t0_ms = 0;      // Start time for relative timestamps
static uint32_t lastMs = 0;     // Last window timestamp
static uint32_t lastCount = 0;  // Last pulse count snapshot

void IRAM_ATTR onFlowPulse() {
  g_pulseCount++;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), onFlowPulse, RISING);

  t0_ms = millis();
  lastMs = t0_ms;

  Serial.println(F("t_ms,Hz,L_per_min"));  // CSV header
}

void loop() {
  const uint32_t now = millis();
  if ((now - lastMs) < FLOW_WINDOW_MS) {
    delay(1);
    return;
  }

  noInterrupts();
  const uint32_t count = g_pulseCount;
  interrupts();

  const uint32_t delta = count - lastCount;
  lastCount = count;
  lastMs += FLOW_WINDOW_MS;

  // If no pulses were detected, assume sensor disconnected or no flow → skip print
  if (delta == 0) {
    return;
  }

  const float Hz = (1000.0f * delta) / (float) FLOW_WINDOW_MS;
  const float L_per_min = (Hz * 60.0f) / (FLOW_K_PULSES_PER_ML * 1000.0f);
  const uint32_t t_ms = now - t0_ms;

  Serial.print(t_ms);
  Serial.print(',');
  Serial.print(Hz, 3);
  Serial.print(',');
  Serial.println(L_per_min, 4);
}
