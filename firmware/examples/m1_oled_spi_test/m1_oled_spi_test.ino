// ====================================================
// M1: OLED Display SPI Test (SSD1309 + OLED_DISPLAY)
// Purpose: Verify wiring and SPI communication by displaying a border and "SPI OK"
// Board: ESP32 (UI Module)
// Bus: VSPI (4-wire SPI + CS)
// ====================================================

#include <U8g2lib.h>

#define OLED_PIN_CS 5
#define OLED_PIN_DC 22
#define OLED_PIN_RST 4

U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI oledDisplay(
    U8G2_R0,  // rotation
    /* cs=*/OLED_PIN_CS,
    /* dc=*/OLED_PIN_DC,
    /* reset=*/OLED_PIN_RST);

void setup() {
  Serial.begin(115200);
  delay(200);
  oledDisplay.begin();
  Serial.println("SSD1309 (SPI) init OK");

  oledDisplay.clearBuffer();
  oledDisplay.drawFrame(0, 0, 126, 64);  // full visible border for 126x64 panel
  oledDisplay.setFont(u8g2_font_6x10_tf);
  oledDisplay.drawStr(14, 28, "OLED SPI Test");
  oledDisplay.drawStr(14, 44, "Status: SPI OK");
  oledDisplay.sendBuffer();
}

void loop() {}
