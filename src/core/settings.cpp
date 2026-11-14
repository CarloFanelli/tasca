#include "core/settings.h"

namespace {

constexpr uint8_t kBrightnessPercent[] = {20, 40, 60, 80, 100};
// 0 means Never.
constexpr uint32_t kDisplayTimeoutMs[] = {15000, 30000, 60000, 120000, 300000, 0};

struct Spec {
    const char* name;
    uint8_t count;
    uint8_t defaultIndex;
};

// Sound, wake on message and Find iPhone: index 0 is Off, 1 is On.
constexpr Spec kSpecs[kSettingCount] = {
    {"Brightness", sizeof(kBrightnessPercent), 2},
    {"Sound", 2, 1},
    {"Display timeout", sizeof(kDisplayTimeoutMs) / sizeof(kDisplayTimeoutMs[0]), 1},
    {"Wake on message", 2, 1},
    {"Find iPhone", 2, 1},
};

const Spec& spec(Setting setting) {
    return kSpecs[static_cast<size_t>(setting)];
}

std::string timeoutLabel(uint32_t ms) {
    if (ms == 0) {
        return "Never";
    }
    if (ms < 60000) {
        return std::to_string(ms / 1000) + " s";
    }
    return std::to_string(ms / 60000) + " min";
}

}  // namespace

const char* settingName(Setting setting) {
    return spec(setting).name;
}

Settings::Settings() {
    for (size_t i = 0; i < kSettingCount; i++) {
        index_[i] = kSpecs[i].defaultIndex;
    }
}

uint8_t Settings::valueIndex(Setting setting) const {
    return index_[static_cast<size_t>(setting)];
}

bool Settings::setValueIndex(Setting setting, uint8_t index) {
    if (index >= spec(setting).count) {
        return false;
    }
    index_[static_cast<size_t>(setting)] = index;
    return true;
}

bool Settings::step(Setting setting, int direction) {
    const uint8_t index = valueIndex(setting);
    if (direction < 0) {
        return index > 0 && setValueIndex(setting, index - 1);
    }
    return setValueIndex(setting, index + 1);
}

std::string Settings::label(Setting setting) const {
    const uint8_t index = valueIndex(setting);
    switch (setting) {
        case Setting::Brightness:
            return std::to_string(kBrightnessPercent[index]) + " %";
        case Setting::DisplayTimeout:
            return timeoutLabel(kDisplayTimeoutMs[index]);
        case Setting::Sound:
        case Setting::WakeOnMessage:
        case Setting::FindIphone:
            break;
    }
    return index == 1 ? "On" : "Off";
}

uint8_t Settings::brightnessPercent() const {
    return kBrightnessPercent[valueIndex(Setting::Brightness)];
}

bool Settings::sound() const {
    return valueIndex(Setting::Sound) == 1;
}

uint32_t Settings::displayTimeoutMs() const {
    return kDisplayTimeoutMs[valueIndex(Setting::DisplayTimeout)];
}

bool Settings::wakeOnMessage() const {
    return valueIndex(Setting::WakeOnMessage) == 1;
}

bool Settings::findIphone() const {
    return valueIndex(Setting::FindIphone) == 1;
}
