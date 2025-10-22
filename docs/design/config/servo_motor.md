# Servo Motor (MG-996R) — Configuration

The calibrated µs limits for both valves are defined in `firmware/control/config.h`.

| Servo | Function | Open (µs) | Closed (µs) |
|:--|:--|:--:|:--:|
| Cold Valve | Cold water control | 1120 | 2080 |
| Hot Valve | Hot water control | 1180 | 2080 |

**Guard Offset:** `SERVO_GUARD_US = 15 µs`

> Calibrated using `firmware/tools/servo_calibration/servo_calibration.ino`.
