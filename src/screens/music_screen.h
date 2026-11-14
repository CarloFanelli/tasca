#pragma once

#include <stdint.h>

#include <vector>

#include "core/key_repeat.h"
#include "screens/reading_screen.h"

// What the iPhone is playing, with keys for its player. Opened from the Menu over the Inbox; ` goes back to the
// Inbox.
class MusicScreen : public ReadingScreen {
public:
    void open();

    void handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) override;
    // The elapsed time shown advances once per second.
    void update(uint32_t nowMs) override;
    void draw() override;

protected:
    void onKeyDown(const KeyEvent& event) override;

private:
    uint32_t shownElapsedS_ = 0;
    // ; or . held: the volume keeps changing.
    KeyRepeat volumeRepeat_{200};
};

extern MusicScreen musicScreen;
