#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "core/keymap.h"

// Longest echo kept, in bytes: the oldest characters are dropped beyond it.
constexpr size_t kEchoMaxBytes = 256;

// The line being typed on the iPhone, rebuilt from the keyboard reports sent to it as a U.S.
// layout would type them: keycap legends, Shift characters, and the Option dead keys for grave
// and acute accents followed by a vowel. Other Option characters show as a placeholder.
class EchoLine {
public:
    // Applies the next report sent; true when the text changed.
    bool apply(const KeyboardReport& report);

    // Empties the line and forgets keys and dead keys.
    void clear();

    // UTF-8 text typed since the last Enter.
    const std::string& text() const { return text_; }

private:
    void type(uint8_t usage, bool shift);
    void append(const char* utf8);
    void removeLastCharacter();

    KeyboardReport previous_{};
    uint8_t deadKey_ = 0;
    std::string text_;
};
