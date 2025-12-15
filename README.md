# Automatic Temperature-Controlled Shower

ESP32-based prototype that mixes hot/cold water to a user-selected setpoint using temperature sensors, a servo-driven valve pair, and a simple UI. Built for NJIT ECE 416.

## Quick Start (Arduino IDE)
1. Install **esp32 by Espressif Systems** via Boards Manager.
2. Libraries: **OneWire**, **DallasTemperature**, **ESP32Servo**, **U8g2**.
3. Configure firmware:
   - `firmware/common/config.h`: set control/UI MACs (scan with `firmware/tools/m2_mac_scan/`), comm channel, and optional ESP-NOW encryption keys.
   - `firmware/control/config.h`: set DS18B20 addresses (scan with `firmware/tools/m1_temp_scan/`) and servo min/max values (from `firmware/tools/m1_servo_calibration/`).
4. Build/upload in two IDE windows:
   - `firmware/control/control.ino` → Control unit (sensors, PID, servos, flow, E-stop).
   - `firmware/ui/ui.ino` → UI unit (buttons, OLED, presets, ESP-NOW sender).
5. Power both boards; UI shows outlet temp and link status. Toggle run/stop from the UI. Serial output on the control unit emits CSV logs (10 Hz) when `PID_LOG_CSV` is enabled.

## Data Logging & Plots
- Control firmware prints CSV logs over USB. Capture to file with your serial monitor or use `tests/scripts/m2_logger_live_plot.py --port /dev/tty... --outfile tests/data/run.csv` to live-plot and save.
- Offline plotting: `python3 tests/scripts/m2_logger_plot.py --pattern my_run*.csv` saves PNGs to `tests/reports/`.
- Python deps: `pandas`, `matplotlib`, and `pyserial` (for live logging).

## Repository Structure
```text
njit-ece-416-project/
├─ firmware/
│  ├─ common/           # Shared comm + setpoint config
│  ├─ control/          # ESP32 control unit (sensors, PID, servos)
│  ├─ ui/               # ESP32 UI unit (buttons, OLED, ESP-NOW)
│  ├─ examples/         # Milestone demos
│  ├─ libraries/        # Local Arduino libs (if any)
│  └─ tools/            # Address/servo calibration sketches
├─ mechanical/
│  ├─ cad/              # 3D models, mounts, enclosure
│  └─ photos/           # Assembly & testbed images
├─ tests/
│  ├─ data/             # CSV logs (temp, flow, servo_us, setpoint)
│  ├─ scripts/          # Analysis helpers (optional)
│  └─ reports/          # Validation results & plots
└─ design/              # Wiring diagrams, block diagram, pin maps, calibration notes
```

## Safety
- Max outlet temp **120 °F** (hard limit in firmware).  
- Link-loss or sensor fault → system defaults to **cold-safe**.  
- Hardware E-stop cuts servo power and forces valve closed. 

## Contributing
- Open an Issue first for features/bugfixes; note required hardware (control/UI boards, sensors, flow).
- Work from a fork or branch off `main`. Touch the relevant area (`firmware/`, `tests/`, `mechanical/`, `design/`) and keep evidence in `tests/data` and `tests/reports`.
- Firmware: use the Arduino libs listed above; only change `firmware/common/config.h` and `firmware/control/config.h` when necessary, and document MAC/address updates.
- Scripts/plots: keep deps to `pandas`, `matplotlib`, `pyserial`; include example commands in your PR.
- Open a PR with a concise summary and evidence paths; call out any safety-impacting changes.
