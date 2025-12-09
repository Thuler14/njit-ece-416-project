# Temperature Sensors (DS18B20) - Configuration

## Overview

- The control module uses three DS18B20 digital temperature sensors connected via a shared OneWire bus.  
- Each sensor has a unique 64-bit ROM address and is configured in [`firmware/control/config.h`](../../../firmware/control/config.h).

---

## Hardware Constants 

| Constant | Description | Value |
|:--|:--|:--:|
| `TEMP_PIN_ONEWIRE` | GPIO pin for DS18B20 data line | `4` |
| `TEMP_HOT_ADDR` | Hot inlet sensor | `0x288DAC1200000093` |
| `TEMP_COLD_ADDR` | Cold inlet sensor | `0x282C4F120000006C` |
| `TEMP_OUT_ADDR` | Outlet sensor | `0x284DF1BA000000C0` |

> Discovered using [`firmware/tools/m1_temp_scan/m1_temp_scan.ino`](../../../firmware/tools/m1_temp_scan/m1_temp_scan.ino).

---

## Timing Constants

| Constant | Description | Value |
|:--|:--|:--:|
| `TEMP_RESOLUTION` | DS18B20 resolution (9–12 bits) | `9` |
| `TEMP_CONVERSION_TIME_MS` | Max conversion time for chosen resolution | `94` |
| `TEMP_LOOP_DT_MS` | Loop interval for temperature read (ms) | `100` |

---

## Filtering Constants

| Constant | Description | Value |
|:--|:--|:--:|
| `TEMP_EMA_ALPHA` | EMA smoothing factor (τ ≈ 0.4 s @ 10 Hz) | `0.20f` |
| `TEMP_MIN_VALID_C` | Minimum valid temperature (°C) | `-60.0f` |
| `TEMP_MAX_VALID_C` | Maximum valid temperature (°C) | `125.0f` |

---

## Notes

- ROM addresses are unique identifiers used to bind each DS18B20 to its physical position.  
- Configuration constants apply globally to all temperature sensors unless overridden.  
- 9-bit mode allows a full conversion within the 100 ms loop.
