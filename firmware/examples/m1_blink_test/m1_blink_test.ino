// ====================================================
// M1: Blink Test (ESP32)
// Purpose: Verify basic board operation by blinking the onboard LED (GPIO 2)
//          and printing "Hello World!" to Serial at 1 Hz
// Board: ESP32 (Control + UI Module)
// ====================================================

#include <Arduino.h>

#define LED_PIN 2  // Onboard LED pin

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  Serial.println("Hello World!");
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
