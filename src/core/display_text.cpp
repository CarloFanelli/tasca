#include "core/display_text.h"

#include "core/utf8.h"

namespace {

constexpr uint32_t kInvalid = 0xFFFFFFFF;

// Decodes the character at text[i]; advances i past it. kInvalid for a malformed sequence.
uint32_t decode(const std::string& text, size_t& i) {
    const unsigned char lead = static_cast<unsigned char>(text[i++]);
    size_t length = 0;
    uint32_t cp = 0;
    if (lead < 0x80) {
        return lead;
    } else if ((lead & 0xE0) == 0xC0) {
        length = 1;
        cp = lead & 0x1F;
    } else if ((lead & 0xF0) == 0xE0) {
        length = 2;
        cp = lead & 0x0F;
    } else if ((lead & 0xF8) == 0xF0) {
        length = 3;
        cp = lead & 0x07;
    } else {
        return kInvalid;
    }
    for (size_t n = 0; n < length; n++) {
        if (i >= text.size() || !isUtf8Continuation(text[i])) {
            return kInvalid;
        }
        cp = (cp << 6) | (static_cast<unsigned char>(text[i++]) & 0x3F);
    }
    return cp;
}

void appendUtf8(std::string& out, uint32_t cp) {
    if (cp < 0x80) {
        out += static_cast<char>(cp);
    } else {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

// Characters drawn as nothing: direction marks, zero-width spaces and joiners, variation selectors.
bool isInvisible(uint32_t cp) {
    return (cp >= 0x200B && cp <= 0x200F) || (cp >= 0x202A && cp <= 0x202E) || (cp >= 0x2060 && cp <= 0x2064) ||
           cp == 0xFEFF || (cp >= 0xFE00 && cp <= 0xFE0F) || (cp >= 0xE0000 && cp <= 0xE007F);
}

// Characters that extend the emoji before them: skin tones and the keycap mark.
bool isEmojiModifier(uint32_t cp) {
    return (cp >= 0x1F3FB && cp <= 0x1F3FF) || cp == 0x20E3;
}

bool isRegionalIndicator(uint32_t cp) {
    return cp >= 0x1F1E6 && cp <= 0x1F1FF;
}

}  // namespace

std::string displayable(const std::string& utf8, bool flatten) {
    std::string out;
    bool joinNext = false;       // after a zero-width joiner: the next character is part of the same emoji
    bool openFlag = false;       // a regional indicator waiting for its pair
    size_t i = 0;
    while (i < utf8.size()) {
        const uint32_t cp = decode(utf8, i);
        if (cp == 0x200D) {
            joinNext = true;
            continue;
        }
        const bool joined = joinNext;
        joinNext = false;
        if (cp == kInvalid || isInvisible(cp) || isEmojiModifier(cp)) {
            continue;
        }
        if (isRegionalIndicator(cp)) {
            if (openFlag) {
                openFlag = false;
                continue;
            }
            openFlag = true;
        } else {
            openFlag = false;
        }
        if (joined && cp > 0xFF) {
            continue;
        }
        if (flatten && (cp == '\n' || cp == '\r' || cp == '\t')) {
            out += ' ';
        } else if (cp > 0xFF) {
            out += kMissingGlyph;
        } else {
            appendUtf8(out, cp);
        }
    }
    return out;
}

namespace {

// Appends word to lines, the last of which is being filled; breaks it inside when it fits no line.
void appendWord(std::vector<std::string>& lines, const std::string& word, int maxWidth, const TextWidth& width) {
    std::string& line = lines.back();
    const std::string joined = line.empty() ? word : line + " " + word;
    if (width(joined) <= maxWidth) {
        line = joined;
        return;
    }
    std::string rest = word;
    if (!line.empty()) {
        lines.emplace_back();
    }
    while (!rest.empty()) {
        std::string& current = lines.back();
        size_t end = 0;
        while (end < rest.size()) {
            size_t next = end + 1;
            while (next < rest.size() && isUtf8Continuation(rest[next])) {
                next++;
            }
            if (width(current + rest.substr(0, next)) > maxWidth && end > 0) {
                break;
            }
            end = next;
        }
        current += rest.substr(0, end);
        rest.erase(0, end);
        if (!rest.empty()) {
            lines.emplace_back();
        }
    }
}

}  // namespace

std::vector<std::string> wrapText(const std::string& text, int maxWidth, const TextWidth& width) {
    std::vector<std::string> lines(1);
    std::string word;
    for (char c : text) {
        if (c == ' ' || c == '\n') {
            if (!word.empty()) {
                appendWord(lines, word, maxWidth, width);
                word.clear();
            }
            if (c == '\n') {
                lines.emplace_back();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        appendWord(lines, word, maxWidth, width);
    }
    return lines;
}

std::string fitWidth(const std::string& text, int maxWidth, const TextWidth& width) {
    if (width(text) <= maxWidth) {
        return text;
    }
    std::string prefix = text;
    while (!prefix.empty()) {
        // Remove the whole last character: its continuation bytes, then its lead byte.
        char removed;
        do {
            removed = prefix.back();
            prefix.pop_back();
        } while (!prefix.empty() && isUtf8Continuation(removed));
        const std::string candidate = prefix + kEllipsis;
        if (width(candidate) <= maxWidth) {
            return candidate;
        }
    }
    return width(kEllipsis) <= maxWidth ? kEllipsis : "";
}
