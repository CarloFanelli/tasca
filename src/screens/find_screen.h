#pragma once

#include <stdint.h>

#include <vector>

#include "core/signal_meter.h"
#include "screens/reading_screen.h"

// Guides to the iPhone by the link's signal strength: a bar, Far, Near or Very close, and a Beep faster closer
// to it, played even with sound off. Opened from the Menu over the Inbox; ` goes back to the Inbox.
class FindScreen : public ReadingScreen {
public:
    // Turns Find iPhone on, as it stays from then on.
    void open();

    void handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) override;
    // Reads the signal about four times per second and plays the Beep when due.
    void update(uint32_t nowMs) override;
    void draw() override;

protected:
    void onKeyDown(const KeyEvent& event) override;

private:
    SignalMeter meter_;
    SignalLevel shownLevel_ = SignalLevel::None;
    int shownBar_ = 0;
    uint32_t nextReadMs_ = 0;
    uint32_t nextBeepMs_ = 0;
    uint32_t nextLogMs_ = 0;
    // The last key pressed here: the Beeps stop after kQuietAfterMs without one.
    uint32_t lastKeyMs_ = 0;
};

extern FindScreen findScreen;
