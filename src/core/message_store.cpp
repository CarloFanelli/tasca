#include "core/message_store.h"

namespace {

size_t bytesOf(const Message& m) {
    return sizeof(Message) + m.chat.size() + m.sender.size() + m.text.size() + m.date.size();
}

}  // namespace

Message* MessageStore::linkedTo(uint32_t uid) {
    for (Message& m : messages_) {
        if (m.linked && m.uid == uid) {
            return &m;
        }
    }
    return nullptr;
}

MessageStore::Result MessageStore::receive(uint32_t uid, bool preExisting, const ChatKey& key,
                                           const std::string& text, const std::string& date) {
    if (Message* m = linkedTo(uid)) {
        memoryBytes_ -= bytesOf(*m);
        m->chat = key.chat;
        m->sender = key.sender;
        m->text = text;
        m->date = date;
        memoryBytes_ += bytesOf(*m);
        dropOverLimits();
        return Result::Updated;
    }
    if (preExisting) {
        // Only unlinked Messages: one already linked in this connection is a different notification.
        for (auto it = messages_.rbegin(); it != messages_.rend(); ++it) {
            if (!it->linked && it->date == date && it->chat == key.chat && it->sender == key.sender &&
                it->text == text) {
                it->uid = uid;
                it->linked = true;
                return Result::Linked;
            }
        }
    }
    Message message;
    message.chat = key.chat;
    message.sender = key.sender;
    message.text = text;
    message.date = date;
    message.uid = uid;
    message.linked = true;
    memoryBytes_ += bytesOf(message);
    messages_.push_back(std::move(message));
    if (!preExisting) {
        liveAdded_++;
        hasLive_ = true;
        newestLiveUid_ = uid;
    }
    dropOverLimits();
    return Result::Added;
}

bool MessageStore::remove(uint32_t uid) {
    for (auto it = messages_.begin(); it != messages_.end(); ++it) {
        if (it->linked && it->uid == uid) {
            memoryBytes_ -= bytesOf(*it);
            messages_.erase(it);
            return true;
        }
    }
    return false;
}

size_t MessageStore::removeUnlinked() {
    const size_t before = messages_.size();
    for (auto it = messages_.begin(); it != messages_.end();) {
        if (it->linked) {
            ++it;
        } else {
            memoryBytes_ -= bytesOf(*it);
            it = messages_.erase(it);
        }
    }
    return before - messages_.size();
}

size_t MessageStore::markChatRead(const std::string& chat) {
    size_t changed = 0;
    for (Message& m : messages_) {
        if (m.chat == chat && !m.read) {
            m.read = true;
            changed++;
        }
    }
    return changed;
}

std::vector<uint32_t> MessageStore::linkedUids(const std::string& chat) const {
    std::vector<uint32_t> uids;
    for (const Message& m : messages_) {
        if (m.chat == chat && m.linked) {
            uids.push_back(m.uid);
        }
    }
    return uids;
}

size_t MessageStore::unlinkedCount(const std::string& chat) const {
    size_t count = 0;
    for (const Message& m : messages_) {
        count += m.chat == chat && !m.linked ? 1 : 0;
    }
    return count;
}

void MessageStore::clear() {
    messages_.clear();
    memoryBytes_ = 0;
}

void MessageStore::unlinkAll() {
    for (Message& m : messages_) {
        m.linked = false;
    }
}

void MessageStore::dropOverLimits() {
    while (messages_.size() > kMaxMessages || memoryBytes_ > kMaxStoreBytes) {
        memoryBytes_ -= bytesOf(messages_.front());
        messages_.pop_front();
    }
}

const Message* MessageStore::newestLive() const {
    if (!hasLive_) {
        return nullptr;
    }
    for (const Message& m : messages_) {
        if (m.linked && m.uid == newestLiveUid_) {
            return &m;
        }
    }
    return nullptr;
}

size_t MessageStore::unread() const {
    size_t count = 0;
    for (const Message& m : messages_) {
        count += m.read ? 0 : 1;
    }
    return count;
}
