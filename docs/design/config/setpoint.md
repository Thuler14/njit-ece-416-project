# Temperature Setpoint — Configuration

## Overview

- Defines default, safety, and preset temperature values for system operation.  
- Values are expressed in **degrees Fahrenheit (°F)**.  
- Configuration is defined in [`firmware/ui/config.h`](../../../firmware/common/config.h).

---

## Setpoint Constants

| Constant | Description | Value (°F) |
|:--|:--|:--:|
| `SETPOINT_DEFAULT_F` | Default setpoint at startup | `100.0` |
| `SETPOINT_MIN_F` | Minimum safety limit | `80.0` |
| `SETPOINT_MAX_F` | Maximum safety limit | `120.0` |
| `SETPOINT_STEP_F` | Adjustment step size | `1.0` |
| `SETPOINT_PRESET_A_F` | Preset A value | `98.0` |
| `SETPOINT_PRESET_B_F` | Preset B value | `105.0` |

---

## Notes

- All limits are enforced in firmware to prevent unsafe temperature selection.  
- Preset A and B provide quick access to preferred comfort temperatures.
