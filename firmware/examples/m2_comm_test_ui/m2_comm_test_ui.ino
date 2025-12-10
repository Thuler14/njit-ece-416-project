// ====================================================
// M2: ESP-NOW Communication Test — UI
// Purpose: Send packets (setpoint/flags) to Control and verify ACK over a single-peer ESP-NOW link
// Board: ESP32 (UI Module), STA mode only
// Channel: 6  |  Encryption: Optional (set COMM_USE_ENCRYPTION = true)
// Peer MAC: COMM_CTRL_MAC
// ====================================================

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>

#define COMM_CHANNEL 6
#define COMM_USE_ENCRYPTION 0

const uint8_t COMM_CTRL_MAC[6]{0x3C, 0x8A, 0x1F, 0x80, 0xA9, 0xD4};
const uint8_t COMM_PMK[16]{'s', 'h', 'o', 'w', 'e', 'r', 'c', 't', 'r', 'l', '_', 'p', 'm', 'k', '1', '6'};
const uint8_t COMM_LMK[16]{'s', 't', 'a', 't', 'i', 'c', '_', 'l', 'm', 'k', '_', 'u', 'i', 'c', 't', 'r'};

#define COMM_FLAG_ACK (1 << 0)
#define COMM_FLAG_TEMP_VALID (1 << 3)
#define COMM_FLAG_FLOW_VALID (1 << 4)

typedef struct __attribute__((packed)) {
  uint32_t ms;
  uint16_t seq;
  float setpointF;
  float flowLpm;
  uint8_t flags;  // bit0 = ACK
} COMM_Payload;

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

  if (!mac) return;  // safety

  // accept only Control
  if (memcmp(mac, COMM_CTRL_MAC, 6) != 0) {
    char m[18];
    snprintf(m, sizeof(m), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.printf("IGNORED from %s\n", m);
    return;
  }

  if (data_len != sizeof(COMM_Payload)) {  // size check
    Serial.printf("IGNORED bad len=%d\n", data_len);
    return;
  }

  COMM_Payload rx;
  char m[18];

  memcpy(&rx, data, sizeof(rx));
  snprintf(m, sizeof(m), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Serial.printf("UI RECV <- %s seq=%u ack=%u outlet=%.2f flow=%.2f ms=%lu flags=0x%02X\n",
                m, rx.seq, (rx.flags & COMM_FLAG_ACK), rx.setpointF, rx.flowLpm, (unsigned long) rx.ms, rx.flags);
}

bool addPeer(const uint8_t* peer) {
  esp_now_peer_info_t p = {};

  memcpy(p.peer_addr, peer, 6);

  p.channel = COMM_CHANNEL;  // fixed channel
  p.ifidx = WIFI_IF_STA;
  p.encrypt = COMM_USE_ENCRYPTION ? 1 : 0;

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

    COMM_Payload tx;
    tx.ms = now;
    tx.seq = ++seq;
    tx.setpointF = 102.0f;  // placeholder value
    tx.flowLpm = 0.0f;
    tx.flags = 0;

    esp_err_t e = esp_now_send(COMM_CTRL_MAC, (uint8_t*) &tx, sizeof(tx));
    Serial.printf("UI TX -> %s seq=%u err=%d\n", macStr(COMM_CTRL_MAC).c_str(), tx.seq, (int) e);
  }
}
