# Temperature Sensors (DS18B20) — Configuration

The ROM addresses for each sensor are defined in `firmware/control/config.h`.

| Sensor | Function | ROM Address |
|:--|:--|:--:|
| T_hot | Hot inlet | 0x288DAC1200000093 |
| T_cold | Cold inlet | 0x282C4F120000006C |
| T_out | Outlet | 0x28594D120000009C |

> Discovered using `firmware/tools/m1_temp_scan/m1_temp_scan.ino`.
