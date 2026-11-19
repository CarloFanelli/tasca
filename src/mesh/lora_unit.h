#pragma once

#include <string>

#include "core/mesh_store.h"

// The LoRa unit on the Grove port, listening to the Meshtastic public channel. loop() only.
namespace lora {

void begin();
// Talks to the unit and stores what it heard; true when the unit came or went, or the Mesh screen's content changed.
bool poll();
// Found, set up and listening.
bool present();
// The Mesh name chosen in Settings, empty for the default: announced to the other nodes when it changes, when the
// unit is found and every 3 hours.
void setMeshName(const std::string& chosen);
// The name announced: the one chosen, or the default.
const std::string& meshName();
// Sends text to every node on the public channel and lists it as the user's message; it waits while the unit sends
// another packet, and reads "not sent" when the unit is missing or goes. False, with nothing listed, when it would
// pass 10% of the last hour on air.
bool sendText(const std::string& text);
MeshStore& messages();

}  // namespace lora
