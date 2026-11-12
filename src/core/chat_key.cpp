#include "core/chat_key.h"

namespace {

constexpr size_t kMarkSize = 3;

// U+200E, U+200F and U+202A-U+202E, all three bytes long in UTF-8.
bool isDirectionMark(const std::string& text, size_t at) {
    if (at + kMarkSize > text.size() || static_cast<unsigned char>(text[at]) != 0xE2 ||
        static_cast<unsigned char>(text[at + 1]) != 0x80) {
        return false;
    }
    const unsigned char last = static_cast<unsigned char>(text[at + 2]);
    return last == 0x8E || last == 0x8F || (last >= 0xAA && last <= 0xAE);
}

std::string trimmed(const std::string& name) {
    size_t begin = 0;
    size_t end = name.size();
    while (isDirectionMark(name, begin)) {
        begin += kMarkSize;
    }
    while (end >= begin + kMarkSize && isDirectionMark(name, end - kMarkSize)) {
        end -= kMarkSize;
    }
    return begin < end ? name.substr(begin, end - begin) : std::string();
}

std::string orPlaceholder(const std::string& name) {
    return name.empty() ? "?" : name;
}

}  // namespace

bool isGroup(const std::string& chat, const std::string& sender) {
    return sender != chat;
}

ChatKey chatKeyOf(const std::string& title, const std::string& subtitle) {
    const std::string sender = orPlaceholder(trimmed(title));
    const std::string group = trimmed(subtitle);
    return {group.empty() ? sender : group, sender};
}
