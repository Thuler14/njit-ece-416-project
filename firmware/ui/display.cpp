#include "display.h"

// Global OLED object (hardware SPI)
U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI oledDisplay(
    U8G2_R0,  // rotation
    /* cs=*/OLED_PIN_CS,
    /* dc=*/OLED_PIN_DC,
    /* reset=*/OLED_PIN_RST);

void displayInit() {
  oledDisplay.begin();
  oledDisplay.setContrast(255);
}

void displayDraw(const DisplayState& s) {
  oledDisplay.clearBuffer();

  // Setpoint
  oledDisplay.setFont(u8g2_font_7x13B_mf);
  char line1[32];
  snprintf(line1, sizeof(line1),
           "Setpoint: %5.1f\xB0"
           "F",
           s.setpointF);
  oledDisplay.drawStr(0, 18, line1);

  // Run
  oledDisplay.setFont(u8g2_font_7x13B_mf);
  oledDisplay.drawStr(0, 40, "Run:");
  oledDisplay.drawStr(50, 40, s.runFlag ? "ON" : "OFF");

  // TX status (bottom right)
  oledDisplay.setFont(u8g2_font_5x7_mf);
  char tiny[32];
  snprintf(tiny, sizeof(tiny), "TX:%lu %s",
           (unsigned long) s.txDoneCount,
           s.lastResultOk ? "OK" : "FAIL");
  uint16_t w = oledDisplay.getStrWidth(tiny);
  uint16_t y_base = 64 - 1;
  oledDisplay.drawStr(126 - w - 1, y_base, tiny);

  // Pending marker (bottom left)
  if (s.pending) oledDisplay.drawStr(0, y_base, "*");

  oledDisplay.sendBuffer();
}
