#pragma once

// ====================================================
// Temperature Sensors (DS18B20)
// Scanned: firmware/tools/m1_temp_scan/m1_temp_scan.ino
// ====================================================

// OneWire bus pin
#define PIN_TEMP_ONEWIRE 4  // DS18B20 data line (GPIO 4)

// Sensor ROM addresses (8 bytes each)
#define ADDR_T_HOT {0x28, 0x8D, 0xAC, 0x12, 0x00, 0x00, 0x00, 0x93}
#define ADDR_T_COLD {0x28, 0x2C, 0x4F, 0x12, 0x00, 0x00, 0x00, 0x6C}
#define ADDR_T_OUT {0x28, 0x59, 0x4D, 0x12, 0x00, 0x00, 0x00, 0x9C}
