#pragma once

#include <string>

// The Chat a WhatsApp notification belongs to, and who wrote it.
struct ChatKey {
    std::string chat;
    std::string sender;
};

// Single chat: title = contact, subtitle empty. Group: title = sender, subtitle = group name.
// Invisible direction marks at either end are removed; an empty name becomes "?".
ChatKey chatKeyOf(const std::string& title, const std::string& subtitle);

// A Message is from a group when its sender is not its Chat.
bool isGroup(const std::string& chat, const std::string& sender);
