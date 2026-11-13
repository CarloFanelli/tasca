#include "core/siri_press.h"

bool SiriPress::start(uint32_t nowMs) {
    if (active()) {
        return false;
    }
    phase_ = Phase::Waking;
    startMs_ = nowMs;
    return true;
}

SiriAction SiriPress::update(uint32_t nowMs, bool enterHeld) {
    switch (phase_) {
        case Phase::Idle:
            break;
        case Phase::Waking:
            if (nowMs - startMs_ >= kWakeMs) {
                phase_ = Phase::Holding;
                holdMs_ = nowMs;
                return SiriAction::StartHold;
            }
            break;
        case Phase::Holding:
            if (!enterHeld && nowMs - holdMs_ >= kMinHoldMs) {
                phase_ = Phase::Idle;
                return SiriAction::EndHold;
            }
            break;
    }
    return SiriAction::None;
}

bool SiriPress::cancel() {
    const bool holding = phase_ == Phase::Holding;
    phase_ = Phase::Idle;
    return holding;
}

bool SiriCloseWindow::keyDown(uint32_t nowMs, bool plainBack) {
    if (!open_) {
        return false;
    }
    open_ = false;
    return plainBack && nowMs - openedMs_ < kOpenMs;
}
