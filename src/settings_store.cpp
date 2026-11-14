#include "settings_store.h"

#include <Preferences.h>

#include "log.h"

namespace settings_store {

namespace {

Preferences preferences;
bool opened = false;

// Storage keys, in the order of Setting; at most 15 characters.
constexpr const char* kKeys[kSettingCount] = {"brightness", "sound", "timeout", "wake", "findiphone"};

const char* keyOf(Setting setting) {
    return kKeys[static_cast<size_t>(setting)];
}

}  // namespace

void load(Settings& settings) {
    // Read-write: a read-only open fails on first start, before the namespace exists.
    opened = preferences.begin("settings", false);
    if (!opened) {
        logMessage(LogLevel::Warn, "SYS", "settings storage unavailable: defaults used");
        return;
    }
    for (size_t i = 0; i < kSettingCount; i++) {
        const Setting setting = static_cast<Setting>(i);
        if (!preferences.isKey(keyOf(setting))) {
            continue;
        }
        if (!settings.setValueIndex(setting, preferences.getUChar(keyOf(setting), 0xFF))) {
            logMessage(LogLevel::Warn, "SYS", "stored setting %s unreadable: default used", keyOf(setting));
        }
    }
}

void save(const Settings& settings, Setting setting) {
    if (!opened || preferences.putUChar(keyOf(setting), settings.valueIndex(setting)) == 0) {
        logMessage(LogLevel::Warn, "SYS", "setting %s not saved: kept until restart", keyOf(setting));
    }
}

}  // namespace settings_store
