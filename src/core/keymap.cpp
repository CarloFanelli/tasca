#include "core/keymap.h"

#include <algorithm>

namespace {

constexpr uint8_t kRows = 4;
constexpr uint8_t kCols = 14;
constexpr size_t kMaxKeys = kKeyboardReportSize - 2;
constexpr uint8_t kNone = 0;

constexpr KeyPos kFnKey{2, 0};
constexpr KeyPos kShiftKey{2, 1};

// HID usage of each key position on a US keyboard; modifier keys are 0. Del sends Backspace.
const uint8_t kUsages[kRows][kCols] = {
    {0x35, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x2D, 0x2E, 0x2A},
    {0x2B, 0x14, 0x1A, 0x08, 0x15, 0x17, 0x1C, 0x18, 0x0C, 0x12, 0x13, 0x2F, 0x30, 0x31},
    {kNone, kNone, 0x04, 0x16, 0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x33, 0x34, 0x28},
    {kNone, kNone, kNone, 0x1D, 0x1B, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38, 0x2C},
};

struct FnKey {
    KeyPos key;
    uint8_t usage;
};

// With Fn held only these keys send something.
const FnKey kFnLayer[] = {
    {{2, 11}, 0x52},  // ; Up
    {{3, 10}, 0x50},  // , Left
    {{3, 11}, 0x51},  // . Down
    {{3, 12}, 0x4F},  // / Right
    {{0, 0}, 0x29},   // ` Esc
};

struct AccentKey {
    KeyPos key;
    bool withFn;      // true: Fn+key; false: Ctrl+key
    uint8_t deadKey;  // typed with Option before the vowel
    uint8_t vowel;
};

const AccentKey kAccents[] = {
    {{2, 2}, true, kDeadGrave, 0x04},   // Fn+a  à
    {{1, 3}, true, kDeadGrave, 0x08},   // Fn+e  è
    {{1, 8}, true, kDeadGrave, 0x0C},   // Fn+i  ì
    {{1, 9}, true, kDeadGrave, 0x12},   // Fn+o  ò
    {{1, 7}, true, kDeadGrave, 0x18},   // Fn+u  ù
    {{1, 3}, false, kDeadAcute, 0x08},  // Ctrl+e  é
};

struct ModifierKey {
    KeyPos key;
    uint8_t bit;
};

const ModifierKey kModifiers[] = {
    {{3, 0}, kHidCtrl},
    {{2, 1}, kHidShift},
    {{3, 1}, kHidOption},
    {{3, 2}, kHidCommand},  // Alt
};

bool samePos(KeyPos a, KeyPos b) {
    return a.row == b.row && a.col == b.col;
}

uint8_t usageOf(KeyPos key, bool fn) {
    if (key.row >= kRows || key.col >= kCols) {
        return kNone;
    }
    if (!fn) {
        return kUsages[key.row][key.col];
    }
    for (const FnKey& f : kFnLayer) {
        if (samePos(f.key, key)) {
            return f.usage;
        }
    }
    return kNone;
}

constexpr KeyPos kCtrlKey{3, 0};
constexpr KeyPos kSpaceKey{3, 13};
constexpr KeyPos kEnterKey{2, 13};

// The accent typed by `key` with the keys down, or nullptr. Fn+Ctrl+e matches neither entry.
const AccentKey* accentOf(KeyPos key, bool fn, bool ctrl) {
    for (const AccentKey& a : kAccents) {
        if (samePos(a.key, key) && a.withFn == fn && (fn || ctrl) && !(fn && ctrl && a.vowel == 0x08)) {
            return &a;
        }
    }
    return nullptr;
}

}  // namespace

void KeyboardMapper::reset(const std::vector<KeyPos>& pressed) {
    report_ = {};
    masked_ = pressed;
}

bool KeyboardMapper::takeEject() {
    const bool pending = ejectPending_;
    ejectPending_ = false;
    return pending;
}

bool KeyboardMapper::takeSiri() {
    const bool pending = siriPending_;
    siriPending_ = false;
    return pending;
}

std::vector<KeyboardReport> KeyboardMapper::update(const std::vector<KeyPos>& pressed) {
    const auto released = [&](KeyPos key) { return !containsKey(pressed, key); };
    masked_.erase(std::remove_if(masked_.begin(), masked_.end(), released), masked_.end());
    oncePerPress_.erase(std::remove_if(oncePerPress_.begin(), oncePerPress_.end(), released), oncePerPress_.end());

    const bool fn = containsKey(pressed, kFnKey);
    const bool ctrl = containsKey(pressed, kCtrlKey);
    std::vector<KeyboardReport> reports;
    KeyboardReport next{};
    std::vector<uint8_t> down;
    for (const KeyPos& key : pressed) {
        if (containsKey(masked_, key)) {
            continue;
        }
        for (const ModifierKey& m : kModifiers) {
            if (samePos(m.key, key)) {
                next[0] |= m.bit;
            }
        }
        if (containsKey(oncePerPress_, key)) {
            continue;  // acted on once on its press, never repeated
        }
        if (fn && samePos(key, kSpaceKey)) {
            ejectPending_ = true;
            oncePerPress_.push_back(key);
            keyPresses_++;
            continue;
        }
        if (fn && samePos(key, kEnterKey)) {
            siriPending_ = true;
            oncePerPress_.push_back(key);
            keyPresses_++;
            continue;
        }
        if (const AccentKey* accent = accentOf(key, fn, ctrl)) {
            const uint8_t shift = containsKey(pressed, kShiftKey) ? kHidShift : 0;
            KeyboardReport step{};
            step[0] = kHidOption;
            step[2] = accent->deadKey;
            reports.push_back(step);
            reports.push_back(KeyboardReport{});
            step = {};
            step[0] = shift;
            step[2] = accent->vowel;
            reports.push_back(step);
            step[2] = kNone;
            reports.push_back(step);
            oncePerPress_.push_back(key);
            keyPresses_++;
            continue;
        }
        const uint8_t usage = usageOf(key, fn);
        if (usage != kNone) {
            down.push_back(usage);
        }
    }

    // Keys already in the report keep their slot; new keys take the free ones, in scan order.
    size_t slot = 2;
    for (size_t i = 2; i < kKeyboardReportSize; i++) {
        if (report_[i] != kNone && std::find(down.begin(), down.end(), report_[i]) != down.end()) {
            next[slot++] = report_[i];
        }
    }
    for (uint8_t usage : down) {
        if (slot - 2 == kMaxKeys) {
            break;
        }
        if (std::find(next.begin() + 2, next.begin() + slot, usage) == next.begin() + slot) {
            next[slot++] = usage;
            keyPresses_++;
        }
    }

    if (!reports.empty()) {
        // The sequence released every key on the host: keys still held would type again if
        // pressed back, so they wait for their release.
        report_ = reports.back();
        for (const KeyPos& key : pressed) {
            if (usageOf(key, fn) != kNone && !containsKey(masked_, key)) {
                masked_.push_back(key);
            }
        }
        std::fill(next.begin() + 2, next.end(), kNone);
    }
    if (next != report_) {
        reports.push_back(next);
        report_ = next;
    }
    return reports;
}
