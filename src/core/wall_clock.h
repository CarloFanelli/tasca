#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>

// The time of day read from the iPhone, kept running on millis() until restart. loop() only.
class WallClock {
public:
    // Sets the clock from a Current Time value (Bluetooth 0x2A2B, local time) read at nowMs; false, leaving the
    // clock as it was, when the value is short or not a valid time.
    bool setFromCurrentTime(const uint8_t* data, size_t length, uint32_t nowMs);
    bool known() const { return known_; }
    // The minute of the day at nowMs (0 to 1439), or -1 when the clock is not known.
    int minuteOfDay(uint32_t nowMs) const;

private:
    bool known_ = false;
    uint32_t secondOfDay_ = 0;
    uint32_t setMs_ = 0;
};

// A minute of the day as "HH:MM".
std::string clockLabel(int minuteOfDay);
