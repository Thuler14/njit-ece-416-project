#include "valve_mix.h"

#include <ESP32Servo.h>
#include <math.h>

#include "config.h"

static Servo sHot, sCold;
static bool sAttached = false;

// Soft limits (guard band applied around mechanical min/max)
static int hotSoftMin, hotSoftMax;
static int coldSoftMin, coldSoftMax;

// Last commanded pulse widths (for logging/tests)
static int g_lastHot = (SERVO_HOT_MIN_US + SERVO_HOT_MAX_US) / 2;
static int g_lastCold = (SERVO_COLD_MIN_US + SERVO_COLD_MAX_US) / 2;

// Linear interpolation between two pulse widths
static inline int lerp_us(int a, int b, float t) {
  return (int) lroundf(a + (b - a) * t);
}

void valveMixInit() {
  const int hMin = min(SERVO_HOT_MIN_US, SERVO_HOT_MAX_US);
  const int hMax = max(SERVO_HOT_MIN_US, SERVO_HOT_MAX_US);
  const int cMin = min(SERVO_COLD_MIN_US, SERVO_COLD_MAX_US);
  const int cMax = max(SERVO_COLD_MIN_US, SERVO_COLD_MAX_US);

  hotSoftMin = hMin + SERVO_GUARD_US;
  hotSoftMax = hMax - SERVO_GUARD_US;
  coldSoftMin = cMin + SERVO_GUARD_US;
  coldSoftMax = cMax - SERVO_GUARD_US;

  sHot.setPeriodHertz(50);
  sCold.setPeriodHertz(50);
  sHot.attach(SERVO_PIN_HOT);
  sCold.attach(SERVO_PIN_COLD);
  sAttached = true;

  valveMixCloseAll();
}

void valveMixCloseAll() {
  if (!sAttached) return;

  sHot.writeMicroseconds(SERVO_HOT_MAX_US);
  sCold.writeMicroseconds(SERVO_COLD_MAX_US);

  g_lastHot = SERVO_HOT_MAX_US;
  g_lastCold = SERVO_COLD_MAX_US;
}

void applyMixRatio(float ratio) {
  if (!sAttached) return;

  const float r = constrain(ratio, 0.0f, 1.0f);

  // HOT: r=0 → MAX (closed), r=1 → MIN (open)
  int hotTarget = lerp_us(SERVO_HOT_MAX_US, SERVO_HOT_MIN_US, r);
  // COLD: r=0 → MIN (open),   r=1 → MAX (closed)
  int coldTarget = lerp_us(SERVO_COLD_MIN_US, SERVO_COLD_MAX_US, r);

  hotTarget = constrain(hotTarget, hotSoftMin, hotSoftMax);
  coldTarget = constrain(coldTarget, coldSoftMin, coldSoftMax);

  sHot.writeMicroseconds(hotTarget);
  sCold.writeMicroseconds(coldTarget);
  g_lastHot = hotTarget;
  g_lastCold = coldTarget;
}

int lastHotUs() { return g_lastHot; }
int lastColdUs() { return g_lastCold; }
