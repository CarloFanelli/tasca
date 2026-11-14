#pragma once

#include "core/ams.h"

class NimBLEClient;

// Apple Media Service client: what the iPhone plays, on the connection ANCS uses.
namespace ams {

void begin();

// Blocking discovery and subscription on the connection's GATT client: loop() only.
void setUp(NimBLEClient* client);
// The iPhone changed its services: AMS alone is discovered again. loop() only.
void onServicesChanged(NimBLEClient* client);
void onDisconnected();

// loop() only: decodes queued updates, reading a cut value whole. True when what the Music screen shows
// changed, the offered commands included.
bool poll();

// Subscribed to the iPhone's now-playing data.
bool available();
const NowPlaying& nowPlaying();

// The current player offers command; every command until the iPhone lists them.
bool offered(AmsCommand command);
// Sends command to the current player. Blocking GATT write: loop() only.
void send(AmsCommand command);

}  // namespace ams
