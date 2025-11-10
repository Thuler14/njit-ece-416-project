/*
 * ================================================================
 *  Module: communication
 *  Purpose: Provides Control Unit communication functions for the
 *           shower controller. Handles reception of setpoint and
 *           run-state commands from the UI Unit via ESP-NOW.
 *
 *  Communication:
 *    - Receives COMM_Payload packets from UI Unit
 *    - Validates payload length and updates last received command
 *    - Sends ACK or ERR response back to UI
 *
 *  Dependencies:
 *    - <stdint.h>   (basic integer types)
 *    - EspNowLink   (transport layer)
 *    - config.h     (COMM_* constants and MAC addresses)
 *
 *  Interface:
 *    bool commInit();
 *    bool commPollCommand(CommCommand& outCmd);
 *
 *  Data Structures:
 *    struct CommCommand {
 *      float setpointF;    // desired setpoint (°F)
 *      bool  runFlag;      // true=ON, false=OFF
 *      uint32_t lastSeq;   // last received sequence number
 *      bool  lastOk;       // true=valid packet, false=error
 *    };
 * ================================================================
 */

#pragma once

#include <stdint.h>

// Holds the latest command received from the UI
struct CommCommand {
  float setpointF;
  bool runFlag;
  uint32_t lastSeq;
  bool lastOk;
};

// Initialize ESP-NOW communication for Control Unit
bool commInit();

// Poll for a new command from the UI (returns true if new data received)
bool commPollCommand(CommCommand& outCmd);
