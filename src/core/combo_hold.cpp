#include "core/combo_hold.h"

bool ComboHold::update(const std::vector<KeyPos>& pressed, uint32_t nowMs) {
    for (const KeyPos& key : keys_) {
        if (!containsKey(pressed, key)) {
            holding_ = false;
            fired_ = false;
            return false;
        }
    }
    if (!holding_) {
        holding_ = true;
        heldSinceMs_ = nowMs;
    }
    if (fired_ || nowMs - heldSinceMs_ < holdMs_) {
        return false;
    }
    fired_ = true;
    return true;
}
