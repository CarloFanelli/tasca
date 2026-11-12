#pragma once

#include <stdint.h>

// Turns the screen off after a time without activity. Times come from a millisecond clock that may
// wrap around.
class ScreenTimer {
public:
    // The time without activity before the screen turns off; 0 keeps it on.
    void setTimeout(uint32_t ms) { timeoutMs_ = ms; }
    // Activity at nowMs: the timeout restarts. True when the screen was off: it turns on.
    bool activity(uint32_t nowMs);
    // True once, when the timeout has passed since the last activity: the screen turns off.
    bool expire(uint32_t nowMs);
    bool off() const { return off_; }

private:
    uint32_t timeoutMs_ = 0;
    uint32_t lastActivityMs_ = 0;
    bool off_ = false;
};
