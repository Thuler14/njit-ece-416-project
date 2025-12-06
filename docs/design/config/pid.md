# PID Controller — Configuration

## Overview

- PI(D) controller drives the hot/cold valve mix ratio to hold the outlet temperature at the setpoint.  
- PID configuration is defined in [`firmware/control/config.h`](../../../firmware/control/config.h).

---

## Control Gains and Limits

| Parameter | Description | Value |
|:--|:--|:--:|
| `PID_KP` | Proportional gain | `0.10` |
| `PID_KI` | Integral gain (per second) | `0.08` |
| `PID_KD` | Derivative gain | `0.00` |
| `PID_OUT_MIN` | Output lower bound (mix ratio) | `0.0` |
| `PID_OUT_MAX` | Output upper bound (mix ratio) | `1.0` |

---

## Notes

- Integrator is clamped to `[PID_OUT_MIN, PID_OUT_MAX]` to limit windup.  
- Timing uses measured loop intervals (`millis()` delta) with a 12 ms target delay; retune if the loop rate changes.  
