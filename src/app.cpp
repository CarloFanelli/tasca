#include "app.h"

#include <M5Cardputer.h>

#include <algorithm>

#include "ble/ancs_client.h"
#include "ble/ble_manager.h"
#include "ble/hid_keyboard.h"
#include "log.h"
#include "screens/chat_screen.h"
#include "screens/inbox_screen.h"
#include "screens/keyboard_screen.h"
#include "screens/settings_screen.h"
#include "ui/status_bar.h"

namespace app {

namespace {

// How long the clear prompt waits for the second Del.
constexpr uint32_t kClearConfirmMs = 3000;
// How long "pairing reset" stays on screen before the board restarts.
constexpr uint32_t kResetNoticeMs = 1500;
// One short beep, with a pause between beeps so that each one is heard.
constexpr float kBeepHz = 2000;
constexpr uint32_t kBeepMs = 80;
constexpr uint32_t kBeepGapMs = 120;
size_t pendingBeeps = 0;
uint32_t nextBeepMs = 0;

}  // namespace

bool displayReady = false;
bool keyboardReady = false;
ComboHold pairingResetCombo({kFnKey, kDelKey}, 3000);
ClearPrompt clearPrompt(kClearConfirmMs);
std::vector<ChatSummary> chats;
ReadingNav nav;
Settings settings;
int32_t batteryLevel = -1;
bool statusDirty = true;
bool screenDirty = true;
bool contextDirty = false;
ScreenTimer screenTimer;
ScreenStack screens(&inboxScreen);

bool isKey(KeyPos a, KeyPos b) {
    return a.row == b.row && a.col == b.col;
}

bool canDraw() {
    return displayReady && !screenTimer.off();
}

bool wakeScreen() {
    if (!screenTimer.activity(millis())) {
        return false;
    }
    // Before any key is sent: with the slow timings a key would wait up to a slow interval.
    ble::requestTimings(false);
    if (displayReady) {
        M5Cardputer.Display.wakeup();
    }
    logMessage(LogLevel::Debug, "SYS", "screen on");
    statusDirty = true;
    screenDirty = true;
    return true;
}

void applyBrightness() {
    if (displayReady) {
        M5Cardputer.Display.setBrightness(settings.brightnessPercent() * 255 / 100);
    }
}

void rebuildInbox() {
    chats = buildInbox(ancs::messages().messages());
    nav.sync(chats);
}

void onMessagesChanged() {
    rebuildInbox();
    statusDirty = true;
    if (nav.inChat()) {
        const bool chatLeft = std::none_of(chats.begin(), chats.end(),
                                           [](const ChatSummary& c) { return c.chat == nav.openedChat(); });
        if (chatLeft) {
            chatScreen.close();
        } else {
            if (screens.top() == &chatScreen) {
                chatScreen.markRead();
            }
            chatScreen.layout();
        }
    }
    if (inKeyboardMode()) {
        contextDirty = true;
    } else if (screens.top() != &settingsScreen) {
        screenDirty = true;
    }
}

bool sendKeyboardReport(const KeyboardReport& report) {
    if (!ble::sendKeyboardReport(report)) {
        logMessage(LogLevel::Warn, "HID", "keyboard report not sent");
        return false;
    }
    return true;
}

void queueBeeps(size_t count) {
    pendingBeeps += count;
}

void pollBeeps() {
    if (pendingBeeps == 0 || static_cast<int32_t>(millis() - nextBeepMs) < 0) {
        return;
    }
    pendingBeeps--;
    M5Cardputer.Speaker.tone(kBeepHz, kBeepMs);
    nextBeepMs = millis() + kBeepMs + kBeepGapMs;
}

void delOnChat(const std::string& chat) {
    // A third Del while the Chat is being cleared does nothing.
    if (ancs::clearing()) {
        return;
    }
    if (clearPrompt.del(chat, millis())) {
        ancs::clearChat(chat);
    }
    screenDirty = true;
}

void restartAfterPairingReset() {
    if (canDraw()) {
        ui::drawStatus();
    }
    logMessage(LogLevel::Info, "SYS", "restarting after pairing reset");
    Serial.flush();
    delay(kResetNoticeMs);
    ESP.restart();
}

}  // namespace app
