#pragma once

#include <stddef.h>

#include <deque>
#include <string>
#include <vector>

#include "core/message_store.h"

// One Chat of the Inbox, as of its newest Message.
struct ChatSummary {
    std::string chat;
    std::string sender;  // of the newest Message
    std::string text;    // of the newest Message
    size_t unread = 0;
    bool group = false;  // the newest Message's sender is not the Chat itself
};

// An unread count as shown on screen: "99+" above 99.
std::string unreadLabel(size_t count);

// Every Chat of the stored Messages once, the one with the newest Message first. "Newest" is the
// latest date; between equal dates, the Message stored later.
std::vector<ChatSummary> buildInbox(const std::deque<Message>& messages);
