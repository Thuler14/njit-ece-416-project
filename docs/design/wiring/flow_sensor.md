# Flow Sensor (YF-S201) — Wiring

One **YF-S201** Hall-effect flow sensor is used to measure the total outlet water flow rate.

**Pull-up:** One 10 kΩ resistor between **GPIO 21** and **3V3**

| Wire Color | Signal | ESP32 Pin |
|:--|:--|:--:|
| Yellow | Flow Signal | GPIO 21 |
| Red | VCC | 5V |
| Black | GND | GND |

> 📷 Photo: [`mechanical/photos/m2_flow_sensor_wiring.jpg`](../../../mechanical/photos/m2_flow_sensor_wiring.jpg)
