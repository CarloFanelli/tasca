#pragma once

#include <stdint.h>

#include <vector>

#include "core/screen.h"

// While the iPhone's link is not secured: "Tasca", the line that says what to do, and the battery, over
// every other screen. Keys do nothing on it but the pairing reset.
class ConnectionScreen : public Screen {
public:
    // Opens over the top screen while the link is not secured, and closes back to it once it is, without
    // turning the screen on. Every loop.
    void sync(const std::vector<KeyPos>& pressed);

    void handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) override;
    void draw() override;
};

extern ConnectionScreen connectionScreen;
