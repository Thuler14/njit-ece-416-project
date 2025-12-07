# Emergency Stop Button - Wiring

Single momentary push button used to cut servo drive in firmware (active-low with internal pull-up).

| Signal | ESP32 Pin | Notes |
|:--|:--:|:--|
| E-STOP | GPIO 33 | Input with `INPUT_PULLUP`; momentary push to GND triggers E-stop |

Recommendation: wire the button to short the pin to GND when pressed. Keep leads short and routed away from servo power to reduce noise pickup.
