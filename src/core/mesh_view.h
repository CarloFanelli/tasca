#pragma once

#include <stdint.h>

#include <string>
#include <vector>

#include "core/chat_view.h"
#include "core/display_text.h"
#include "core/mesh_store.h"

// Time since a message arrived: "now" under a minute, then "5m", "2h", "3d".
std::string elapsedLabel(uint32_t elapsedMs);

// The Mesh screen's lines, oldest message first: a header with its arrival time "HH:MM" (its age when the clock was
// not known), sender's short name and, for the user's own messages, their mark ("sent", "heard"...), then its text
// wrapped to maxWidth.
std::vector<ChatLine> meshLines(const MeshStore& store, uint32_t nowMs, int maxWidth, const TextWidth& width);
