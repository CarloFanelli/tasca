#include "screens/keyboard_screen.h"

#include <M5Cardputer.h>

#include <algorithm>

#include "ble/ble_manager.h"
#include "ble/hid_keyboard.h"
#include "core/chat_key.h"
#include "core/display_text.h"
#include "core/echo_line.h"
#include "core/keymap.h"
#include "log.h"
#include "screens/chat_screen.h"
#include "siri.h"
#include "ui/draw.h"

using namespace app;
using namespace ui;

KeyboardScreen keyboardScreen;

namespace {

constexpr int kModeTop = 88;
constexpr int kKeyAreaTop = 106;
// Consumer control usage that toggles the iPhone's on-screen keyboard.
constexpr uint16_t kEjectUsage = 0xB8;
// The iPhone tells a keyboard nothing about app or field changes: a pause starts a new echo line.
constexpr uint32_t kEchoIdleMs = 10000;
constexpr uint32_t kBannerMs = 3000;
constexpr uint16_t kBannerBackground = 0x0010;
// Above the mode line: the context Message or the Banner, a title then text lines.
constexpr int kContextTextTop = kStatusHeight + 18;
constexpr int kContextLineHeight = 13;

KeyboardMapper keyboard;
EchoLine echo;
uint32_t lastReportMs = 0;
uint32_t keyPressesAtModeStart = 0;
// A Message that arrived while typing, shown until bannerUntilMs.
bool bannerShown = false;
uint32_t bannerUntilMs = 0;
std::string bannerChat;
std::string bannerText;

// The end of the echo line that fits the screen, left-aligned in the key area.
void drawEcho() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kKeyAreaTop, display.width(), display.height() - kKeyAreaTop, TFT_BLACK);
    display.setTextDatum(middle_left);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setFont(&fonts::efontCN_16);
    display.drawString(lastFitting(echo.text(), display.width() - 2 * kMargin, displayWidth()).c_str(), kMargin,
                       (kKeyAreaTop + display.height()) / 2);
}

void drawMode() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kModeTop, display.width(), kKeyAreaTop - kModeTop, TFT_BLACK);
    display.setTextDatum(middle_center);
    display.setFont(&fonts::Font2);
    display.setTextColor(keyboardReady ? TFT_GREEN : TFT_RED, TFT_BLACK);
    display.drawString(keyboardReady ? "Keyboard mode" : "Keyboard mode - iPhone not ready", display.width() / 2,
                       (kModeTop + kKeyAreaTop) / 2);
}

// A title line, then text wrapped to the lines left above the mode line; the last is cut with "…".
void drawMessageBox(const std::string& title, const std::string& text, uint16_t titleColor, uint16_t background) {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kStatusHeight, display.width(), kModeTop - kStatusHeight, background);
    const int width = display.width() - 2 * kMargin;
    display.setTextDatum(top_left);
    display.setFont(&fonts::efontCN_16);
    display.setTextColor(titleColor, background);
    display.drawString(fitWidth(displayable(title, true), width, displayWidth()).c_str(), kMargin, kStatusHeight + 1);
    display.setFont(&fonts::efontCN_12);
    display.setTextColor(TFT_WHITE, background);
    std::vector<std::string> lines = wrapText(displayable(text, false), width, displayWidth());
    const size_t fit = static_cast<size_t>((kModeTop - kContextTextTop) / kContextLineHeight);
    if (lines.size() > fit) {
        lines[fit - 1] = fitWidth(lines[fit - 1] + " " + lines[fit], width, displayWidth());
        lines.resize(fit);
    }
    for (size_t i = 0; i < lines.size(); i++) {
        display.drawString(lines[i].c_str(), kMargin, kContextTextTop + static_cast<int>(i) * kContextLineHeight);
    }
}

// The iPhone releases every key still held.
void releaseKeys() {
    if (keyboardReady && keyboard.report() != KeyboardReport{}) {
        keyboard.reset({});
        sendKeyboardReport(keyboard.report());
    }
}

}  // namespace

void KeyboardScreen::open(const std::vector<KeyPos>& pressed) {
    keyboard.reset(pressed);
    keyPressesAtModeStart = keyboard.keyPresses();
    logMessage(LogLevel::Info, "HID", "keyboard mode on");
    echo.clear();
    bannerShown = false;
    screens.open(this);
    statusDirty = true;
    screenDirty = true;
}

