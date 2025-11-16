#include "buttons.h"

static const uint8_t btnPins[BUTTON_COUNT] = {
    BTN_PIN_UP, BTN_PIN_DOWN, BTN_PIN_OK, BTN_PIN_A, BTN_PIN_B};

// Per-button feature configuration
struct ButtonFeatures {
  bool click;
  bool dblClick;
  bool longPress;
  bool repeat;
};

static constexpr ButtonFeatures kFeatures[BUTTON_COUNT] = {
    /* UP   */ {true, false, true, true},
    /* DOWN */ {true, false, true, true},
    /* OK   */ {false, true, true, false},
    /* A    */ {false, false, true, false},
    /* B    */ {false, false, true, false},
};

static inline bool hasClick(uint8_t id) { return kFeatures[id].click; }
static inline bool hasDbl(uint8_t id) { return kFeatures[id].dblClick; }
static inline bool hasLong(uint8_t id) { return kFeatures[id].longPress; }
static inline bool hasRepeat(uint8_t id) { return kFeatures[id].repeat; }

struct BtnState {
  bool rawDown = false;
  bool stableDown = false;
  unsigned long lastChangeMs = 0;
  unsigned long downStartMs = 0;
  unsigned long nextRepeatMs = 0;
  bool longFired = false;
  bool singleClickPending = false;
  unsigned long lastReleaseMs = 0;
  bool suppressRelease = false;
};

static BtnState btnState[BUTTON_COUNT];
static ButtonsEvents latched;
static bool blockSingles[BUTTON_COUNT] = {false};

struct ChordState {
  ButtonId a, b;
  unsigned long holdMs;
  bool isActive;
  bool hasFired;
  unsigned long startMs;
};

static ChordState chords[] = {
    {BUTTON_UP, BUTTON_DOWN, 2000, false, false, 0},
    {BUTTON_COUNT, BUTTON_COUNT, 0, false, false, 0},
    {BUTTON_COUNT, BUTTON_COUNT, 0, false, false, 0},
};

static inline bool btnIsDown(uint8_t id) {
  return digitalRead(btnPins[id]) == LOW;
}

// Event writers
static void emitClick(uint8_t id) {
  if (blockSingles[id] || !hasClick(id)) return;
  switch (id) {
    case BUTTON_UP:
      latched.upClick = true;
      break;
    case BUTTON_DOWN:
      latched.downClick = true;
      break;
    case BUTTON_OK:
      latched.okClick = true;
      break;
    case BUTTON_A:
      latched.aClick = true;
      break;
    case BUTTON_B:
      latched.bClick = true;
      break;
  }
}

static void emitDbl(uint8_t id) {
  if (blockSingles[id] || !hasDbl(id)) return;
  switch (id) {
    case BUTTON_UP:
      latched.upDblClick = true;
      break;
    case BUTTON_DOWN:
      latched.downDblClick = true;
      break;
    case BUTTON_OK:
      latched.okDblClick = true;
      break;
    case BUTTON_A:
      latched.aDblClick = true;
      break;
    case BUTTON_B:
      latched.bDblClick = true;
      break;
  }
}

static void emitLong(uint8_t id) {
  if (blockSingles[id] || !hasLong(id)) return;
  switch (id) {
    case BUTTON_UP:
      latched.upLong = true;
      break;
    case BUTTON_DOWN:
      latched.downLong = true;
      break;
    case BUTTON_OK:
      latched.okLong = true;
      break;
    case BUTTON_A:
      latched.aLong = true;
      break;
    case BUTTON_B:
      latched.bLong = true;
      break;
  }
}

static void emitRepeat(uint8_t id) {
  if (blockSingles[id] || !hasRepeat(id)) return;
  switch (id) {
    case BUTTON_UP:
      latched.upRepeat = true;
      break;
    case BUTTON_DOWN:
      latched.downRepeat = true;
      break;
    case BUTTON_OK:
      latched.okRepeat = true;
      break;
    case BUTTON_A:
      latched.aRepeat = true;
      break;
    case BUTTON_B:
      latched.bRepeat = true;
      break;
  }
}

