# ESP-NOW Communication — Configuration

## Overview

- The system uses **ESP-NOW** for direct, low-latency wireless communication between the **Control** and **UI** modules.
- Both ESP32 boards operate in **Station (STA)** mode on a fixed Wi-Fi channel to maintain a stable one-to-one link.
- MAC addresses and encryption keys are configured in [`firmware/config/config.h`](../../../firmware/config/config.h).

---

## Hardware Constants

| Constant | Description | Typical Value |
|:--|:--|:--:|
| `COMM_CTRL_MAC` | Control unit’s STA MAC address | `0x3C8A1F80A9D4` |
| `COMM_UI_MAC` | UI unit’s STA MAC address | `0x8C4F00359BF4` |

> Discovered using [`firmware/tools/m2_mac_scan/m2_mac_scan.ino`](../../../firmware/tools/m2_mac_scan/m2_mac_scan.ino).

---

## Firmware Constants

| Constant | Description | Typical Value |
|:--|:--|:--:|
| `COMM_CHANNEL` | Wi-Fi channel used for both boards | `6` |
| `COMM_USE_ENCRYPTION` | Enable/disable ESP-NOW encryption (`1 = ON`, `0 = OFF`) | `0` |
| `COMM_PMK` | Primary Master Key (global) | `showerctrl_pmk16` |
| `COMM_LMK` | Local Master Key (per-peer key) | `static_lmk_uictr` |

---

## Notes
- Each board uses the **other board’s MAC address** as its peer.
  - UI → `CTRL_MAC`  
  - Control → `UI_MAC`
- Both must share the same **channel** and, if encryption is enabled, the **same PMK/LMK pair**.
