#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>

// The user's settings, each one value out of a fixed list.
enum class Setting : uint8_t { Brightness, Sound, DisplayTimeout, WakeOnMessage, FindIphone, MeshSound };
constexpr size_t kSettingCount = 6;

// The setting's name as listed on the Settings screen.
const char* settingName(Setting setting);

class Settings {
public:
    // Every setting at its default.
    Settings();

    // Position of the current value in the setting's list: what is stored.
    uint8_t valueIndex(Setting setting) const;
    // Restores a stored position; false, keeping the current value, when it is outside the list.
    bool setValueIndex(Setting setting, uint8_t index);
    // Moves the value one step back (direction < 0) or forward; false at the end of the list.
    bool step(Setting setting, int direction);
    // The value as shown: "60 %", "On", "30 s", "1 min", "Never".
    std::string label(Setting setting) const;

    uint8_t brightnessPercent() const;
    bool sound() const;
    // 0 means Never.
    uint32_t displayTimeoutMs() const;
    bool wakeOnMessage() const;
    // The Out-of-range alert and the Find iPhone screen's Beeps, played even with sound off.
    bool findIphone() const;
    // A beep for each Mesh message, whatever sound says.
    bool meshSound() const;

private:
    uint8_t index_[kSettingCount];
};
