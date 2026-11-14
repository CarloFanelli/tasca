#pragma once

#include <stdint.h>

#include "screens/reading_screen.h"

// Brightness, sound, display timeout, wake on new message and Find iPhone, then the actions, each run on a
// second Enter. Opened over the Inbox or Chat screen, which stays as it was.
class SettingsScreen : public ReadingScreen {
public:
    // fromStart: the first row selected, as when opened from the Menu.
    void open(bool fromStart = false);
    // Cancels an action waiting for confirmation; true when one was.
    bool cancelPrompt();

    void update(uint32_t nowMs) override;
    // An action waiting for confirmation is cancelled, as by any other key.
    void onCovered() override { cancelPrompt(); }
    void draw() override;

protected:
    void onKeyDown(const KeyEvent& event) override;
};

extern SettingsScreen settingsScreen;
