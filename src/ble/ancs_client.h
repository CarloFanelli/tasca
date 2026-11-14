#pragma once

#include <stdint.h>

#include <string>

#include "core/incoming_call.h"
#include "core/message_store.h"

class NimBLEClient;

// Apple Notification Center Service client, on the connection where the Cardputer is peripheral.
namespace ancs {

enum class State { Idle, Active, Denied };

void begin();

// Blocking discovery and subscriptions on the connection's GATT client: loop() only, never from a NimBLE
// callback. True when the state changed.
bool setUp(NimBLEClient* client);
void onDisconnected();

// loop() only: decodes queued packets and sends the pending requests.
// Returns true when the state, the stored Messages or the ringing calls changed.
bool poll();

State state();
// The WhatsApp Messages received; written only from loop().
const MessageStore& messages();

// The iPhone calls ringing; written only from loop().
const CallTracker& ringingCalls();

// Answers (positive) or declines the call shown, once per call, if its notification offers that action.
// Blocking GATT write: loop() only. Returns true when the call changed.
bool actOnCall(bool positive);

// Marks every Message of chat read on the Cardputer; true when one was unread. loop() only.
bool markChatRead(const std::string& chat);

// Asks the iPhone to clear every notification of chat linked to a stored Message, one per poll(); each
// Message leaves when the iPhone reports its notification removed. loop() only.
void clearChat(const std::string& chat);
// Clear requests are still waiting to be sent.
bool clearing();
// True once per clearing that ended with Messages the iPhone refused to clear or that were not linked.
bool takeNotCleared();

// Removes every Message; those still on the iPhone come back at the next connection. loop() only.
void clearMessages();

}  // namespace ancs
