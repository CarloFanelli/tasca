#pragma once

#include <stddef.h>

#include <string>
#include <vector>

#include "core/echo_line.h"
#include "core/key_events.h"
#include "core/keymap.h"

// Text typed on the Cardputer itself, as Keyboard mode would type it on the iPhone: keycap legends, Fn+vowel and
// Ctrl+e accents, Del deleting. Keys past maxBytes or maxChars characters type nothing.
class TextEntry {
public:
    // reserved: keys the screen acts on (Enter, `), which type nothing.
    TextEntry(size_t maxBytes, size_t maxChars, std::vector<KeyPos> reserved)
        : maxBytes_(maxBytes), maxChars_(maxChars), reserved_(std::move(reserved)) {}

    // Starts from text, empty by default; keys down now type nothing until released.
    void start(const std::vector<KeyPos>& pressed, const std::string& text = "");
    // The keys down this loop; true when the text changed.
    bool update(const std::vector<KeyPos>& pressed);
    const std::string& text() const { return echo_.text(); }

private:
    size_t maxBytes_;
    size_t maxChars_;
    std::vector<KeyPos> reserved_;
    KeyboardMapper mapper_;
    EchoLine echo_;
};
