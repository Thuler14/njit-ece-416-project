// ====================================================
// M1: Push Button Read Test (5-Button UI)
// Purpose: Demonstrate reading 5 push buttons (▲ ▼ ● A B) with debounce,
//          printing each label once per press over Serial (115200)
// Board: ESP32 (UI Module)
// ====================================================

#include "../../ui/config.h"

void setup() {
  Serial.begin(115200);
  delay(50);
  for (auto& b : buttons) {
    pinMode(b.pin, INPUT_PULLUP);  // active-low
    b.lastStable = true;           // released
    b.t = millis();
  }
}

void loop() {
  const unsigned long now = millis();
  for (auto& b : buttons) {
    bool rawPressed = (digitalRead(b.pin) == LOW);  // LOW = pressed
    if (rawPressed != b.lastStable) {
      if (now - b.t >= DEBOUNCE_MS) {
        b.lastStable = rawPressed;
        if (rawPressed) Serial.println(b.label);  // print on press only
        b.t = now;                                // reset timer
      }
    } else {
      b.t = now;
    }
  }
}
