#include "core/lora_airtime.h"

namespace {

constexpr uint32_t kSpreadingFactor = 9;
constexpr uint32_t kBandwidthHz = 250000;
// Coding rate 4/5.
constexpr uint32_t kCodingRate = 1;
constexpr uint32_t kPreambleSymbols = 16;
constexpr uint32_t kWindowMs = 3600000;
constexpr uint32_t kLimitMs = kWindowMs / 10;

}  // namespace

// Semtech's LoRa time on air, with an explicit header, CRC on and no low data rate optimisation (symbols of 2 ms).
uint32_t loraAirtimeMs(size_t size) {
    const uint32_t symbolUs = (1u << kSpreadingFactor) * 1000000u / kBandwidthHz;
    const int32_t bits = static_cast<int32_t>(8 * size) - 4 * static_cast<int32_t>(kSpreadingFactor) + 28 + 16;
    const int32_t perBlock = 4 * static_cast<int32_t>(kSpreadingFactor);
    const uint32_t blocks = bits > 0 ? static_cast<uint32_t>((bits + perBlock - 1) / perBlock) : 0;
    const uint32_t payloadSymbols = 8 + blocks * (kCodingRate + 4);
    // The preamble lasts its symbols plus 4.25.
    const uint32_t preambleUs = kPreambleSymbols * symbolUs + symbolUs * 17 / 4;
    return (preambleUs + payloadSymbols * symbolUs + 500) / 1000;
}

bool AirtimeBudget::allows(uint32_t nowMs, uint32_t airtimeMs) {
    forgetOld(nowMs);
    uint32_t total = airtimeMs;
    for (const Spent& s : spent_) {
        total += s.airtimeMs;
    }
    return total <= kLimitMs;
}

void AirtimeBudget::spend(uint32_t nowMs, uint32_t airtimeMs) {
    spent_.push_back({nowMs, airtimeMs});
}

void AirtimeBudget::forgetOld(uint32_t nowMs) {
    while (!spent_.empty() && nowMs - spent_.front().atMs >= kWindowMs) {
        spent_.pop_front();
    }
}
