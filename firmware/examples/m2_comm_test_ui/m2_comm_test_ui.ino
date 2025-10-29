// ====================================================
// M2: ESP-NOW Communication Test — UI
// Purpose: Send packets (setpoint/flags) to Control and verify ACK over a single-peer ESP-NOW link
// Board: ESP32 (UI Module), STA mode only
// Channel: 6  |  Encryption: Optional (set COMM_USE_ENCRYPTION = 1)
// Peer MAC: COMM_CTRL_MAC
// ====================================================

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>

#include "../../config/config.h"  // Centralized configuration

typedef struct __attribute__((packed)) {
  uint32_t ms;
  uint16_t seq;
  float setpointF;
  uint8_t flags;  // bit0 = ACK
} Payload;

static uint16_t seq = 0;
unsigned long tLast = 0;

String macStr(const uint8_t* m) {
  char b[18];
  snprintf(b, sizeof(b), "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
  return String(b);
}

void onSend(const wifi_tx_info_t* info, esp_now_send_status_t status) {
  const uint8_t* mac = info ? info->des_addr : nullptr;
  char m[18];
  if (mac) snprintf(m, sizeof(m), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("UI SEND -> %s status=%s\n",
                mac ? m : "(null)",
                status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void onRecv(const esp_now_recv_info_t* info, const uint8_t* data, int data_len) {
  const uint8_t* mac = info ? info->src_addr : nullptr;  // sender MAC
  if (!mac) return;                                      // safety

  // accept only Control
  if (memcmp(mac, COMM_CTRL_MAC, 6) != 0) {
    char m[18];
    snprintf(m, sizeof(m), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.printf("IGNORED from %s\n", m);
    return;
  }

  if (data_len != sizeof(Payload)) {  // size check
    Serial.printf("IGNORED bad len=%d\n", data_len);
    return;
  }

  Payload rx;
  memcpy(&rx, data, sizeof(rx));
  char m[18];
  snprintf(m, sizeof(m), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("UI RECV <- %s seq=%u ack=%u setpoint=%.2f ms=%lu\n",
                m, rx.seq, (rx.flags & 1), rx.setpointF, (unsigned long) rx.ms);
}

bool addPeer(const uint8_t* peer) {
  esp_now_peer_info_t p = {};
  memcpy(p.peer_addr, peer, 6);
  p.channel = COMM_CHANNEL;  // fixed channel
  p.ifidx = WIFI_IF_STA;
  p.encrypt = COMM_USE_ENCRYPTION;
  if (p.encrypt) memcpy(p.lmk, COMM_LMK, 16);
  return esp_now_add_peer(&p) == ESP_OK;
}

void lockChannel(uint8_t ch) {
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);  // set channel before ESP-NOW
  esp_wifi_set_promiscuous(false);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  // STA mode only
  delay(50);
  lockChannel(COMM_CHANNEL);

  Serial.println("\n=== UI ESPNOW ===");
  Serial.printf("Local: %s\n", WiFi.macAddress().c_str());
  Serial.printf("Peer : %s\n", macStr(COMM_CTRL_MAC).c_str());
  Serial.printf("Chan : %d  Enc:%s\n", COMM_CHANNEL, COMM_USE_ENCRYPTION ? "ON" : "OFF");

  if (esp_now_init() != ESP_OK) {
    Serial.println("esp_now_init FAILED");
    while (1) delay(1000);
  }
  if (COMM_USE_ENCRYPTION) esp_now_set_pmk(COMM_PMK);
  if (!addPeer(COMM_CTRL_MAC)) {
    Serial.println("add_peer FAILED");
    while (1) delay(1000);
  }

  esp_now_register_send_cb(onSend);
  esp_now_register_recv_cb(onRecv);

  Serial.println("UI READY");
}

void loop() {
  unsigned long now = millis();
  if (now - tLast >= 1000) {  // 1 Hz heartbeat
    tLast = now;
    Payload tx;
    tx.ms = now;
    tx.seq = ++seq;
    tx.setpointF = 102.0f;  // placeholder value
    tx.flags = 0;

    esp_err_t e = esp_now_send(COMM_CTRL_MAC, (uint8_t*) &tx, sizeof(tx));
    Serial.printf("UI TX -> %s seq=%u err=%d\n", macStr(COMM_CTRL_MAC).c_str(), tx.seq, (int) e);
  }
}
