#include "core/key_repeat.h"

void KeyRepeat::start(KeyPos key, uint32_t nowMs) {
    active_ = true;
    key_ = key;
    lastMs_ = nowMs;
}

bool KeyRepeat::update(const std::vector<KeyPos>& pressed, uint32_t nowMs) {
    if (!active_) {
        return false;
    }
    if (!containsKey(pressed, key_)) {
        active_ = false;
        return false;
    }
    if (nowMs - lastMs_ < intervalMs_) {
        return false;
    }
    lastMs_ = nowMs;
    return true;
}
