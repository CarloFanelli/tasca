#pragma once

#include <stdint.h>

#include <vector>

#include "core/key_events.h"

// An action repeated while its key is held: started on the press, then due every intervalMs until the key is
// released.
class KeyRepeat {
public:
    explicit KeyRepeat(uint32_t intervalMs) : intervalMs_(intervalMs) {}

    // The press of key at nowMs, a millisecond clock that may wrap around, did the action once.
    void start(KeyPos key, uint32_t nowMs);
    // True each time the interval has passed with the key still held; once released, false until started again.
    bool update(const std::vector<KeyPos>& pressed, uint32_t nowMs);
    KeyPos key() const { return key_; }

private:
    uint32_t intervalMs_;
    bool active_ = false;
    KeyPos key_{};
    uint32_t lastMs_ = 0;
};
