#pragma once

#include <Arduino.h>

// Simple PI (future PID) controller used to drive the hot/cold valve mix ratio.
class PID {
 public:
  PID(float kp, float ki, float minOut, float maxOut);

  // Update controller state with new error measurement and timestep (seconds).
  float update(float error, float dtSeconds);

  // Reset integrator to zero (useful when disabling control loop).
  void reset();

  void setGains(float kp, float ki);
  void setOutputLimits(float minOut, float maxOut);

 private:
  float Kp;
  float Ki;
  float integral;
  float outMin;
  float outMax;
};
