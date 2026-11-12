#pragma once

#include <stddef.h>
#include <stdint.h>

#include <deque>
#include <string>

#include "core/chat_key.h"

// Limits that bound the store's memory: oldest Messages are dropped beyond either.
constexpr size_t kMaxMessages = 200;
// Each Message with its Chat, sender, text and date.
constexpr size_t kMaxStoreBytes = 56 * 1024;

// One WhatsApp notification kept on the Cardputer.
struct Message {
    std::string chat;
    std::string sender;
    std::string text;
    std::string date;  // yyyyMMdd'T'HHmmSS, the iPhone's local time
    bool read = false;
    // The iPhone notification it came from. UIDs restart at every connection: valid only while linked.
    uint32_t uid = 0;
    bool linked = false;
};

// The Messages kept in RAM, oldest first.
class MessageStore {
public:
    enum class Result { Added, Updated, Linked };

    // Takes the attributes of notification uid:
    // - Updated: a Message linked to uid gets the new content, read state unchanged;
    // - Linked: a PreExisting copy of an unlinked Message (same date, Chat, sender, text) is
    //   linked to uid instead of stored again;
    // - Added: otherwise an unread Message is appended.
    // Then drops the oldest Messages while a limit is exceeded.
    Result receive(uint32_t uid, bool preExisting, const ChatKey& key, const std::string& text,
                   const std::string& date);

    // Marks the Message linked to uid read; false when there is none or it was already read.
    bool markRead(uint32_t uid);

    // Forgets every UID: called when the connection ends.
    void unlinkAll();

    const std::deque<Message>& messages() const { return messages_; }
    size_t size() const { return messages_.size(); }
    size_t unread() const;
    size_t memoryBytes() const { return memoryBytes_; }

private:
    Message* linkedTo(uint32_t uid);
    void dropOverLimits();

    std::deque<Message> messages_;
    size_t memoryBytes_ = 0;
};
