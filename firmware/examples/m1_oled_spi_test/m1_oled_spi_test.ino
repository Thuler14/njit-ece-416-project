// ====================================================
// M1: OLED Display SPI Test (SSD1309 + U8g2)
// Purpose: Verify wiring and SPI communication by displaying a border and "SPI OK"
// Board: ESP32 (UI Module)
// Bus: VSPI (4-wire SPI + CS)
// ====================================================

#include <U8g2lib.h>

#include "../../ui/config.h"  // Centralized constants

U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI u8g2(
    U8G2_R0,  // rotation
    /* cs=*/PIN_OLED_CS,
    /* dc=*/PIN_OLED_DC,
    /* reset=*/PIN_OLED_RST);

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
