#pragma once

#include <stdint.h>

#include <vector>

#include "core/text_entry.h"
#include "screens/reading_screen.h"

// Brightness, sound, display timeout, wake on new message, Find iPhone and Mesh sound, then the Mesh name, edited
// in place, then the actions, each run on a second Enter. Opened over the Inbox or Chat screen, which stays as it
// was.
class SettingsScreen : public ReadingScreen {
public:
    SettingsScreen();

    // fromStart: the first row selected, as when opened from the Menu.
    void open(bool fromStart = false);
    // Cancels an action waiting for confirmation; true when one was.
    bool cancelPrompt();

    void update(uint32_t nowMs) override;
    // While the Mesh name is edited, keys type it.
    void handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) override;
    // An action waiting for confirmation is cancelled, as by any other key, and so is an edit.
    void onCovered() override;
    void draw() override;

protected:
    void onKeyDown(const KeyEvent& event) override;

private:
    void drawNameRow(int top, bool selected);

    // The keys held this loop, for starting an edit.
    std::vector<KeyPos> pressed_;
    bool editing_ = false;
    TextEntry nameEntry_;
};

extern SettingsScreen settingsScreen;
