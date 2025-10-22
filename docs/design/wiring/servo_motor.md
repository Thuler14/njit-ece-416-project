# Servo Motors (Cold / Hot Valves) — Wiring

Two MG996R servos are used to control the cold and hot water valves.

**Power:** 6 V DC supply (≥ 3 A) shared by both servos.  
**Common ground:** Connect 6 V GND to ESP32 GND.

| Servo | Signal | ESP32 Pin |
|:--|:--|:--:|
| Cold Valve | PWM | GPIO 18 |
| Hot Valve | PWM | GPIO 19 |
| Both | V+ | 6 V |
| Both | GND | GND |

> 📷 Photo: `mechanical/photos/m1_servo_motor_wiring.jpg`
