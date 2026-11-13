#include "siri.h"

#include <Arduino.h>

#include "app.h"
#include "ble/hid_keyboard.h"
#include "core/keymap.h"
#include "core/siri_press.h"
#include "log.h"

namespace {

// Consumer control usage that asks the iPhone to start Siri (Voice Command).
constexpr uint16_t kSiriUsage = 0xCF;
constexpr uint8_t kEscUsage = 0x29;

// Siri starts on a held command, as on a headset button: a Shift tap wakes the iPhone first, then the
// command stays down until Enter is released.
SiriPress siriPress;
bool siriEnterHeld = false;
SiriCloseWindow siriCloseWindow;

}  // namespace

void requestSiri() {
    if (!app::keyboardReady) {
        logMessage(LogLevel::Warn, "HID", "siri not requested: iPhone not ready");
        return;
    }
    if (!siriPress.start(millis())) {
        return;
    }
    // On a locked iPhone with notifications the first key only lights the screen: Shift spends it.
    KeyboardReport shift{};
    shift[0] = kHidShift;
    if (!app::sendKeyboardReport(shift) || !app::sendKeyboardReport(KeyboardReport{})) {
        siriPress.cancel();
        logMessage(LogLevel::Warn, "HID", "siri request not sent");
        return;
    }
    siriEnterHeld = true;
    siriCloseWindow.open(millis());
    logMessage(LogLevel::Info, "HID", "siri requested");
}

bool closeSiriOnBack(const KeyEvent& event) {
    if (event.kind != KeyEventKind::Down ||
        !siriCloseWindow.keyDown(millis(), app::isKey(event.key, app::kBackKey) && event.modifiers == 0)) {
        return false;
    }
    KeyboardReport esc{};
    esc[2] = kEscUsage;
    if (!app::keyboardReady || !app::sendKeyboardReport(esc) || !app::sendKeyboardReport(KeyboardReport{})) {
        logMessage(LogLevel::Warn, "HID", "siri close not sent");
    } else {
        logMessage(LogLevel::Info, "HID", "siri closed");
    }
    return true;
}

void releaseSiri(const KeyEvent& event) {
    if (event.kind == KeyEventKind::Up && app::isKey(event.key, app::kOpenKey)) {
        siriEnterHeld = false;
    }
}

void pollSiri() {
    if (!siriPress.active()) {
        return;
    }
    if (!app::keyboardReady) {
        siriPress.cancel();
        return;
    }
    switch (siriPress.update(millis(), siriEnterHeld)) {
        case SiriAction::StartHold:
            if (!ble::setConsumerUsage(kSiriUsage)) {
                siriPress.cancel();
                logMessage(LogLevel::Warn, "HID", "siri hold not sent");
            }
            break;
        case SiriAction::EndHold:
            if (!ble::setConsumerUsage(0)) {
                logMessage(LogLevel::Warn, "HID", "siri release not sent");
            }
            break;
        case SiriAction::None:
            break;
    }
}

void cancelSiri() {
    siriEnterHeld = false;
    if (siriPress.cancel() && !ble::setConsumerUsage(0)) {
        logMessage(LogLevel::Warn, "HID", "siri release not sent");
    }
}
