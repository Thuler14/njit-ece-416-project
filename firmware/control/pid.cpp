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
  integral += error * Ki * dtSeconds;
  integral = constrain(integral, outMin, outMax);

  float derivative = 0.0f;
  if (dtSeconds > 0.0f && hasPrevError) {
    derivative = (error - prevError) / dtSeconds;
  }
  prevError = error;
  hasPrevError = true;

  float output = Kp * error + integral + Kd * derivative;
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
