#pragma once

#include <stdint.h>

#include <utility>
#include <vector>

#include "core/key_events.h"

// Detects a set of keys held together for a minimum time. Fires once per hold:
// the keys must be released before it can fire again. Other keys may be held meanwhile.
class ComboHold {
public:
    ComboHold(std::vector<KeyPos> keys, uint32_t holdMs) : keys_(std::move(keys)), holdMs_(holdMs) {}

    // pressed: the keys currently down; nowMs: a millisecond clock that may wrap around.
    // Returns true on the update where the hold time is first reached.
    bool update(const std::vector<KeyPos>& pressed, uint32_t nowMs);

private:
    std::vector<KeyPos> keys_;
    uint32_t holdMs_;
    uint32_t heldSinceMs_ = 0;
    bool holding_ = false;
    bool fired_ = false;
};
