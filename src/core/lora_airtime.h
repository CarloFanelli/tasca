#pragma once

#include <stddef.h>
#include <stdint.h>

#include <deque>

// Time on air of a packet of size bytes with the Meshtastic MediumFast settings (SF9, 250 kHz, CR 4/5, preamble 16).
uint32_t loraAirtimeMs(size_t size);

// The 869.4-869.65 MHz band allows 10% of the time on air: 360 s in any hour.
class AirtimeBudget {
public:
    // True when airtimeMs more stays within the last hour's limit.
    bool allows(uint32_t nowMs, uint32_t airtimeMs);
    void spend(uint32_t nowMs, uint32_t airtimeMs);

private:
    struct Spent {
        uint32_t atMs;
        uint32_t airtimeMs;
    };
    void forgetOld(uint32_t nowMs);

    std::deque<Spent> spent_;
};
