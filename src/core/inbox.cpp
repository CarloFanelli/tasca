#include "core/inbox.h"

#include <algorithm>

namespace {

struct Entry {
    ChatSummary summary;
    std::string date;
    size_t position;  // in the store, of the newest Message
};

}  // namespace

std::string unreadLabel(size_t count) {
    return count > 99 ? "99+" : std::to_string(count);
}

std::vector<ChatSummary> buildInbox(const std::deque<Message>& messages) {
    std::vector<Entry> entries;
    for (size_t i = 0; i < messages.size(); i++) {
        const Message& m = messages[i];
        auto it = std::find_if(entries.begin(), entries.end(),
                               [&](const Entry& e) { return e.summary.chat == m.chat; });
        if (it == entries.end()) {
            entries.push_back({{m.chat, "", "", 0, false}, "", 0});
            it = entries.end() - 1;
        }
        it->summary.unread += m.read ? 0 : 1;
        // dates are yyyyMMdd'T'HHmmSS: string order is time order.
        if (m.date >= it->date) {
            it->summary.sender = m.sender;
            it->summary.text = m.text;
            it->summary.group = isGroup(m.chat, m.sender);
            it->date = m.date;
            it->position = i;
        }
    }
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        return a.date != b.date ? a.date > b.date : a.position > b.position;
    });
    std::vector<ChatSummary> chats;
    chats.reserve(entries.size());
    for (Entry& e : entries) {
        chats.push_back(std::move(e.summary));
    }
    return chats;
}
