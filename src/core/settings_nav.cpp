#include "core/settings_nav.h"

bool SettingsNav::up() {
    if (selected_ == 0) {
        return false;
    }
    selected_--;
    return true;
}

bool SettingsNav::down() {
    if (selected_ + 1 >= rows_) {
        return false;
    }
    selected_++;
    return true;
}

bool SettingsNav::enter(uint32_t nowMs) {
    if (!onAction()) {
        return false;
    }
    if (confirming_) {
        confirming_ = false;
        return true;
    }
    confirming_ = true;
    promptSinceMs_ = nowMs;
    return false;
}

bool SettingsNav::cancel() {
    const bool was = confirming_;
    confirming_ = false;
    return was;
}

bool SettingsNav::expire(uint32_t nowMs) {
    return confirming_ && nowMs - promptSinceMs_ >= confirmMs_ && cancel();
}
