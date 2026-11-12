#pragma once

#include <stddef.h>
#include <stdint.h>

#include <array>
#include <vector>

#include "core/key_events.h"

// HID keyboard input report: modifier bits, a reserved byte, up to six key usages.
constexpr size_t kKeyboardReportSize = 8;
using KeyboardReport = std::array<uint8_t, kKeyboardReportSize>;

// Modifier bits of the report's first byte (left-hand keys).
constexpr uint8_t kHidCtrl = 0x01;
constexpr uint8_t kHidShift = 0x02;
constexpr uint8_t kHidOption = 0x04;
constexpr uint8_t kHidCommand = 0x08;

// Usages typed with Option as dead keys on a U.S. layout: grave (Option+`) and acute (Option+e).
constexpr uint8_t kDeadGrave = 0x35;
constexpr uint8_t kDeadAcute = 0x08;

// Turns the Cardputer keys currently down into HID keyboard reports, by key position (US
// usages): the host's keyboard layout decides the characters. Accented letters (Fn+vowel,
// Ctrl+e) become Option dead-key sequences, as typed on a U.S. layout. Fn+Space asks for the
// consumer Eject usage, which toggles the iPhone's on-screen keyboard; Fn+Enter asks for Siri.
class KeyboardMapper {
public:
    // Starts from an empty report; keys down now are ignored until released.
    void reset(const std::vector<KeyPos>& pressed);

    // Recomputes the report for the keys down and returns the reports to send, in order: none
    // when nothing changed, one per change, a short sequence when an accented letter is typed.
    std::vector<KeyboardReport> update(const std::vector<KeyPos>& pressed);

    const KeyboardReport& report() const { return report_; }

    // True once per Fn+Space press since the last call.
    bool takeEject();
    // True once per Fn+Enter press since the last call.
    bool takeSiri();

    // Keys sent since construction (letters, accented letters, Eject, Siri), modifiers excluded.
    uint32_t keyPresses() const { return keyPresses_; }

private:
    KeyboardReport report_{};
    std::vector<KeyPos> masked_;
    std::vector<KeyPos> oncePerPress_;  // accent, Eject and Siri keys already acted on, until released
    bool ejectPending_ = false;
    bool siriPending_ = false;
    uint32_t keyPresses_ = 0;
};
