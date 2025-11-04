# Flow Sensor (YF-S201) - Configuration

## Overview

- The control module uses one **YF-S201** Hall-effect flow sensor to measure the total outlet flow rate.
- The sensor outputs a digital pulse signal proportional to flow velocity, detected via an interrupt on the ESP32. 
- Configuration constants are defined in [`firmware/control/config.h`](../../../firmware/control/config.h).

---

## Hardware Constants

| Constant | Description | Value |
|:--|:--|:--:|
| `FLOW_PIN` | GPIO used for pulse input | `21` |

---

## Measurement Constants

| Constant | Description | Value |
|:--|:--|:--:|
| `FLOW_K_PULSES_PER_ML` | K-factor — pulses per mL of water (≈7500/L) | `7.5f` |
| `FLOW_WINDOW_MS` | Sampling window for pulse counting (ms) | `500` |

---

## Notes

- The **K-factor (7.5 pulses/mL)** depends on the specific sensor and flow path and will be refined during **M3 calibration**.  
- The current sampling window of **500 ms** balances noise reduction with responsiveness.
