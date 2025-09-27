# Temperature Sensors (DS18B20) — Wiring

Three DS18B20 temperature sensors are used to measure the hot, cold, and mixed water temperatures.

**Pull-up:** One 4.7 kΩ resistor between **GPIO 4** and **3V3**

| Wire Color | Signal | ESP32 Pin |
|:--|:--|:--:|
| Yellow | DQ | GPIO 4 |
| Red | VDD | 3V3 |
| Blue | GND | GND |

> 📷 Photo: `mechanical/photos/m1_temp_sensor_wiring.jpg`