void KeyboardScreen::close() {
    releaseKeys();
    logMessage(LogLevel::Info, "HID", "keyboard mode off: %u keys sent",
               static_cast<unsigned>(keyboard.keyPresses() - keyPressesAtModeStart));
    echo.clear();
    bannerShown = false;
    screens.close();
    // Back on the Chat screen: what arrived while typing is now on display.
    if (screens.top() == &chatScreen) {
        chatScreen.layout();
    }
    statusDirty = true;
    screenDirty = true;
}

void KeyboardScreen::resume(const std::vector<KeyPos>& pressed) {
    keyboard.reset(pressed);
}

void KeyboardScreen::onCovered() {
    releaseKeys();
}

void KeyboardScreen::updateReady(const std::vector<KeyPos>& pressed) {
    const bool ready = ble::phase() == ble::Phase::Secured && ble::keyboardReportsEnabled();
    if (ready == keyboardReady) {
        return;
    }
    keyboardReady = ready;
    logMessage(LogLevel::Info, "HID", ready ? "keyboard ready" : "keyboard not ready");
    // Keys held while the iPhone was away are not replayed: they count once released and pressed again.
    keyboard.reset(pressed);
    // The iPhone released every key; the echo line itself is kept.
    echo.apply(KeyboardReport{});
    statusDirty = true;
    if (inKeyboardMode() && canDraw()) {
        drawMode();
    }
}

void KeyboardScreen::showBanner(const Message& message) {
    bannerChat = message.chat;
    bannerText = withSender(isGroup(message.chat, message.sender), message.sender, message.text);
    bannerShown = true;
    bannerUntilMs = millis() + kBannerMs;
    contextDirty = true;
}

// The Banner while it lasts; otherwise the last Message of the Chat screen Keyboard mode was entered from.
void KeyboardScreen::drawContext() {
    if (bannerShown) {
        drawMessageBox(bannerChat, bannerText, TFT_YELLOW, kBannerBackground);
        return;
    }
    const auto chat = std::find_if(chats.begin(), chats.end(),
                                   [](const ChatSummary& c) { return nav.inChat() && c.chat == nav.openedChat(); });
    if (chat == chats.end()) {
        auto& display = M5Cardputer.Display;
        display.fillRect(0, kStatusHeight, display.width(), kModeTop - kStatusHeight, TFT_BLACK);
        return;
    }
    drawMessageBox(chat->chat, withSender(chat->group, chat->sender, chat->text), TFT_CYAN, TFT_BLACK);
}

void KeyboardScreen::handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) {
    // Typed text never reaches the serial log: no KEY lines in this mode. A key that turns the screen on
    // still reaches the iPhone.
    for (const KeyEvent& event : events) {
        releaseSiri(event);
        if (event.kind == KeyEventKind::Down) {
            wakeScreen();
        }
        // The ` that closed Siri is not typed: held keys wait for their release.
        if (closeSiriOnBack(event)) {
            keyboard.reset(pressed);
        }
    }
    pairingResetCombo.update({}, millis());
    if (!keyboardReady) {
        return;
    }
    bool echoChanged = false;
    for (const KeyboardReport& report : keyboard.update(pressed)) {
        // The echo follows only what reached the iPhone.
        if (sendKeyboardReport(report)) {
            echoChanged |= echo.apply(report);
            lastReportMs = millis();
        }
    }
    // Only with every key up: a held key (e.g. the iOS accent popup) is not a pause.
    if (!echo.text().empty() && keyboard.report() == KeyboardReport{} && millis() - lastReportMs >= kEchoIdleMs) {
        echo.clear();
        echoChanged = true;
    }
    if (echoChanged && canDraw()) {
        drawEcho();
    }
    if (keyboard.takeEject() && !ble::sendConsumerUsage(kEjectUsage)) {
        logMessage(LogLevel::Warn, "HID", "eject not sent");
    }
    if (keyboard.takeSiri()) {
        requestSiri();
    }
}

void KeyboardScreen::update(uint32_t nowMs) {
    if (bannerShown && static_cast<int32_t>(nowMs - bannerUntilMs) >= 0) {
        bannerShown = false;
        contextDirty = true;
    }
}

void KeyboardScreen::draw() {
    drawContext();
    drawMode();
    drawEcho();
}
