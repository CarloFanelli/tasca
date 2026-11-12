#include "core/chat_view.h"

#include <algorithm>

std::string timeOf(const std::string& date) {
    if (date.size() < 13) {
        return "--:--";
    }
    return date.substr(9, 2) + ":" + date.substr(11, 2);
}

std::vector<ChatLine> chatLines(const std::deque<Message>& messages, const std::string& chat, int maxWidth,
                                const TextWidth& width) {
    std::vector<const Message*> ofChat;
    for (const Message& m : messages) {
        if (m.chat == chat) {
            ofChat.push_back(&m);
        }
    }
    std::stable_sort(ofChat.begin(), ofChat.end(),
                     [](const Message* a, const Message* b) { return a->date < b->date; });
    std::vector<ChatLine> lines;
    for (const Message* m : ofChat) {
        std::string header = timeOf(m->date);
        if (isGroup(m->chat, m->sender)) {
            header += " " + displayable(m->sender, true);
        }
        lines.push_back({fitWidth(header, maxWidth, width), true});
        for (std::string& line : wrapText(displayable(m->text, false), maxWidth, width)) {
            lines.push_back({std::move(line), false});
        }
    }
    return lines;
}
