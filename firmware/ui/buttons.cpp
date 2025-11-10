#include "buttons.h"

// Global list of button descriptors (active-low inputs)
Btn BTN_LIST[BTN_COUNT] = {
    {"▲", BTN_PIN_UP, true, 0},
    {"▼", BTN_PIN_DOWN, true, 0},
    {"●", BTN_PIN_OK, true, 0},
    {"A", BTN_PIN_A, true, 0},
    {"B", BTN_PIN_B, true, 0},
};

void buttonsInit() {
  const unsigned long now = millis();
  for (auto& b : BTN_LIST) {
    pinMode(b.pin, INPUT_PULLUP);
    b.lastStable = true;
    b.t = now;
  }
}

bool buttonIsDown(BtnIndex idx) {
  if (idx >= BTN_COUNT) return false;
  return digitalRead(BTN_LIST[idx].pin) == LOW;
}

bool buttonPressed(BtnIndex idx) {
  if (idx >= BTN_COUNT) return false;

  Btn& b = BTN_LIST[idx];
  const unsigned long now = millis();
  const bool pressed = (digitalRead(b.pin) == LOW);

  // Simple debounce: state must remain stable for BTN_DEBOUNCE_MS
  if (pressed != b.lastStable) {
    if (now - b.t >= BTN_DEBOUNCE_MS) {
      b.lastStable = pressed;
      b.t = now;
      return pressed;
    }
  } else {
    b.t = now;
  }
  return false;
}
