#pragma once

#include <stddef.h>

#include <string>
#include <vector>

#include "core/inbox.h"

// Where the user is in Reading mode: the Inbox with its selected Chat and scroll, or the Chat screen
// of one Chat with its scroll.
class ReadingNav {
public:
    // Takes a rebuilt Inbox: the selected Chat stays selected wherever it moved; when it is gone,
    // the selection keeps its position, clamped to the list.
    void sync(const std::vector<ChatSummary>& chats);

    // Moves the selection by one; false at the first or last Chat.
    bool up();
    bool down();

    // Index of the selected Chat in the last synced Inbox; 0 when it is empty.
    size_t selected() const { return index_; }

    // First Chat to draw in a window of rows Chats, scrolled just enough to show the selection.
    size_t firstVisible(size_t rows);

    // Opens the Chat screen of the selected Chat, scrolled to its end; false when the Inbox is empty.
    bool openChat();
    void closeChat();
    bool inChat() const { return inChat_; }
    // The Chat whose screen is open.
    const std::string& openedChat() const { return openedChat_; }

    // Takes the Chat screen's size: total lines, how many fit, and the line where the newest Message
    // starts. A screen just opened, or showing its last line, shows the newest Message from its start,
    // with earlier lines above it when it is short; otherwise the scroll stays, clamped to the new size.
    void setChatLines(size_t total, size_t visible, size_t newestStart);
    // Scrolls the Chat screen by one line; false at the top or bottom.
    bool scrollUp();
    bool scrollDown();
    // First line of the Chat screen to draw.
    size_t chatTop() const { return chatTop_; }

private:
    void select(size_t index);

    std::vector<std::string> chats_;
    std::string selectedChat_;
    size_t index_ = 0;
    size_t top_ = 0;
    bool inChat_ = false;
    std::string openedChat_;
    size_t chatTop_ = 0;
    size_t chatTotal_ = 0;
    size_t chatVisible_ = 0;
};