static void updateChords(unsigned long now) {
  for (size_t i = 0; i < sizeof(chords) / sizeof(chords[0]); ++i) {
    ChordState& c = chords[i];
    if (c.a >= BUTTON_COUNT || c.b >= BUTTON_COUNT) continue;

    bool both = btnIsDown(c.a) && btnIsDown(c.b);
    if (both) {
      blockSingles[c.a] = blockSingles[c.b] = true;

      if (!c.isActive) {
        c.isActive = true;
        c.hasFired = false;
        c.startMs = now;
      } else if (!c.hasFired && now - c.startMs >= c.holdMs) {
        c.hasFired = true;
        btnState[c.a].suppressRelease = true;
        btnState[c.b].suppressRelease = true;
        if (i == 0) latched.chordStepLong = true;
      }
    } else {
      c.isActive = false;
      c.hasFired = false;
    }
  }
}

static void updateDebounce(uint8_t id, unsigned long now) {
  BtnState& s = btnState[id];
  bool raw = btnIsDown(id);

  if (raw != s.rawDown) {
    s.rawDown = raw;
    s.lastChangeMs = now;
  }

  if (now - s.lastChangeMs < BTN_DEBOUNCE_MS) return;
  if (s.stableDown == s.rawDown) return;

  bool prev = s.stableDown;
  s.stableDown = s.rawDown;

  if (!prev && s.stableDown) {
    s.downStartMs = now;
    s.nextRepeatMs = now + BTN_REPEAT_DELAY_MS;
    s.longFired = false;
  } else if (prev && !s.stableDown) {
    unsigned long pressDur = now - s.downStartMs;

    if (s.suppressRelease) {
      s.suppressRelease = false;
      s.singleClickPending = false;
      return;
    }

    if (!s.longFired && pressDur >= BTN_DEBOUNCE_MS && pressDur < BTN_LONGPRESS_MS) {
      if (hasDbl(id) && BTN_DBLCLICK_MS > 0) {
        if (!s.singleClickPending) {
          s.singleClickPending = true;
          s.lastReleaseMs = now;
        } else if (now - s.lastReleaseMs <= BTN_DBLCLICK_MS) {
          s.singleClickPending = false;
          emitDbl(id);
        } else {
          emitClick(id);
          s.singleClickPending = true;
          s.lastReleaseMs = now;
        }
      } else if (hasClick(id)) {
        emitClick(id);
      }
    }
  }
}

static void updateHeld(uint8_t id, unsigned long now) {
  BtnState& s = btnState[id];
  if (!s.stableDown || s.suppressRelease) return;

  if (!s.longFired && hasLong(id) && now - s.downStartMs >= BTN_LONGPRESS_MS) {
    s.longFired = true;
    emitLong(id);
  }

  if (hasRepeat(id) && BTN_REPEAT_MS > 0 &&
      s.nextRepeatMs && now >= s.nextRepeatMs) {
    s.nextRepeatMs += BTN_REPEAT_MS;
    emitRepeat(id);
  }
}

static void finishPendingSingles(unsigned long now) {
  if (BTN_DBLCLICK_MS == 0) return;

  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    BtnState& s = btnState[i];
    if (!s.singleClickPending || s.suppressRelease) continue;
    if (now - s.lastReleaseMs > BTN_DBLCLICK_MS) {
      s.singleClickPending = false;
      emitClick(i);
    }
  }
}

void buttonsInit() {
  unsigned long now = millis();

  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    pinMode(btnPins[i], INPUT_PULLUP);
    BtnState& s = btnState[i];
    s.rawDown = s.stableDown = btnIsDown(i);
    s.lastChangeMs = now;
    blockSingles[i] = false;
  }

  latched = ButtonsEvents{};
}

bool buttonsPoll(ButtonsEvents& out) {
  unsigned long now = millis();

  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) blockSingles[i] = false;

  updateChords(now);

  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    updateDebounce(i, now);
    updateHeld(i, now);
  }

  finishPendingSingles(now);

  bool any = latched.chordStepLong;
  if (!any) {
    for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
      any |= latched.upClick || latched.upDblClick || latched.upLong || latched.upRepeat;
      any |= latched.downClick || latched.downDblClick || latched.downLong || latched.downRepeat;
      any |= latched.okClick || latched.okDblClick || latched.okLong || latched.okRepeat;
      any |= latched.aClick || latched.aDblClick || latched.aLong || latched.aRepeat;
      any |= latched.bClick || latched.bDblClick || latched.bLong || latched.bRepeat;
    }
  }

  out = latched;
  latched = ButtonsEvents{};
  return any;
}
