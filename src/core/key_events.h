#pragma once

#include <stddef.h>
#include <stdint.h>

#include <vector>

// A physical key of the Cardputer keyboard: row 0-3 from the top, column 0-13 from the left.
struct KeyPos {
    uint8_t row;
    uint8_t col;
};

bool containsKey(const std::vector<KeyPos>& keys, KeyPos key);

constexpr uint8_t kModCtrl = 1 << 0;
constexpr uint8_t kModOpt = 1 << 1;
constexpr uint8_t kModAlt = 1 << 2;
constexpr uint8_t kModShift = 1 << 3;
constexpr uint8_t kModFn = 1 << 4;

constexpr size_t kKeyTextCapacity = 64;

enum class KeyEventKind { Down, Up };

// modifiers: the modifiers held at the time of the event, the event's own key excluded.
struct KeyEvent {
    KeyEventKind kind;
    KeyPos key;
    uint8_t modifiers;
};

// "Ctrl+Shift+a", "Fn+;", "Enter": modifiers in the fixed order Ctrl, Opt, Alt, Shift, Fn.
void formatKeyLabel(char* out, size_t outSize, KeyPos key, uint8_t modifiers);

// "down ';' mods=Fn", "up Enter mods=none": printable keys quoted, the others by name.
void formatKeyEvent(char* out, size_t outSize, const KeyEvent& event);

// Turns successive snapshots of the pressed keys into press and release events.
class KeyTracker {
public:
    // Releases are reported before presses, each in snapshot order.
    std::vector<KeyEvent> update(const std::vector<KeyPos>& pressed);

    bool hasLastPressed() const { return hasLastPressed_; }
    const KeyEvent& lastPressed() const { return lastPressed_; }

private:
    std::vector<KeyPos> pressed_;
    KeyEvent lastPressed_{KeyEventKind::Down, {0, 0}, 0};
    bool hasLastPressed_ = false;
};
