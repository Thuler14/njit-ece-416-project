// ====================================================
// M1: Push Button Read Test (5-Button UI)
// Purpose: Demonstrate reading 5 push buttons (▲ ▼ ● A B) with debounce,
//          printing each label once per press over Serial (115200)
// Board: ESP32 (UI Module)
// ====================================================

#define BTN_DEBOUNCE_MS 20

struct Btn {
  const char* label;
  uint8_t pin;
  bool lastStable;
  unsigned long t;
};

static Btn BTN_LIST[] = {
    {"▲", 25, true, 0},
    {"▼", 26, true, 0},
    {"●", 27, true, 0},
    {"A", 14, true, 0},
    {"B", 13, true, 0},
};

void setup() {
  Serial.begin(115200);
  delay(50);
  for (auto& b : BTN_LIST) {
    pinMode(b.pin, INPUT_PULLUP);  // active-low
    b.lastStable = true;           // released
    b.t = millis();
  }
}

void loop() {
  const unsigned long now = millis();
  for (auto& b : BTN_LIST) {
    bool rawPressed = (digitalRead(b.pin) == LOW);  // LOW = pressed
    if (rawPressed != b.lastStable) {
      if (now - b.t >= BTN_DEBOUNCE_MS) {
        b.lastStable = rawPressed;
        if (rawPressed) Serial.println(b.label);  // print on press only
        b.t = now;                                // reset timer
      }
    } else {
      b.t = now;
    }
  }
}
