#pragma once

#include <stdint.h>

enum class SiriAction { None, StartHold, EndHold };

// One Fn+Enter: a wake tap first (the caller sends it on start), then the Voice Command held from
// kWakeMs until Enter is released, for at least kMinHoldMs. On a locked iPhone with notifications the
// first key only lights the screen; the tap spends that key so the hold starts Siri. Times come from a
// millisecond clock that may wrap around.
class SiriPress {
public:
    static constexpr uint32_t kWakeMs = 400;
    static constexpr uint32_t kMinHoldMs = 800;

    // A new press at nowMs; false while one is active.
    bool start(uint32_t nowMs);
    // What to send now, given whether Enter is still held.
    SiriAction update(uint32_t nowMs, bool enterHeld);
    // Ends the press at once; true when a hold was down and must be released.
    bool cancel();
    bool active() const { return phase_ != Phase::Idle; }

private:
    enum class Phase { Idle, Waking, Holding };
    Phase phase_ = Phase::Idle;
    uint32_t startMs_ = 0;
    uint32_t holdMs_ = 0;
};

// After Fn+Enter the first key decides: a plain ` closes Siri (Esc) instead of its usual use, within
// kOpenMs, since iOS does not tell when Siri closed on its own. Any other key ends the window.
class SiriCloseWindow {
public:
    static constexpr uint32_t kOpenMs = 60000;

    void open(uint32_t nowMs) {
        open_ = true;
        openedMs_ = nowMs;
    }
    // A key went down at nowMs; plainBack is ` without modifiers. True when it closes Siri.
    bool keyDown(uint32_t nowMs, bool plainBack);

private:
    bool open_ = false;
    uint32_t openedMs_ = 0;
};
