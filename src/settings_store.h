#pragma once

#include <string>

#include "core/settings.h"

// Keeps the settings in the ESP32's non-volatile storage across restarts and pairing resets.
namespace settings_store {

// Reads every stored setting into settings; a missing one keeps its default.
void load(Settings& settings);
// Stores one setting's current value.
void save(const Settings& settings, Setting setting);
// The Mesh name chosen in Settings; empty when none was.
std::string loadMeshName();
void saveMeshName(const std::string& name);

}  // namespace settings_store
