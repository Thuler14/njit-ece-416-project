#pragma once
#include <stdint.h>

// ====================================================
// ESP-NOW Communication
// ====================================================

// --- Configuration ---
#define COMM_CHANNEL 6         // Wi-Fi channel (1–13)
#define COMM_USE_ENCRYPTION 0  // 1 = ON, 0 = OFF

// --- MAC Addresses ---
// Scanned using: firmware/tools/m2_mac_scan/m2_mac_scan.ino
static const uint8_t COMM_CTRL_MAC[6]{0x3C, 0x8A, 0x1F, 0x80, 0xA9, 0xD4};  // Control Unit
static const uint8_t COMM_UI_MAC[6]{0x8C, 0x4F, 0x00, 0x35, 0x9B, 0xF4};    // UI Unit

// --- Encryption Keys ---
static const uint8_t COMM_PMK[16]{'s', 'h', 'o', 'w', 'e', 'r', 'c', 't', 'r', 'l', '_', 'p', 'm', 'k', '1', '6'};  // Primary Master Key
static const uint8_t COMM_LMK[16]{'s', 't', 'a', 't', 'i', 'c', '_', 'l', 'm', 'k', '_', 'u', 'i', 'c', 't', 'r'};  // Local Master Key
