#pragma once

#include <U8g2lib.h>

// ====================================================
// OLED Display (SSD1309 SPI Blue)
// ====================================================

#define PIN_OLED_CS 5   // Chip Select (CS)
#define PIN_OLED_DC 22  // Data/Command (DC)
#define PIN_OLED_RST 4  // Reset (RST)

// ====================================================
// U8g2 Display Object
// ====================================================
U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI u8g2(
    U8G2_R0,  // rotation
    /* cs=*/PIN_OLED_CS,
    /* dc=*/PIN_OLED_DC,
    /* reset=*/PIN_OLED_RST);
