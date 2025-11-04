// ====================================================
// M1: OLED "UI Ready" Screen (SSD1309 + U8g2)
// Purpose: Render "UI READY" with a thermometer icon using full-buffer SPI (no flicker)
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

// Simple thermometer icon (vector, no bitmap needed)
static void drawThermometer(int x, int y) {
  // Bulb
  oledDisplay.drawCircle(x + 8, y + 40, 8);  // outer bulb
  oledDisplay.drawDisc(x + 8, y + 40, 6);    // fill
  // Stem outline
  oledDisplay.drawFrame(x + 6, y + 8, 4, 28);  // stem tube
  // Neck connecting stem to bulb
  oledDisplay.drawBox(x + 6, y + 32, 4, 6);
  // Mercury level (adjust height to taste)
  oledDisplay.drawBox(x + 7, y + 24, 2, 14);
  // Tick marks
  for (int i = 0; i <= 5; ++i) {
    int yy = y + 8 + i * 5;
    oledDisplay.drawHLine(x + 11, yy, 6);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  oledDisplay.begin();
  Serial.println("SSD1309 (SPI) init OK");

  // Draw once (FULL buffer prevents flicker)
  oledDisplay.clearBuffer();

  // Safe border
  oledDisplay.drawFrame(0, 0, 126, 64);

  // Icon on the left
  drawThermometer(8, 6);

  // Text centered horizontally
  const char* msg = "UI Ready";
  oledDisplay.setFont(u8g2_font_helvB12_tf);
  uint16_t w = oledDisplay.getStrWidth(msg);
  uint16_t x = (126 - w) / 2;
  oledDisplay.drawStr(x, 26, msg);

  // Small subtitle
  oledDisplay.setFont(u8g2_font_6x10_tf);
  oledDisplay.drawStr(x, 44, "text + graphics");

  oledDisplay.sendBuffer();  // push to display
}

void loop() {}
