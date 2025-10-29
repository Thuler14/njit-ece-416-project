// ====================================================
// M2: MAC Address Scan Tool (ESP32)
// Purpose: Print the Station (STA) MAC address for peer setup in ESP-NOW communication
// Board: ESP32 (UI or Control Unit)
// ====================================================

#include <WiFi.h>

uint8_t mac[6];

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  delay(200);

  // Print normal format
  Serial.println();
  Serial.print("STA MAC: ");
  Serial.println(WiFi.macAddress());

  // Print copy-ready format
  WiFi.macAddress(mac);
  Serial.println();
  Serial.print("Copy-paste format: {");
  for (int i = 0; i < 6; i++) {
    Serial.printf("0x%02X", mac[i]);
    if (i < 5) Serial.print(", ");
  }
  Serial.println("}");
}

void loop() {}
