// ====================================================
// M1: Temperature Address Scan Tool (DS18B20)
// Purpose: Detect and print ROM addresses in copy-paste format for config.h
// Board: ESP32 (Control Unit)
// Bus: GPIO 4 with 4.7kΩ pull-up to 3V3
// ====================================================

#include <OneWire.h>

#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);

void printAddress(uint8_t* addr) {
  // Hex string
  Serial.print("0x");
  for (uint8_t i = 0; i < 8; i++) {
    if (addr[i] < 16) Serial.print("0");
    Serial.print(addr[i], HEX);
  }
  Serial.print("  -> {");
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print("0x");
    if (addr[i] < 16) Serial.print("0");
    Serial.print(addr[i], HEX);
    if (i < 7) Serial.print(",");
  }
  Serial.println("}");
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[DS18B20 Address Scanner]");
}

void loop() {
  uint8_t addr[8];
  oneWire.reset_search();

  Serial.println("Scanning...");
  while (oneWire.search(addr)) {
    if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC error, skipping");
      continue;
    }
    if (addr[0] != 0x28) {
      Serial.println("Not a DS18B20, skipping");
      continue;
    }
    printAddress(addr);
  }
  Serial.println("Done.\n");

  delay(5000);  // rescan every 5 s
}
