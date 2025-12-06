#include "communication.h"

#include <Arduino.h>
#include <EspNowLink.h>

#include "../common/config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

// RX sequence tracking
static CommCommand s_lastCmd{};  // last command sent
static bool s_newCmd = false;    // true = new command available
static unsigned long s_lastRxMs = 0;  // millis() when last valid packet arrived

// Protects s_lastCmd and s_newCmd
static portMUX_TYPE s_cmdMux = portMUX_INITIALIZER_UNLOCKED;

static void on_rx(const uint8_t src_mac[6], const uint8_t* data, size_t len, void* ctx) {
  COMM_Payload ack{};
  ack.ms = millis();

  CommCommand cmd{};
  cmd.lastOk = false;

  if (len == sizeof(COMM_Payload)) {
    s_lastRxMs = millis();
    // Valid packet: extract data and prepare ACK
    COMM_Payload p;
    memcpy(&p, data, sizeof(p));

    cmd.setpointF = p.setpointF;
    cmd.runFlag = (p.flags & COMM_FLAG_RUN);
    cmd.lastSeq = p.seq;
    cmd.lastOk = true;

    ack.seq = p.seq;
    ack.flags = COMM_FLAG_ACK;
  } else {
    // Malformed packet: mark error and prepare ERR response
    cmd.lastOk = false;
    cmd.lastSeq = 0;

    ack.seq = 0;
    ack.flags = COMM_FLAG_ERR;
  }

  // Store latest command atomically
  portENTER_CRITICAL(&s_cmdMux);
  s_lastCmd = cmd;
  s_newCmd = true;
  portEXIT_CRITICAL(&s_cmdMux);

  // Send ACK/ERR back to UI (ignore send error here)
  (void) espnow_link_send(&ack, sizeof(ack));
}

static void on_tx(const uint8_t dst_mac[6], bool ok, void* ctx) {
  // TX callback (not used by Control Unit)
}

bool commInit() {
  EspNowLinkConfig config{
      .peerMac = COMM_UI_MAC,
      .channel = COMM_CHANNEL,
      .useEncryption = COMM_USE_ENCRYPTION,
      .pmk = COMM_USE_ENCRYPTION ? COMM_PMK : nullptr,
      .lmk = COMM_USE_ENCRYPTION ? COMM_LMK : nullptr,
      .rxHandler = on_rx,
      .txHandler = on_tx,
      .ctx = nullptr,
  };

  return espnow_link_begin(config) == ENL_OK;
}

bool commPollCommand(CommCommand& outCmd) {
  portENTER_CRITICAL(&s_cmdMux);
  if (!s_newCmd) {
    portEXIT_CRITICAL(&s_cmdMux);
    return false;
  }
  s_newCmd = false;
  outCmd = s_lastCmd;
  portEXIT_CRITICAL(&s_cmdMux);
  return true;
}

unsigned long commLastRxMs() {
  return s_lastRxMs;
}

void commMarkLinkLost() {
  s_lastRxMs = 0;
}
