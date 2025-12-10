#include "display.h"

// Global OLED object (hardware SPI)
U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI oledDisplay(
    U8G2_R0,  // rotation
    /* cs=*/OLED_PIN_CS,
    /* dc=*/OLED_PIN_DC,
    /* reset=*/OLED_PIN_RST);

// Simple circular run/stop icon in the top-right area
static void drawRunIcon(bool runFlag) {
  const uint8_t cx = 116;
  const uint8_t cy = 10;
  const uint8_t r = 8;

  // Circle outline
  oledDisplay.drawCircle(cx, cy, r);

  if (runFlag) {
    // "Play" triangle
    oledDisplay.drawTriangle(
        cx - 3, cy - 5,
        cx - 3, cy + 5,
        cx + 4, cy);
  } else {
    // "Pause" bars
    oledDisplay.drawBox(cx - 3, cy - 4, 2, 9);
    oledDisplay.drawBox(cx + 2, cy - 4, 2, 9);
  }
}

void displayInit() {
  oledDisplay.begin();
  oledDisplay.setContrast(255);
}

void displayDraw(const DisplayState& s) {
  oledDisplay.clearBuffer();

  // ─────────────────────────────
  // Top bar: RUN/STOP label + icon
  // ─────────────────────────────
  oledDisplay.setFont(u8g2_font_6x10_mf);
  oledDisplay.drawStr(0, 10, s.runFlag ? "RUN" : "STOP");
  drawRunIcon(s.runFlag);

  // Mode label for main value
  const bool showingFlow = s.showingFlow;
  const bool showingSetpoint = s.showingSetpoint && !showingFlow;  // flow overlay wins
  oledDisplay.drawStr(0, 24, showingFlow ? "FLOW" : (showingSetpoint ? "SET" : "OUT"));

  // ─────────────────────────────
  // Center: main setpoint (large)
  // ─────────────────────────────
  // Big numeric part
  bool valueValid;
  float valueF;
  const char* unitStr;
  if (showingFlow) {
    valueValid = s.flowValid;
    valueF = s.flowLpm;
    unitStr = "L/m";
  } else {
    valueValid = showingSetpoint || s.outletValid;
    valueF = showingSetpoint ? s.setpointF : s.outletTempF;
    unitStr =
        "\xB0"
        "F";
  }

  oledDisplay.setFont(u8g2_font_logisoso24_tf);  // 24-px tall font
  char tempStr[8];
  if (valueValid) {
    if (showingFlow) {
      snprintf(tempStr, sizeof(tempStr), "%3.2f", valueF);
    } else {
      snprintf(tempStr, sizeof(tempStr), "%3.1f", valueF);
    }
  } else {
    snprintf(tempStr, sizeof(tempStr), "---");
  }

  uint16_t tempW = oledDisplay.getStrWidth(tempStr);
  oledDisplay.setFont(u8g2_font_7x13B_mf);
  uint16_t unitW = oledDisplay.getStrWidth(unitStr);
  oledDisplay.setFont(u8g2_font_logisoso24_tf);

  uint8_t tempX = (128 - (tempW + 2 + unitW)) / 2;
  uint8_t tempY = 44;  // baseline

  oledDisplay.drawStr(tempX, tempY, tempStr);

  // Degree + unit, slightly smaller font
  oledDisplay.setFont(u8g2_font_7x13B_mf);
  oledDisplay.drawStr(tempX + tempW + 2, tempY, unitStr);

  // ─────────────────────────────
  // Bottom bar: step size + TX status
  // ─────────────────────────────
  oledDisplay.setFont(u8g2_font_5x7_mf);
  const uint8_t y_base = 63;

  // Step size (left)
  char stepStr[16];
  snprintf(stepStr, sizeof(stepStr), "\xB1%0.1f", s.stepF);
  oledDisplay.drawStr(0, y_base, stepStr);

  // TX status (right)
  char tiny[32];
  snprintf(tiny, sizeof(tiny), "TX:%lu %s",
           (unsigned long) s.txDoneCount,
           s.lastResultOk ? "OK" : "FAIL");
  uint16_t w = oledDisplay.getStrWidth(tiny);
  oledDisplay.drawStr(126 - w, y_base, tiny);

  // Pending marker (small indicator above step)
  if (s.pending) {
    oledDisplay.drawStr(0, y_base - 9, "PEND");
  }

  oledDisplay.sendBuffer();
}
