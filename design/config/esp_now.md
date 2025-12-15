# ESP-NOW Communication — Configuration

## Overview

- The system uses **ESP-NOW** for direct, low-latency wireless communication between the **Control** and **UI** modules.
- Both ESP32 boards operate in **Station (STA)** mode on a fixed Wi-Fi channel to maintain a stable one-to-one link.
- MAC addresses and encryption keys are configured in [`firmware/config/config.h`](../../firmware/common/config.h).

---

## MAC Addresses

| Constant | Device | Value |
|:--|:--|:--:|
| `COMM_CTRL_MAC` | Control unit | `3C:8A:1F:80:A9:D4` |
| `COMM_UI_MAC` | UI unit | `8C:4F:00:35:9B:F4` |

> Discovered using [`firmware/tools/m2_mac_scan/m2_mac_scan.ino`](../../firmware/tools/m2_mac_scan/m2_mac_scan.ino).

---

## Communication Constants

| Constant | Description | Value |
|:--|:--|:--:|
| `COMM_CHANNEL` | Wi-Fi channel used for both boards | `6` |
| `COMM_USE_ENCRYPTION` | Enable/disable ESP-NOW encryption (`true`/`false`) | `false` |

---

## Encryption Keys

| Constant | Description | Value |
|:--|:--|:--:|
| `COMM_PMK` | Primary Master Key (global) | `showerctrl_pmk16` |
| `COMM_LMK` | Local Master Key (per-peer key) | `static_lmk_uictr` |

---

## Protocol

| Constant | Description | Value |
|:--|:--|:--:|
| `COMM_PROTOCOL_VERSION` | Protocol version | `1` |
| `COMM_FLAG_ACK` | Acknowledgment bit | `1 << 0` |
| `COMM_FLAG_RUN` | Run/Stop bit | `1 << 1` |
| `COMM_FLAG_ERR` | Error bit | `1 << 2` |

---

## Data Structure

`COMM_Payload` defines the transmitted packet containing timestamp, sequence number, setpoint, and flag bits used for control and acknowledgment.

---

## Notes
- Each board uses the **other board’s MAC address** as its peer.
  - UI → `CTRL_MAC`  
  - Control → `UI_MAC`
- Both must share the same **channel** and, if encryption is enabled, the **same PMK/LMK pair**.
