#include "core/wall_clock.h"

#include <stdio.h>

namespace {

constexpr uint32_t kSecondsPerDay = 24 * 60 * 60;
// Year (2 bytes), month, day, hours, minutes, seconds: the rest of the value is not needed.
constexpr size_t kTimeLength = 7;

}  // namespace

bool WallClock::setFromCurrentTime(const uint8_t* data, size_t length, uint32_t nowMs) {
    if (length < kTimeLength) {
        return false;
    }
    const uint8_t hours = data[4];
    const uint8_t minutes = data[5];
    const uint8_t seconds = data[6];
    if (hours > 23 || minutes > 59 || seconds > 59) {
        return false;
    }
    secondOfDay_ = hours * 3600u + minutes * 60u + seconds;
    setMs_ = nowMs;
    known_ = true;
    return true;
}

int WallClock::minuteOfDay(uint32_t nowMs) const {
    if (!known_) {
        return -1;
    }
    // Unsigned subtraction: right across the millis() wrap.
    const uint32_t elapsedSeconds = (nowMs - setMs_) / 1000;
    return static_cast<int>((secondOfDay_ + elapsedSeconds % kSecondsPerDay) % kSecondsPerDay / 60);
}

std::string clockLabel(int minuteOfDay) {
    const unsigned minute = static_cast<unsigned>(minuteOfDay) % (24 * 60);
    char label[6];
    snprintf(label, sizeof(label), "%02u:%02u", minute / 60, minute % 60);
    return label;
}
