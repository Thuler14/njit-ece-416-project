# OLED Display — Configuration

## Overview

- The system uses a **128×64 px SSD1309** OLED driven over **SPI** using the **U8g2** graphics library.
- It shows setpoint, run state, and status indicators for transmission activity.
- Display configuration is defined in [`firmware/ui/config.h`](../../../firmware/ui/config.h).

---

## Hardware Constants

| Constant | Description | Value |
|:--|:--|:--:|
| `OLED_PIN_CS` | Chip Select (CS) | `5` |
| `OLED_PIN_DC` | Data/Command (DC) | `22` |
| `OLED_PIN_RST` | Reset (RST) | `4` |

---

## U8g2 Setup (firmware)

- Rotation: `U8G2_R0` (no rotation)
- Constructor: `U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI`

---

## Notes

- Operates on 3.3 V logic.  
- Use full-buffer (`_F_`) U8g2 variants for stable screen refresh. 
