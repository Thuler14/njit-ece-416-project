/*
 * ================================================================
 *  Module: pid
 *  Purpose: PID controller used to drive the hot/cold valve mix
 *           ratio. Integrates error, computes a derivative term,
 *           and clamps output to the allowed range (windup guard).
 *
 *  Dependencies:
 *    - Arduino.h (constrain helper)
 *
 *  Interface:
 *    PID(float kp, float ki, float kd, float minOut, float maxOut);
 *    float update(float error, float dtSeconds);
 *    void reset();
 *    void setGains(float kp, float ki, float kd);
 *    void setOutputLimits(float minOut, float maxOut);
 * ================================================================
 */

#pragma once

#include <Arduino.h>

// PID controller used to drive the hot/cold valve mix ratio.
class PID {
 public:
  PID(float kp, float ki, float kd, float minOut, float maxOut);

  // Update controller state with new error measurement and timestep (seconds).
  float update(float error, float dtSeconds);

  // Last output returned by update()
  float lastOutput() const { return lastOutputValue; }

  float getKp() const { return Kp; }
  float getKi() const { return Ki; }

  // Reset integrator to zero (useful when disabling control loop).
  void reset();

  void setGains(float kp, float ki, float kd);
  void setOutputLimits(float minOut, float maxOut);

 private:
  float Kp;
  float Ki;
  float Kd;
  float integral;
  float outMin;
  float outMax;
  float prevError;
  bool hasPrevError;
  float lastOutputValue;
};
