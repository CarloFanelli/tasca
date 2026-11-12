#pragma once

#include <stdint.h>

#include <string>

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
// The WhatsApp Messages received; written only from loop().
const MessageStore& messages();

// Marks every Message of chat read on the Cardputer; true when one was unread. loop() only.
bool markChatRead(const std::string& chat);

}  // namespace ancs
