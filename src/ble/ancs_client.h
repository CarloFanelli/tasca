#pragma once

#include <stdint.h>

#include "core/message_store.h"

// Apple Notification Center Service client, on the connection where the Cardputer is peripheral.
namespace ancs {

enum class State { Idle, Active, Denied };

void begin();

// Called from loop() by the BLE manager. onSecured may repeat for the same connection.
void onSecured(uint16_t connHandle);
void onDisconnected();

// loop() only: runs the pending setup (blocking GATT calls) and decodes queued packets.
// Returns true when the state or the stored Messages changed.
bool poll();

State state();
// The WhatsApp Messages received; written only by poll().
const MessageStore& messages();

}  // namespace ancs
