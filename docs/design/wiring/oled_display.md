# OLED Display (SSD1309 SPI Blue) — Wiring

The **128×64 px SSD1309** OLED connects via a ESP32’s **VSPI bus** using a **4-wire SPI interface** compatible with **U8g2**.

- **Bus type:** VSPI (SPI 4-wire + CS)

| OLED Pin | Signal | ESP32 Pin |
|:--|:--|:--:|
| 1 | GND | GND |
| 2 | VCC | 3V3 |
| 3 | SCK | GPIO 18 |
| 4 | SDA / MOSI | GPIO 23 |
| 5 | RES / RST | GPIO 4 |
| 6 | DC | GPIO 22 |
| 7 | CS | GPIO 5 |

> 📷 Photo: `mechanical/photos/m1_oled_display_wiring.jpg`
