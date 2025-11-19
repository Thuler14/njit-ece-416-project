#include "pid.h"

PID::PID(float kp, float ki, float minOut, float maxOut)
    : Kp(kp), Ki(ki), integral(0.0f), outMin(minOut), outMax(maxOut) {}

float PID::update(float error, float dtSeconds) {
  integral += error * Ki * dtSeconds;
  integral = constrain(integral, outMin, outMax);
  float output = Kp * error + integral;
  return constrain(output, outMin, outMax);
}

void PID::reset() {
  integral = 0.0f;
}

void PID::setGains(float kp, float ki) {
  Kp = kp;
  Ki = ki;
}

void PID::setOutputLimits(float minOut, float maxOut) {
  outMin = minOut;
  outMax = maxOut;
  integral = constrain(integral, outMin, outMax);
}
