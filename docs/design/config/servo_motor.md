# Servo Motors (MG-996R) — Configuration

## Overview

- Two **MG996R servo motors** are used to control the cold and hot water valves.  
- Each servo is calibrated for fully closed and fully open positions, with pulse widths defined in [`firmware/control/config.h`](../../../firmware/control/config.h).
- The calibration ensures consistent mechanical limits and prevents overtravel.

---

## Hardware Constants

| Constant | Description | Value |
|:--|:--|:--:|
| `SERVO_PIN_HOT` | GPIO pin for hot valve servo | `19` |
| `SERVO_PIN_COLD` | GPIO pin for cold valve servo | `18` |

## Calibration Constants

| Constant | Description | Typical Value (µs) |
|:--|:--|:--:|
| `SERVO_HOT_MIN_US` | Hot valve — fully **open** position | `1180` |
| `SERVO_HOT_MAX_US` | Hot valve — fully **closed** position | `2080` |
| `SERVO_COLD_MIN_US` | Cold valve — fully **open** position | `1120` |
| `SERVO_COLD_MAX_US` | Cold valve — fully **closed** position | `2080` |
| `SERVO_GUARD_US` | Safety margin from mechanical stop | `15` |

> Calibrated using [`firmware/tools/servo_calibration/m1_servo_calibration.ino`](../../../firmware/tools/m1_servo_calibration/m1_servo_calibration.ino).

---

## Notes

- Guard offsets prevent mechanical stress by reducing servo travel slightly before reaching the physical stop.
- The calibrated limits correspond to real valve endpoints; any adjustments should be re-measured and updated in `config.h`.
