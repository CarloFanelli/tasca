#pragma once

#include <stdint.h>

#include <vector>

#include "core/screen.h"

// An iPhone call ringing, over any screen and mode: "Incoming call" and the caller. While it is open no
// key reaches the iPhone.
class CallScreen : public Screen {
public:
    // Opens over the top screen when a call rings, and closes back to it when none does. Every loop.
    void sync(const std::vector<KeyPos>& pressed);

    void handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) override;
    void update(uint32_t nowMs) override;
    void draw() override;
};

extern CallScreen callScreen;
