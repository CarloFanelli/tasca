#include "screens/call_screen.h"

#include <M5Cardputer.h>

#include "app.h"
#include "ble/ancs_client.h"
#include "core/display_text.h"
#include "core/incoming_call.h"
#include "log.h"
#include "screens/keyboard_screen.h"
#include "siri.h"
#include "ui/draw.h"

using namespace app;
using namespace ui;

CallScreen callScreen;

namespace {

// A double beep per ring.
constexpr size_t kBeepsPerRing = 2;
constexpr int kTitleTop = kStatusHeight + 12;
constexpr int kCallerTop = kTitleTop + 2 * kLineHeight;
constexpr int kHintTop = kCallerTop + 3 * kLineHeight;

// The keys the call accepts, or the failure of the action sent.
std::string hintOf(const Call& call) {
    if (call.actionFailed) {
        return "Failed \u2014 use the iPhone";
    }
    if (call.positiveAction && call.negativeAction) {
        return "Enter answer   ` decline";
    }
    if (call.positiveAction) {
        return "Enter answer";
    }
    return call.negativeAction ? "` decline" : "";
}

RingTimer ring;

}  // namespace

void CallScreen::sync(const std::vector<KeyPos>& pressed) {
    const bool ringing = ancs::ringingCalls().shown() != nullptr;
    const bool open = screens.isOpen(this);
    if (ringing && !open) {
        // A held Voice Command would stay down on the iPhone during the call.
        cancelSiri();
        wakeScreen();
        screens.open(this);
        ring.start(millis());
        logMessage(LogLevel::Info, "SYS", "call screen opened");
    } else if (!ringing && open) {
        ring.stop();
        screens.remove(this);
        if (inKeyboardMode()) {
            keyboardScreen.resume(pressed);
        }
        // The display timeout starts again from the end of the call.
        wakeScreen();
        logMessage(LogLevel::Info, "SYS", "call screen closed");
    } else {
        return;
    }
    statusDirty = true;
    screenDirty = true;
}

void CallScreen::handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) {
    (void)pressed;
    pairingResetCombo.update({}, millis());
    // Only Enter and ` act, once per call; every other key, with Fn or any modifier too, is ignored.
    for (const KeyEvent& event : events) {
        if (event.kind != KeyEventKind::Down || event.modifiers != 0) {
            continue;
        }
        if (isKey(event.key, kOpenKey)) {
            screenDirty |= ancs::actOnCall(true);
        } else if (isKey(event.key, kBackKey)) {
            screenDirty |= ancs::actOnCall(false);
        }
    }
}

void CallScreen::update(uint32_t nowMs) {
    if (ring.due(nowMs) && settings.sound()) {
        queueBeeps(kBeepsPerRing);
    }
}

void CallScreen::draw() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    const Call* call = ancs::ringingCalls().shown();
    if (call == nullptr) {
        return;
    }
    display.setFont(&fonts::efontCN_16);
    display.setTextDatum(top_center);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.drawString("Incoming call", display.width() / 2, kTitleTop);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    const std::string caller =
        fitWidth(displayable(callerName(*call), true), display.width() - 2 * kMargin, displayWidth());
    display.drawString(caller.c_str(), display.width() / 2, kCallerTop);
    display.setTextColor(call->actionFailed ? TFT_RED : TFT_LIGHTGREY, TFT_BLACK);
    display.drawString(hintOf(*call).c_str(), display.width() / 2, kHintTop);
}
