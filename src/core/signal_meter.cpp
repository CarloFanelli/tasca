#include "core/signal_meter.h"

namespace {

// Average signal at or above which a level starts, measured on the user's iPhone: about -33 dBm touching its
// top, -55 at 30 cm, -66 at 1 m, -75 to -81 at 3 m and at 5 m alike.
constexpr int kVeryCloseRssi = -70;
constexpr int kNearRssi = -76;
// The bar's empty and full ends.
constexpr int kBarEmptyRssi = -90;
constexpr int kBarFullRssi = -30;

}  // namespace

uint32_t beepPeriodMs(SignalLevel level) {
    switch (level) {
        case SignalLevel::Far:       return 2000;
        case SignalLevel::Near:      return 1000;
        case SignalLevel::VeryClose: return 250;
        case SignalLevel::None:      break;
    }
    return 0;
}

void SignalMeter::add(int8_t rssi, uint32_t nowMs) {
    if (rssi >= 0) {
        return;
    }
    rssi_[next_] = rssi;
    atMs_[next_] = nowMs;
    next_ = (next_ + 1) % kCapacity;
    if (count_ < kCapacity) {
        count_++;
    }
}

bool SignalMeter::average(uint32_t nowMs, int* rssi) const {
    int sum = 0;
    int recent = 0;
    for (size_t i = 0; i < count_; i++) {
        if (nowMs - atMs_[i] <= kWindowMs) {
            sum += rssi_[i];
            recent++;
        }
    }
    if (recent == 0) {
        return false;
    }
    *rssi = sum / recent;
    return true;
}

SignalLevel SignalMeter::level(uint32_t nowMs) const {
    int rssi = 0;
    if (!average(nowMs, &rssi)) {
        return SignalLevel::None;
    }
    if (rssi >= kVeryCloseRssi) {
        return SignalLevel::VeryClose;
    }
    return rssi >= kNearRssi ? SignalLevel::Near : SignalLevel::Far;
}

int SignalMeter::barPercent(uint32_t nowMs) const {
    int rssi = 0;
    if (!average(nowMs, &rssi) || rssi <= kBarEmptyRssi) {
        return 0;
    }
    if (rssi >= kBarFullRssi) {
        return 100;
    }
    return (rssi - kBarEmptyRssi) * 100 / (kBarFullRssi - kBarEmptyRssi);
}
