#pragma once

#include <stdint.h>

#include <string>

#include "core/mesh_store.h"

// The Mesh history on the microSD card: one line per record, tab-separated, ending in '\n'. Texts and names escape
// backslash, tab and line breaks, so a record never spans two lines.

// A message: sender, packet ID, minute of the day (-1 unknown), mark, text.
std::string historyMessageLine(const MeshMessage& message);
// A later mark of the user's message id.
std::string historyDeliveryLine(uint32_t id, Delivery delivery);
// A node: ID, last signal strength, short name, long name.
std::string historyNodeLine(const MeshNode& node);

// Applies one stored line, without its '\n', to store; false for a line that is not a record.
bool loadHistoryLine(const std::string& line, MeshStore& store);
