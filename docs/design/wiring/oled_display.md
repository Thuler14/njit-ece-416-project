# OLED Display (SSD1309 SPI Blue) — Wiring

The OLED display (SSD1309) uses a 4-wire SPI connection handled by the **U8g2** library on the ESP32’s VSPI bus.

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
