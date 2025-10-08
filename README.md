# Automatic Temperature-Controlled Shower

ESP32-based prototype that mixes hot/cold water to a user-selected setpoint using temperature sensors, a servo-driven valve, and a simple UI. Built for NJIT ECE 416.

## Quick Start (Arduino IDE)
1. Arduino IDE → Boards Manager → **esp32 by Espressif Systems**  
2. Libraries: **OneWire**, **DallasTemperature**, **ESP32Servo**, **U8g2**
3. Open `firmware/control/` and `firmware/ui/` sketches in separate IDE windows.
4. Upload to two ESP32 boards:
   - **Control**: sensors + PID + servo
   - **UI**: push button + OLED + ESP-NOW sender
5. Set MAC of control board in the UI sketch; set DS18B20 addresses in control sketch.

## Repository Structure
```text
njit-ece-416-project/
├─ firmware/
│  ├─ control/          # ESP32: sensors, PID, servo
│  └─ ui/               # ESP32: push button, OLED, ESP-NOW
├─ mechanical/
│  ├─ cad/              # 3D models, mounts, enclosure
│  └─ photos/           # Assembly & testbed images
├─ tests/
│  ├─ data/             # CSV logs (temp, flow, servo_us, setpoint)
│  ├─ scripts/          # Analysis helpers (optional)
│  └─ reports/          # Validation results & plots
└─ docs/
   ├─ design/           # Wiring diagrams, block diagram, pin maps
   ├─ proposal/         # Original proposal files
   └─ final-report/     # Final write-up & slides
```

## Safety
- Max outlet temp **120 °F** (hard limit in firmware).  
- Link-loss or sensor fault → system defaults to **cold-safe**.  
- Hardware E-stop cuts servo power and forces valve closed. 
