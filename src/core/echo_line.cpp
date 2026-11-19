#include "core/echo_line.h"

#include <algorithm>

#include "core/utf8.h"

namespace {

constexpr uint8_t kEnter = 0x28;
constexpr uint8_t kBackspace = 0x2A;

const char* const kPlaceholder = "?";

struct Legend {
    char plain;
    char shifted;
};

// U.S. keycap legends for usages 0x1E-0x38 (digits and punctuation); 0 where nothing is typed.
const Legend kSymbols[] = {
    {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'}, {'7', '&'},
    {'8', '*'}, {'9', '('}, {'0', ')'}, {0, 0},     {0, 0},     {0, 0},     {0, 0},
    {' ', ' '}, {'-', '_'}, {'=', '+'}, {'[', '{'}, {']', '}'}, {'\\', '|'}, {0, 0},
    {';', ':'}, {'\'', '"'}, {'`', '~'}, {',', '<'}, {'.', '>'}, {'/', '?'},
};
constexpr uint8_t kFirstSymbol = 0x1E;
constexpr uint8_t kLastSymbol = 0x38;

struct AccentedVowel {
    uint8_t vowel;
    const char* grave[2];  // lower, upper
    const char* acute[2];
};

const AccentedVowel kAccented[] = {
    {0x04, {"à", "À"}, {"á", "Á"}},
    {0x08, {"è", "È"}, {"é", "É"}},
    {0x0C, {"ì", "Ì"}, {"í", "Í"}},
    {0x12, {"ò", "Ò"}, {"ó", "Ó"}},
    {0x18, {"ù", "Ù"}, {"ú", "Ú"}},
};

// The character a U.S. layout types for usage, written to out; false when it types none.
bool legendOf(uint8_t usage, bool shift, char out[2]) {
    out[1] = '\0';
    if (usage >= 0x04 && usage <= 0x1D) {
        out[0] = static_cast<char>((shift ? 'A' : 'a') + (usage - 0x04));
        return true;
    }
    if (usage >= kFirstSymbol && usage <= kLastSymbol) {
        const Legend& legend = kSymbols[usage - kFirstSymbol];
        out[0] = shift ? legend.shifted : legend.plain;
        return out[0] != 0;
    }
    return false;
}

}  // namespace

bool EchoLine::apply(const KeyboardReport& report) {
    const std::string before = text_;
    const uint8_t modifiers = report[0];
    for (size_t i = 2; i < kKeyboardReportSize; i++) {
        const uint8_t usage = report[i];
        if (usage == 0 || std::find(previous_.begin() + 2, previous_.end(), usage) != previous_.end()) {
            continue;  // no key, or held since the previous report
        }
        if (modifiers & (kHidCtrl | kHidCommand)) {
            continue;  // shortcuts type nothing
        }
        if (modifiers & kHidOption) {
            if (usage == kDeadGrave || usage == kDeadAcute) {
                deadKey_ = usage;
            } else {
                append(kPlaceholder);
            }
            continue;
        }
        type(usage, (modifiers & kHidShift) != 0);
    }
    previous_ = report;
    return text_ != before;
}

void EchoLine::type(uint8_t usage, bool shift) {
    if (usage == kBackspace) {
        // The iPhone drops a pending dead key first, like the character before it.
        if (deadKey_ != 0) {
            deadKey_ = 0;
        } else {
            removeLastCharacter();
        }
        return;
    }
    if (usage == kEnter) {
        text_.clear();
        deadKey_ = 0;
        return;
    }
    char legend[2];
    if (!legendOf(usage, shift, legend)) {
        return;  // arrows, Esc, Tab and other keys without a character
    }
    if (deadKey_ != 0) {
        const uint8_t deadKey = deadKey_;
        deadKey_ = 0;
        for (const AccentedVowel& a : kAccented) {
            if (a.vowel == usage) {
                append((deadKey == kDeadGrave ? a.grave : a.acute)[shift ? 1 : 0]);
                return;
            }
        }
        append(kPlaceholder);
    }
    append(legend);
}

void EchoLine::append(const char* utf8) {
    text_ += utf8;
    if (text_.size() <= kEchoMaxBytes) {
        return;
    }
    size_t cut = text_.size() - kEchoMaxBytes;
    while (cut < text_.size() && isUtf8Continuation(text_[cut])) {
        cut++;
    }
    text_.erase(0, cut);
}

void EchoLine::removeLastCharacter() {
    while (!text_.empty()) {
        const char last = text_.back();
        text_.pop_back();
        if (!isUtf8Continuation(last)) {
            return;
        }
    }
}

void EchoLine::clear(const std::string& text) {
    previous_ = {};
    deadKey_ = 0;
    text_ = text;
}
