// ====================================================
// M1: OLED "UI Ready" Screen (SSD1309 + U8g2)
// Purpose: Render "UI READY" with a thermometer icon using full-buffer SPI (no flicker)
// Board: ESP32 (UI Module)
// Bus: VSPI (4-wire SPI + CS)
// ====================================================

#include "../../ui/config.h"  // Centralized configuration

// Simple thermometer icon (vector, no bitmap needed)
static void drawThermometer(int x, int y) {
  // Bulb
  u8g2.drawCircle(x + 8, y + 40, 8);  // outer bulb
  u8g2.drawDisc(x + 8, y + 40, 6);    // fill
  // Stem outline
  u8g2.drawFrame(x + 6, y + 8, 4, 28);  // stem tube
  // Neck connecting stem to bulb
  u8g2.drawBox(x + 6, y + 32, 4, 6);
  // Mercury level (adjust height to taste)
  u8g2.drawBox(x + 7, y + 24, 2, 14);
  // Tick marks
  for (int i = 0; i <= 5; ++i) {
    int yy = y + 8 + i * 5;
    u8g2.drawHLine(x + 11, yy, 6);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  u8g2.begin();
  Serial.println("SSD1309 (SPI) init OK");

  // Draw once (FULL buffer prevents flicker)
  u8g2.clearBuffer();

  // Safe border
  u8g2.drawFrame(0, 0, 126, 64);

  // Icon on the left
  drawThermometer(8, 6);

  // Text centered horizontally
  const char* msg = "UI Ready";
  u8g2.setFont(u8g2_font_helvB12_tf);
  uint16_t w = u8g2.getStrWidth(msg);
  uint16_t x = (126 - w) / 2;
  u8g2.drawStr(x, 26, msg);

  // Small subtitle
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(x, 44, "text + graphics");

  u8g2.sendBuffer();  // push to display
}

void loop() {
}
