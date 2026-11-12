#include "core/key_events.h"

#include <stdio.h>
#include <string.h>

namespace {

constexpr uint8_t kRows = 4;
constexpr uint8_t kCols = 14;

// Unshifted legends of the US layout; keys without a glyph are named.
const char* const kKeyNames[kRows][kCols] = {
    {"`", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Del"},
    {"Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\"},
    {"Fn", "Shift", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "Enter"},
    {"Ctrl", "Opt", "Alt", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "Space"},
};

struct ModifierKey {
    KeyPos key;
    uint8_t bit;
    const char* name;
};

// Listed in the order modifiers are printed.
const ModifierKey kModifierKeys[] = {
    {{3, 0}, kModCtrl, "Ctrl"},
    {{3, 1}, kModOpt, "Opt"},
    {{3, 2}, kModAlt, "Alt"},
    {{2, 1}, kModShift, "Shift"},
    {{2, 0}, kModFn, "Fn"},
};

bool samePos(KeyPos a, KeyPos b) {
    return a.row == b.row && a.col == b.col;
}

const char* keyName(KeyPos key) {
    if (key.row >= kRows || key.col >= kCols) {
        return "?";
    }
    return kKeyNames[key.row][key.col];
}

uint8_t modifierBit(KeyPos key) {
    for (const ModifierKey& m : kModifierKeys) {
        if (samePos(m.key, key)) {
            return m.bit;
        }
    }
    return 0;
}

uint8_t modifiersOf(const std::vector<KeyPos>& keys) {
    uint8_t mask = 0;
    for (const KeyPos& k : keys) {
        mask |= modifierBit(k);
    }
    return mask;
}

// Appends "Ctrl+Shift" style text for mask at offset; returns the new offset.
size_t appendModifiers(char* out, size_t outSize, size_t offset, uint8_t mask) {
    bool first = true;
    for (const ModifierKey& m : kModifierKeys) {
        if (!(mask & m.bit)) {
            continue;
        }
        int written = snprintf(out + offset, outSize - offset, "%s%s", first ? "" : "+", m.name);
        if (written < 0 || static_cast<size_t>(written) >= outSize - offset) {
            return outSize - 1;
        }
        offset += static_cast<size_t>(written);
        first = false;
    }
    return offset;
}

}  // namespace

bool containsKey(const std::vector<KeyPos>& keys, KeyPos key) {
    for (const KeyPos& k : keys) {
        if (samePos(k, key)) {
            return true;
        }
    }
    return false;
}

void formatKeyEvent(char* out, size_t outSize, const KeyEvent& event) {
    if (outSize == 0) {
        return;
    }
    const char* name = keyName(event.key);
    const bool printable = strlen(name) == 1;
    int written = snprintf(out, outSize, printable ? "%s '%s' mods=" : "%s %s mods=",
                           event.kind == KeyEventKind::Down ? "down" : "up", name);
    if (written < 0 || static_cast<size_t>(written) >= outSize) {
        return;
    }
    size_t offset = static_cast<size_t>(written);
    if (event.modifiers == 0) {
        snprintf(out + offset, outSize - offset, "none");
    } else {
        appendModifiers(out, outSize, offset, event.modifiers);
    }
}

std::vector<KeyEvent> KeyTracker::update(const std::vector<KeyPos>& pressed) {
    std::vector<KeyEvent> events;
    const uint8_t held = modifiersOf(pressed);
    for (const KeyPos& key : pressed_) {
        if (!containsKey(pressed, key)) {
            events.push_back({KeyEventKind::Up, key, static_cast<uint8_t>(held & ~modifierBit(key))});
        }
    }
    for (const KeyPos& key : pressed) {
        if (!containsKey(pressed_, key)) {
            events.push_back({KeyEventKind::Down, key, static_cast<uint8_t>(held & ~modifierBit(key))});
        }
    }
    pressed_ = pressed;
    return events;
}
