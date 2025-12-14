#include "pid.h"

PID::PID(float kp, float ki, float kd, float minOut, float maxOut)
    : Kp(kp),
      Ki(ki),
      Kd(kd),
      integral(0.0f),
      outMin(minOut),
      outMax(maxOut),
      prevError(0.0f),
      hasPrevError(false),
      lastOutputValue(0.0f) {}

float PID::update(float error, float dtSeconds) {
  // Reset integrator on zero-crossing to limit overshoot around setpoint
  if (hasPrevError && (error * prevError) < 0.0f) {
    integral = 0.0f;
  }

  float derivative = 0.0f;
  if (dtSeconds > 0.0f && hasPrevError) {
    derivative = (error - prevError) / dtSeconds;
  }

  // Conditional integration (anti-windup)
  float proposedIntegral = integral + error * Ki * dtSeconds;
  proposedIntegral = constrain(proposedIntegral, outMin, outMax);

  float provisionalOutput = Kp * error + proposedIntegral + Kd * derivative;

  // If output would saturate and integration would push further into saturation, freeze integrator
  bool saturatingHigh = (provisionalOutput > outMax) && (error > 0.0f);
  bool saturatingLow = (provisionalOutput < outMin) && (error < 0.0f);
  if (!saturatingHigh && !saturatingLow) {
    integral = proposedIntegral;
  }

  float output = Kp * error + integral + Kd * derivative;

  prevError = error;
  hasPrevError = true;

  lastOutputValue = constrain(output, outMin, outMax);
  return lastOutputValue;
}

void PID::reset() {
  integral = 0.0f;
  prevError = 0.0f;
  hasPrevError = false;
  lastOutputValue = 0.0f;
}

void PID::setGains(float kp, float ki, float kd) {
  Kp = kp;
  Ki = ki;
  Kd = kd;
}

void PID::setOutputLimits(float minOut, float maxOut) {
  outMin = minOut;
  outMax = maxOut;
  integral = constrain(integral, outMin, outMax);
}
