#pragma once

#include <stdint.h>

#include "core/keymap.h"

class NimBLEServer;

namespace ble {

// Creates the HID (keyboard + consumer control), Battery and Device Information services.
// The table is final: iOS caches it at pairing, so it must not change once a device has bonded.
void createHidServices(NimBLEServer* server, uint8_t batteryLevel);

// True once the iPhone has enabled the keyboard input report on the current connection.
bool keyboardReportsEnabled();

// loop() only. False when the report could not be queued for sending.
bool sendKeyboardReport(const KeyboardReport& report);

// loop() only. Sends a consumer control usage (e.g. Eject) and its release. False when either
// report could not be queued for sending.
bool sendConsumerUsage(uint16_t usage);
// Sends a consumer control report holding usage down until the next call; 0 releases it.
bool setConsumerUsage(uint16_t usage);

}  // namespace ble
