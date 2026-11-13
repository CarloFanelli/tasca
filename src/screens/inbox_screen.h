#pragma once

#include "screens/reading_screen.h"

// The list of Chats, newest first; always at the bottom of the Screen stack.
class InboxScreen : public ReadingScreen {
public:
    void draw() override;

protected:
    void onKeyDown(const KeyEvent& event) override;
};

extern InboxScreen inboxScreen;
