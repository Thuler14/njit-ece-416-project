#include "communication.h"

#include <Arduino.h>
#include <EspNowLink.h>

#include "../common/config.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

// TX sequence tracking
static uint16_t s_seq = 0;          // next sequence to send
static uint16_t s_inFlightSeq = 0;  // 0 = none in flight
static bool s_inFlightUserTx = false;
static unsigned long s_lastHeartbeatMs = 0;
static float s_lastSetpointF = SETPOINT_DEFAULT_F;
static bool s_lastRunFlag = false;

// Current UI→CTRL communication status
static CommStatus s_status{/*lastSeq=*/0,
                           /*txCount=*/0,
                           /*lastOk=*/true,
                           /*pending=*/false,
                           /*outletTempF=*/0.0f,
                           /*outletValid=*/false,
                           /*flowLpm=*/0.0f,
                           /*flowValid=*/false};

static volatile bool s_statusDirty = false;  // status changed since last poll

// Protects s_status, s_statusDirty, s_inFlightSeq, s_inFlightUserTx
static portMUX_TYPE s_statusMux = portMUX_INITIALIZER_UNLOCKED;

static void on_rx(const uint8_t src_mac[6], const uint8_t* data, size_t len, void* ctx) {
  if (len != sizeof(COMM_Payload)) return;  // ignore malformed packets

  COMM_Payload p;
  memcpy(&p, data, sizeof(p));

  if ((p.flags & COMM_FLAG_ACK) && p.seq == s_inFlightSeq) {
    portENTER_CRITICAL(&s_statusMux);
    s_status.lastSeq = p.seq;
    s_status.lastOk = true;
    if (s_inFlightUserTx) s_status.pending = false;
    s_status.txCount++;
    s_status.outletTempF = p.setpointF;
    s_status.outletValid = (p.flags & COMM_FLAG_TEMP_VALID);
    s_status.flowLpm = p.flowLpm;
    s_status.flowValid = (p.flags & COMM_FLAG_FLOW_VALID);
    s_inFlightSeq = 0;
    s_inFlightUserTx = false;
    s_statusDirty = true;
    portEXIT_CRITICAL(&s_statusMux);
  }
}

static void on_tx(const uint8_t dst_mac[6], bool ok, void* ctx) {
  if (!ok && s_inFlightSeq != 0) {
    portENTER_CRITICAL(&s_statusMux);
    s_status.lastSeq = s_inFlightSeq;
    s_status.lastOk = false;
    if (s_inFlightUserTx) s_status.pending = false;
    s_status.txCount++;
    s_inFlightSeq = 0;
    s_inFlightUserTx = false;
    s_statusDirty = true;
    portEXIT_CRITICAL(&s_statusMux);
  }
}

bool commInit() {
  EspNowLinkConfig config{
      .peerMac = COMM_CTRL_MAC,
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

static bool sendCurrent(unsigned long nowMs, bool userTx) {
  // Avoid stomping an in-flight packet (including heartbeats)
  portENTER_CRITICAL(&s_statusMux);
  if (s_inFlightSeq != 0) {
    portEXIT_CRITICAL(&s_statusMux);
    return false;
  }
  portEXIT_CRITICAL(&s_statusMux);

  COMM_Payload p{};
  p.ms = nowMs;
  p.seq = ++s_seq;
  p.setpointF = s_lastSetpointF;
  p.flowLpm = 0.0f;
  p.flags = s_lastRunFlag ? COMM_FLAG_RUN : 0;

  // Mark TX as in-flight
  portENTER_CRITICAL(&s_statusMux);
  s_inFlightSeq = p.seq;
  s_inFlightUserTx = userTx;
  if (userTx) {
    s_status.pending = true;
    s_statusDirty = true;
  }
  portEXIT_CRITICAL(&s_statusMux);

  bool ok = espnow_link_send(&p, sizeof(p)) == ENL_OK;
  s_lastHeartbeatMs = nowMs;

  if (!ok) {
    // Immediate send failure (no ACK expected)
    portENTER_CRITICAL(&s_statusMux);
    s_status.lastSeq = p.seq;
    s_status.lastOk = false;
    if (userTx) s_status.pending = false;
    s_status.txCount++;
    s_inFlightSeq = 0;
    s_inFlightUserTx = false;
    s_statusDirty = true;
    portEXIT_CRITICAL(&s_statusMux);
  }

  return ok;
}

bool commSendSetpoint(float setpointF, bool runFlag) {
  s_lastSetpointF = setpointF;
  s_lastRunFlag = runFlag;
  return sendCurrent(millis(), /*userTx=*/true);
}

void commHeartbeatTick(unsigned long nowMs) {
  // Avoid overlapping with any in-flight packet
  portENTER_CRITICAL(&s_statusMux);
  bool inFlight = (s_inFlightSeq != 0);
  portEXIT_CRITICAL(&s_statusMux);
  if (inFlight) return;
  if ((nowMs - s_lastHeartbeatMs) < UI_HEARTBEAT_MS) return;

  (void) sendCurrent(nowMs, /*userTx=*/false);
}

bool commPollStatus(CommStatus& outStatus) {
  portENTER_CRITICAL(&s_statusMux);
  if (!s_statusDirty) {
    portEXIT_CRITICAL(&s_statusMux);
    return false;
  }
  s_statusDirty = false;
  outStatus = s_status;
  portEXIT_CRITICAL(&s_statusMux);
  return true;
}

void commGetStatus(CommStatus& outStatus) {
  portENTER_CRITICAL(&s_statusMux);
  outStatus = s_status;
  portEXIT_CRITICAL(&s_statusMux);
}
