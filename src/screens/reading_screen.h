#pragma once

#include "core/screen.h"

// A Reading mode screen: keys stay on the Cardputer. Handles what every Reading mode screen shares
// (pairing reset, waking the screen, silencing the Out-of-range alert, Siri), then hands each other key press to
// onKeyDown.
class ReadingScreen : public Screen {
public:
    void handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) override;
    // Shows "Not cleared on the iPhone" when a clearing left Messages; removes it or the prompt after the wait.
    void update(uint32_t nowMs) override;

protected:
    virtual void onKeyDown(const KeyEvent& event) = 0;
};
