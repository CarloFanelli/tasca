#pragma once

#include <stdint.h>

#include <stddef.h>

#include <string>

// The longest Mesh name the user can type: 24 characters, within the 39 bytes Meshtastic nodes accept.
constexpr size_t kMeshNameChars = 24;
constexpr size_t kMeshNameBytes = 39;

// The user's Mesh name: the one chosen in Settings, or "Tasca" and the last 4 hex digits of the node ID when none
// is; cut to 39 bytes, the most Meshtastic nodes accept.
std::string meshLongName(const std::string& chosen, uint32_t node);

// The first 4 characters of the Mesh name, within the 4 bytes Meshtastic nodes accept.
std::string meshShortName(const std::string& longName);
