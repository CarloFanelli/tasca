#pragma once

#include <stddef.h>
#include <stdint.h>

// How near the iPhone is, from the link's signal strength: None when no reading is recent.
enum class SignalLevel : uint8_t { None, Far, Near, VeryClose };

// The Find iPhone screen's Beep period at level; 0 for None, which is silent.
uint32_t beepPeriodMs(SignalLevel level);

// The link's signal strength readings of the last kWindowMs, averaged. Times come from a millisecond clock
// that may wrap around.
class SignalMeter {
public:
    static constexpr uint32_t kWindowMs = 2000;

    // rssi in dBm; 0 means no reading and is skipped (a real reading is always negative).
    void add(int8_t rssi, uint32_t nowMs);
    void clear() { count_ = 0; }
    SignalLevel level(uint32_t nowMs) const;
    // 0 to 100: empty with no recent reading.
    int barPercent(uint32_t nowMs) const;

private:
    // Readings arrive about every 250 ms: 16 cover the window twice.
    static constexpr size_t kCapacity = 16;

    // False when no reading is recent.
    bool average(uint32_t nowMs, int* rssi) const;

    int8_t rssi_[kCapacity] = {};
    uint32_t atMs_[kCapacity] = {};
    size_t next_ = 0;
    size_t count_ = 0;
};
