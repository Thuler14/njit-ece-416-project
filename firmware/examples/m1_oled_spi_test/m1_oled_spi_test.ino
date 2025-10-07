// ====================================================
// M1: OLED Display SPI Test (SSD1309 + U8g2)
// Purpose: Verify wiring and SPI communication by displaying a border and "SPI OK"
// Board: ESP32 (UI Module)
// Bus: VSPI (4-wire SPI + CS)
// ====================================================

#include "../../ui/config.h"  // Centralized configuration

void setup() {
  Serial.begin(115200);
  delay(200);
  u8g2.begin();
  Serial.println("SSD1309 (SPI) init OK");

  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 126, 64);  // full visible border for 126x64 panel
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(14, 28, "OLED SPI Test");
  u8g2.drawStr(14, 44, "Status: SPI OK");
  u8g2.sendBuffer();
}

void loop() {}
