# design
System design artifacts: block/wiring diagrams, pin maps, and configuration notes for the ESP32 control + UI units.

## Contents
- `wiring/` — wiring diagrams and pin tables for sensors, servos, flow sensor, E-stop, OLED, and buttons.
- `config/` — design-time notes for ESP-NOW, PID constants, setpoint presets, and component specifics (OLED, temp sensors, flow sensor, buttons, servos).

## Conventions
- Keep editable sources (Draw.io/Fritzing/etc.) next to exported PNG/PDF.
- Note any calibration values or pin swaps that need to be mirrored in `firmware/common/config.h` and `firmware/control/config.h`.
