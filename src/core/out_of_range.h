#pragma once

#include <stdint.h>

// The Out-of-range alert: started when the link to the iPhone is lost by signal, it stays on until the iPhone
// returns. Its triple Beep is due at the start, then every kPeriodMs for kBeepingMs, unless silenced. Times
// come from a millisecond clock that may wrap around.
class OutOfRangeAlert {
public:
    static constexpr uint32_t kPeriodMs = 5000;
    static constexpr uint32_t kBeepingMs = 120000;

    void start(uint32_t nowMs);
    // Ends the Beeps, the alert stays on; true when they were still due.
    bool silence();
    void stop();
    bool active() const { return active_; }
    // True when a triple Beep is due at nowMs; the next one is a full period later.
    bool due(uint32_t nowMs);

private:
    bool active_ = false;
    bool beeping_ = false;
    uint32_t startMs_ = 0;
    uint32_t nextMs_ = 0;
};
