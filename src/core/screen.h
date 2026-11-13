#pragma once

#include <stdint.h>

#include <vector>

#include "core/key_events.h"

// One screen of the Cardputer. The Screen stack calls the hooks; the main loop calls the rest on the
// screen on top only.
class Screen {
public:
    virtual ~Screen() = default;

    // The keys pressed now and the press and release events since the previous loop.
    virtual void handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) = 0;
    virtual void update(uint32_t nowMs) { (void)nowMs; }
    virtual void draw() = 0;

    // It became the screen on top: just opened, or back on top after the one above closed.
    virtual void onShown() {}
    // Another screen opened over it.
    virtual void onCovered() {}
};
