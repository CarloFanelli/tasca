#pragma once

#include <vector>

#include "core/chat_view.h"
#include "screens/reading_screen.h"

// The Messages of the Chat selected in the Inbox, oldest to newest.
class ChatScreen : public ReadingScreen {
public:
    // Opens the selected Chat over the Inbox; nothing when the Inbox is empty.
    void open();
    // Back to the Inbox, also when the Chat screen is covered.
    void close();
    // Rebuilds the lines from the store and clamps the scroll to them.
    void layout();
    // The Chat screen on display shows its Messages: they are read.
    void markRead();

    void draw() override;
    // Back on top: what arrived meanwhile is now on display.
    void onShown() override { markRead(); }

protected:
    void onKeyDown(const KeyEvent& event) override;

private:
    // Rebuilt only when the Chat's content changes.
    std::vector<ChatLine> lines_;
};

extern ChatScreen chatScreen;
