# firmware/control
ESP32 firmware for the control unit: temperature sensors, PI loop, dual servos, flow sensing, and safety interlocks.

## Setup
- Open `control.ino` in Arduino IDE (ESP32 board support installed).
- Libraries: **OneWire**, **DallasTemperature**, **ESP32Servo**.
- Configure hardware constants in `config.h`:
  - Set DS18B20 ROM addresses (scan with `firmware/tools/m1_temp_scan/`).
  - Servo min/max µs from `firmware/tools/m1_servo_calibration/`.
- Match ESP-NOW MAC/channel/encryption with `firmware/common/config.h`.

## Operation
- Receives setpoint + run/stop from the UI unit via ESP-NOW.
- Polls hot/cold/outlet DS18B20s at 10 Hz with plausibility + rapid-change checks.
- Drives two MG996R servos to mix hot/cold; monitors flow (YF-S201) and E-stop.
- CSV logging (10 Hz) is enabled when `PID_LOG_CSV` is true; capture via USB serial to `tests/data/`. Header: `ms,setF,T_out_raw,T_out_filt,ratio,u,Kp,Ki,flow_lpm,link_ok`.
