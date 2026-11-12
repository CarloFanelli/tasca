#pragma once

#include <deque>
#include <string>
#include <vector>

#include "core/display_text.h"
#include "core/message_store.h"

// One line of the Chat screen: a Message's header (time, and sender in groups) or a line of its text.
struct ChatLine {
    std::string text;
    bool header = false;
};

// "HH:MM" of a date yyyyMMdd'T'HHmmSS; "--:--" when the date is too short.
std::string timeOf(const std::string& date);

// The Messages of chat, oldest to newest by date, as lines no wider than maxWidth. Text is made
// displayable; line breaks in it are kept.
std::vector<ChatLine> chatLines(const std::deque<Message>& messages, const std::string& chat, int maxWidth,
                                const TextWidth& width);
