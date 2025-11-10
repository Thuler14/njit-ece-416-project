#include "EspNowLink.h"

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <string.h>

static EspNowLinkConfig s_config{};
static bool s_started = false;

// Add or update the single configured peer
static EspNowLinkErr add_or_update_peer_() {
  esp_now_peer_info_t p = {};
  memcpy(p.peer_addr, s_config.peerMac, 6);
  p.channel = s_config.channel;
  p.ifidx = WIFI_IF_STA;
  if (s_config.useEncryption) {
    p.encrypt = 1;
    memcpy(p.lmk, s_config.lmk, 16);
  } else
    p.encrypt = 0;

  esp_err_t err;
  if (esp_now_is_peer_exist(p.peer_addr)) {
    err = esp_now_mod_peer(&p);
    return (err == ESP_OK) ? ENL_OK : ENL_PEER_FAIL;
  }

  err = esp_now_add_peer(&p);
  if (err == ESP_OK || err == ESP_ERR_ESPNOW_EXIST) return ENL_OK;
  return ENL_PEER_FAIL;
}

// Bridge ESP-NOW RX callback to user handler
static void onRecv_cb(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  if (!s_config.rxHandler || !info || !data || len <= 0) return;
  uint8_t mac[6];
  memcpy(mac, info->src_addr, 6);
  s_config.rxHandler(mac, data, (size_t) len, s_config.ctx);
}

// Bridge ESP-NOW TX callback to user handler
static void onSend_cb(const esp_now_send_info_t* info, esp_now_send_status_t status) {
  if (!s_config.txHandler || !info) return;
  uint8_t mac[6];
  memcpy(mac, info->src_addr, 6);
  s_config.txHandler(mac, status == ESP_NOW_SEND_SUCCESS, s_config.ctx);
}

// Lock WiFi to a fixed primary channel
static bool lock_channel_(uint8_t ch) {
  if (esp_wifi_set_promiscuous(true) != ESP_OK) return false;
  if (esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    esp_wifi_set_promiscuous(false);
    return false;
  }
  return esp_wifi_set_promiscuous(false) == ESP_OK;
}

// Initialize ESP-NOW, retry once if internal error
static bool init_espnow_() {
  esp_err_t err = esp_now_init();
  if (err == ESP_ERR_ESPNOW_INTERNAL) {
    esp_now_deinit();
    err = esp_now_init();
  }
  return err == ESP_OK;
}

static void deinit_partial_() { esp_now_deinit(); }

EspNowLinkErr espnow_link_begin(const EspNowLinkConfig& config) {
  if (!config.peerMac || config.channel < 1 || config.channel > 13) return ENL_BAD_ARGS;
  if (s_started) return ENL_OK;

  s_config = config;

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  delay(20);

  if (!lock_channel_(s_config.channel)) return ENL_WIFI_CHAN_FAIL;
  if (!init_espnow_()) return ENL_INIT_FAIL;

  if (config.useEncryption) {
    if (!config.pmk || !config.lmk) {
      deinit_partial_();
      return ENL_KEY_NULL;
    }

    if (esp_now_set_pmk(config.pmk) != ESP_OK) {
      deinit_partial_();
      return ENL_PMK_FAIL;
    }
  }

  EspNowLinkErr res = add_or_update_peer_();
  if (res != ENL_OK) {
    deinit_partial_();
    return res;
  }

  esp_now_register_recv_cb(onRecv_cb);
  esp_now_register_send_cb(onSend_cb);
  s_started = true;
  return ENL_OK;
}

EspNowLinkErr espnow_link_send(const void* data, size_t len) {
  if (!data || !len) return ENL_BAD_ARGS;
  esp_err_t err = esp_now_send(s_config.peerMac, (const uint8_t*) data, len);
  return (err == ESP_OK) ? ENL_OK : ENL_SEND_FAIL;
}

const uint8_t* espnow_link_peer_mac() { return s_config.peerMac; }
uint8_t espnow_link_channel() { return s_config.channel; }
