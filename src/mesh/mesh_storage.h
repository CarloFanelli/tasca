#pragma once

#include <stdint.h>

#include "core/mesh_store.h"

// The Mesh history on the microSD card, in Tasca/Mesh/: the last 500 messages and the nodes heard. Blocking card
// access: setup() and loop() only.
namespace mesh_storage {

// Mounts the card and loads the saved messages and nodes into store.
void begin(MeshStore& store);
// Writes what changed in store since the last call: new messages and marks are appended, the nodes rewritten at most
// once a minute.
void save(MeshStore& store, uint32_t nowMs);
// Removes every message from store and from the card.
void clearMessages(MeshStore& store);
// False without a card, or once a write failed: the history then lasts until restart.
bool saving();

}  // namespace mesh_storage
