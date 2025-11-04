# Push Buttons — Configuration

## Overview

- Five **momentary pushbuttons** provide local user input for temperature control and preset functions.
- Each button is connected to an **ESP32 GPIO** with **internal pull-ups enabled** (active-low).
- Button configuration is defined in [`firmware/ui/config.h`](../../../firmware/ui/config.h).

---

## Hardware Constants

| Constant | Description | Value |
|:--|:--|:--:|
| `BTN_PIN_UP` | Increase setpoint (▲) | `25` |
| `BTN_PIN_DOWN` | Decrease setpoint (▼) | `26` |
| `BTN_PIN_OK` | Run/Stop toggle (●) | `27` |
| `BTN_PIN_A` | Preset A | `14` |
| `BTN_PIN_B` | Preset B | `13` |

---

## Timing Constant

| Constant | Description | Value |
|:--|:--|:--:|
| `BTN_DEBOUNCE_MS` | Debounce delay in milliseconds | `20` |

---

## Notes

- Buttons are **active-low**, meaning a press reads `LOW`.
- Presets A and B correspond to user-defined temperature shortcuts.
