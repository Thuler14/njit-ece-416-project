# firmware/ui
ESP32 firmware for the user interface unit: buttons, OLED, presets, and ESP-NOW link to the control unit.

## Setup
- Open `ui.ino` in Arduino IDE (ESP32 board support installed).
- Library: **U8g2** for the SSD1309 OLED.
- Ensure `firmware/common/config.h` MAC/channel/encryption match the control unit.
- Button/OLED pinout lives in `config.h` (defaults: ▲▼● A/B on GPIO 25/26/27/14/13).

## Operation
- Sends setpoint + run/stop to the control unit; sends heartbeat every second.
- UI shortcuts: ▲/▼ adjust setpoint, presets A/B defined in `firmware/common/config.h`.
- Screen shows outlet temp, link status, and flow (when provided by the control unit).
- Control link is over ESP-NOW; update preset values or default setpoint in `firmware/common/config.h`.
