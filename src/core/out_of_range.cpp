#include "core/out_of_range.h"

void OutOfRangeAlert::start(uint32_t nowMs) {
    active_ = true;
    beeping_ = true;
    startMs_ = nowMs;
    nextMs_ = nowMs;
}

bool OutOfRangeAlert::silence() {
    const bool was = beeping_;
    beeping_ = false;
    return was;
}

void OutOfRangeAlert::stop() {
    active_ = false;
    beeping_ = false;
}

bool OutOfRangeAlert::due(uint32_t nowMs) {
    if (!beeping_ || static_cast<int32_t>(nowMs - nextMs_) < 0) {
        return false;
    }
    if (nowMs - startMs_ >= kBeepingMs) {
        beeping_ = false;
        return false;
    }
    nextMs_ = nowMs + kPeriodMs;
    return true;
}
