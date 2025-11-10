/*
 * ================================================================
 *  Module: EspNowLink
 *  Purpose: Thin wrapper around ESP-NOW for point-to-point link.
 *           Handles WiFi STA setup, channel locking, peer config,
 *           optional encryption, and user callbacks.
 *
 *  Dependencies:
 *    - esp_err.h   (ESP-IDF error codes)
 *    - stddef.h    (size_t)
 *    - stdint.h    (fixed-width integer types)
 *    - esp_now.h   (used in EspNowLink.cpp implementation)
 *
 *  Interface:
 *    enum EspNowLinkErr
 *    struct EspNowLinkConfig
 *    EspNowLinkErr espnow_link_begin(const EspNowLinkConfig& config);
 *    EspNowLinkErr espnow_link_send(const void* data, size_t len);
 *    const uint8_t* espnow_link_peer_mac();
 *    uint8_t        espnow_link_channel();
 * ================================================================
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

enum EspNowLinkErr {
  ENL_OK = 0,
  ENL_BAD_ARGS,
  ENL_WIFI_CHAN_FAIL,
  ENL_INIT_FAIL,
  ENL_KEY_NULL,
  ENL_PMK_FAIL,
  ENL_PEER_FAIL,
  ENL_SEND_FAIL,
};

typedef void (*EspNowRxHandler)(const uint8_t mac[6],
                                const uint8_t* data,
                                size_t len,
                                void* ctx);

typedef void (*EspNowTxHandler)(const uint8_t mac[6],
                                bool ok,
                                void* ctx);

struct EspNowLinkConfig {
  const uint8_t* peerMac;     // must point to 6 bytes
  uint8_t channel;            // 1–13
  bool useEncryption;         // true = use PMK/LMK
  const uint8_t* pmk;         // 16-byte PMK or nullptr if !useEncryption
  const uint8_t* lmk;         // 16-byte LMK or nullptr if !useEncryption
  EspNowRxHandler rxHandler;  // may be nullptr
  EspNowTxHandler txHandler;  // may be nullptr
  void* ctx;                  // user context
};

// Initialize ESP-NOW link with given configuration
EspNowLinkErr espnow_link_begin(const EspNowLinkConfig& config);

// Send a payload to the configured peer
EspNowLinkErr espnow_link_send(const void* data, size_t len);

// Return configured peer MAC address
const uint8_t* espnow_link_peer_mac();

// Return configured WiFi channel
uint8_t espnow_link_channel();
