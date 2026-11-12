#include "core/screen_timer.h"

bool ScreenTimer::activity(uint32_t nowMs) {
    lastActivityMs_ = nowMs;
    const bool wasOff = off_;
    off_ = false;
    return wasOff;
}

bool ScreenTimer::expire(uint32_t nowMs) {
    if (off_ || timeoutMs_ == 0 || nowMs - lastActivityMs_ < timeoutMs_) {
        return false;
    }
    off_ = true;
    return true;
}
