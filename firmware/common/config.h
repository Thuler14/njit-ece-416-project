#pragma once
#include <stdint.h>

// ====================================================
// ESP-NOW Communication
// ====================================================

// --- Configuration ---
constexpr uint8_t COMM_CHANNEL = 6;          // Wi-Fi channel (1–13)
constexpr bool COMM_USE_ENCRYPTION = false;  // Enable encryption (true/false)

// --- MAC Addresses ---
// Scanned using: firmware/tools/m2_mac_scan/m2_mac_scan.ino
static const uint8_t COMM_CTRL_MAC[6]{0x3C, 0x8A, 0x1F, 0x80, 0xA9, 0xD4};  // Control Unit
static const uint8_t COMM_UI_MAC[6]{0x8C, 0x4F, 0x00, 0x35, 0x9B, 0xF4};    // UI Unit

// --- Encryption Keys ---
static const uint8_t COMM_PMK[16]{'s', 'h', 'o', 'w', 'e', 'r', 'c', 't', 'r', 'l', '_', 'p', 'm', 'k', '1', '6'};  // Primary Master Key
static const uint8_t COMM_LMK[16]{'s', 't', 'a', 't', 'i', 'c', '_', 'l', 'm', 'k', '_', 'u', 'i', 'c', 't', 'r'};  // Local Master Key

// --- Protocol version ---
constexpr uint8_t COMM_PROTOCOL_VERSION = 1;

// --- Flag bit masks ---
constexpr uint8_t COMM_FLAG_ACK = 1 << 0;  // Acknowledgment bit
constexpr uint8_t COMM_FLAG_RUN = 1 << 1;  // Run/Stop bit
constexpr uint8_t COMM_FLAG_ERR = 1 << 2;  // Error indication bit
// bits 3–7 reserved

// --- Payload Structure ---
typedef struct __attribute__((packed)) {
  uint32_t ms;      // timestamp (ms)
  uint16_t seq;     // sequence number
  float setpointF;  // temperature setpoint
  uint8_t flags;    // status bits (COMM_FLAG_*)
} COMM_Payload;
