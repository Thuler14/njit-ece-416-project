/*
 * ================================================================
 *  Module: communication
 *  Purpose: Provides UI-side communication functions for the shower
 *           controller. Handles transmission of setpoint and run-state
 *           commands to the Control Unit via ESP-NOW.
 *
 *  Communication:
 *    - Sends COMM_Payload packets to Control Unit
 *    - Tracks sequence number, transmission count, and result status
 *    - Optional encryption (PMK/LMK handled in EspNowLink)
 *
 *  Dependencies:
 *    - <stdint.h>  (basic integer types)
 *    - EspNowLink  (transport layer)
 *    - config.h    (COMM_* constants and MAC addresses)
 *
 *  Interface:
 *    bool commInit();
 *    bool commSendSetpoint(float setpointF, bool runFlag);
 *    bool commPollStatus(CommStatus& outStatus);
 *    void commGetStatus(CommStatus& outStatus);
 *
 *  Data Structures:
 *    struct CommStatus {
 *      uint16_t lastSeq;   // last transmitted sequence number
 *      uint32_t txCount;   // total successful or failed transmissions
 *      bool     lastOk;    // true=ACK received, false=TX failed
 *      bool     pending;   // true=awaiting ACK for user TX (setpoint/run)
 *      float    outletTempF;  // latest outlet temperature mirrored from Control
 *      bool     outletValid;  // true if outletTempF is valid
 *    };
 * ================================================================
 */

#pragma once

#include <stdint.h>

// Tracks UI→Control communication state
struct CommStatus {
  uint16_t lastSeq;
  uint32_t txCount;
  bool lastOk;
  bool pending;
  float outletTempF;
  bool outletValid;
};

// Initialize communication layer (ESP-NOW transport setup)
bool commInit();

// Send current setpoint and run-state to Control Unit
bool commSendSetpoint(float setpointF, bool runFlag);

// Heartbeat/service function to be called from loop() with millis()
// Keeps link alive while runFlag is true using the last sent state
void commHeartbeatTick(unsigned long nowMs);

// Check if communication status has changed since last poll
bool commPollStatus(CommStatus& outStatus);

// Retrieve latest communication status snapshot
void commGetStatus(CommStatus& outStatus);
