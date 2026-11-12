#include <M5Cardputer.h>
#include <algorithm>
#include <esp_efuse.h>
#include <esp_efuse_table.h>

#include "ble/ancs_client.h"
#include "ble/ble_manager.h"
#include "ble/hid_keyboard.h"
#include "core/chat_view.h"
#include "core/combo_hold.h"
#include "core/display_text.h"
#include "core/echo_line.h"
#include "core/inbox.h"
#include "core/key_events.h"
#include "core/keymap.h"
#include "core/reading_nav.h"
#include "core/screen_timer.h"
#include "core/settings.h"
#include "core/settings_nav.h"
#include "core/utf8.h"
#include "log.h"
#include "settings_store.h"

namespace {

constexpr uint32_t kSerialWaitMs = 1500;
constexpr int kStatusHeight = 18;
constexpr int kModeTop = 88;
constexpr int kKeyAreaTop = 106;
// How long "pairing reset" stays on screen before the board restarts.
constexpr uint32_t kResetNoticeMs = 1500;
// Consumer control usage that toggles the iPhone's on-screen keyboard.
constexpr uint16_t kEjectUsage = 0xB8;
// The iPhone tells a keyboard nothing about app or field changes: a pause starts a new echo line.
constexpr uint32_t kEchoIdleMs = 10000;
constexpr uint32_t kBatteryPollMs = 30000;
constexpr uint32_t kBannerMs = 3000;
constexpr uint16_t kBannerBackground = 0x0010;
// Keyboard mode, above the mode line: the context Message or the Banner, a title then text lines.
constexpr int kContextTextTop = kStatusHeight + 18;
constexpr int kContextLineHeight = 13;
constexpr int kMargin = 4;
// Inbox entry: Chat name line, then preview line.
constexpr int kRowHeight = 30;
constexpr int kPreviewOffset = 16;
constexpr uint16_t kSelectedBackground = 0x2945;
// Settings screen: how long an action's prompt waits for the second Enter.
constexpr uint32_t kConfirmMs = 3000;
// One short beep per live Message, with a pause between beeps so that each one is heard.
constexpr float kBeepHz = 2000;
constexpr uint32_t kBeepMs = 80;
constexpr uint32_t kBeepGapMs = 120;
constexpr KeyPos kUpKey{2, 11};    // ;
constexpr KeyPos kDownKey{3, 11};  // .
constexpr KeyPos kOpenKey{2, 13};  // Enter
constexpr KeyPos kBackKey{0, 0};   // `
constexpr KeyPos kSettingsKey{2, 3};  // s, with Opt
constexpr KeyPos kPrevKey{3, 10};     // ,
constexpr KeyPos kNextKey{3, 12};     // /
// Chat screen: the Chat name, then one Message line per kLineHeight.
constexpr int kLineHeight = 16;
constexpr int kChatTextTop = kStatusHeight + kLineHeight + 2;

bool displayReady = false;
KeyTracker keyTracker;
// Fn + Del held for 3 s: forget the bonded iPhone.
ComboHold pairingResetCombo({{2, 0}, {0, 13}}, 3000);

// Reading mode: keys stay on the Cardputer. Keyboard mode: keys go to the iPhone.
bool keyboardMode = false;
bool keyboardReady = false;
KeyboardMapper keyboard;
EchoLine echo;
uint32_t lastReportMs = 0;
uint32_t keyPressesAtModeStart = 0;

// Reading mode: the Inbox built from the stored Messages and the user's place in it.
std::vector<ChatSummary> chats;
ReadingNav nav;
// The open Chat screen's lines, rebuilt only when its content changes.
std::vector<ChatLine> openChatLines;
// The Settings screen covers the Inbox or Chat screen it was opened from, which stays as it was.
Settings settings;
// The actions listed after the settings.
constexpr const char* kActions[] = {"Reset pairing", "Clear messages"};
constexpr size_t kActionCount = sizeof(kActions) / sizeof(kActions[0]);
SettingsNav settingsNav(kSettingCount, kActionCount, kConfirmMs);
bool settingsOpen = false;
// Battery percent, negative when unknown.
int32_t batteryLevel = -1;
uint32_t lastBatteryPollMs = 0;
// Redraw only what changed, once per loop.
bool statusDirty = true;
bool screenDirty = true;
// Keyboard mode: only the area above the mode line changed.
bool contextDirty = false;

// Keyboard mode: a Message that arrived while typing, shown until bannerUntilMs.
bool bannerShown = false;
uint32_t bannerUntilMs = 0;
std::string bannerChat;
std::string bannerText;
// Messages added live so far: one more means a new arrival.
size_t seenLiveAdded = 0;
size_t pendingBeeps = 0;
uint32_t nextBeepMs = 0;
// While the screen is off nothing is drawn: the dirty flags stay set until it is on again.
ScreenTimer screenTimer;

bool canDraw() {
    return displayReady && !screenTimer.off();
}

// A key, G0 or live Message: the display timeout restarts. True when this turned the screen on.
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

void pollScreenTimeout() {
    if (!screenTimer.expire(millis())) {
        return;
    }
    if (displayReady) {
        M5Cardputer.Display.sleep();
    }
    logMessage(LogLevel::Debug, "SYS", "screen off");
    ble::requestTimings(true);
}

void pollBeeps() {
    if (pendingBeeps == 0 || static_cast<int32_t>(millis() - nextBeepMs) < 0) {
        return;
    }
    pendingBeeps--;
    M5Cardputer.Speaker.tone(kBeepHz, kBeepMs);
    nextBeepMs = millis() + kBeepMs + kBeepGapMs;
}

bool isKey(KeyPos a, KeyPos b) {
    return a.row == b.row && a.col == b.col;
}

// A Message's text as previewed: in groups it starts with its sender.
std::string withSender(bool group, const std::string& sender, const std::string& text) {
    return group ? sender + ": " + text : text;
}

TextWidth displayWidth() {
    return [](const std::string& text) { return M5Cardputer.Display.textWidth(text.c_str()); };
}

// The end of the echo line that fits the screen, left-aligned in the key area.
void drawEcho() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kKeyAreaTop, display.width(), display.height() - kKeyAreaTop, TFT_BLACK);
    display.setTextDatum(middle_left);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setFont(&fonts::efontCN_16);
    const std::string& text = echo.text();
    size_t start = 0;
    while (start < text.size() && display.textWidth(text.c_str() + start) > display.width() - 2 * kMargin) {
        do {
            start++;
        } while (start < text.size() && isUtf8Continuation(text[start]));
    }
    display.drawString(text.c_str() + start, kMargin, (kKeyAreaTop + display.height()) / 2);
}

const char* linkText() {
    switch (ancs::state()) {
        case ancs::State::Active:
            return "ANCS active";
        case ancs::State::Denied:
            return "ANCS denied";
        case ancs::State::Idle:
            break;
    }
    return ble::phaseName(ble::phase());
}

// Mode on the left, link state after it, unread count and battery on the right.
void drawStatus() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, 0, display.width(), kStatusHeight, TFT_BLACK);
    display.drawFastHLine(0, kStatusHeight - 1, display.width(), TFT_DARKGREY);
    display.setFont(&fonts::Font2);
    constexpr int y = (kStatusHeight - 1) / 2;

    const char* mode = keyboardMode ? "Keyboard" : "Reading";
    display.setTextDatum(middle_left);
    display.setTextColor(keyboardMode ? (keyboardReady ? TFT_GREEN : TFT_RED) : TFT_LIGHTGREY, TFT_BLACK);
    display.drawString(mode, 2, y);
    const int linkLeft = 2 + display.textWidth(mode) + 8;

    char right[32];
    char battery[8];
    if (batteryLevel < 0) {
        snprintf(battery, sizeof(battery), "--%%");
    } else {
        snprintf(battery, sizeof(battery), "%d%%", static_cast<int>(batteryLevel));
    }
    snprintf(right, sizeof(right), "%s unread  %s", unreadLabel(ancs::messages().unread()).c_str(), battery);
    display.setTextDatum(middle_right);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString(right, display.width() - 2, y);

    const int linkWidth = display.width() - 2 - display.textWidth(right) - 8 - linkLeft;
    display.setTextDatum(middle_left);
    display.setTextColor(TFT_CYAN, TFT_BLACK);
    display.drawString(fitWidth(linkText(), linkWidth, displayWidth()).c_str(), linkLeft, y);
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

void drawInboxEntry(const ChatSummary& chat, int top, bool selected) {
    auto& display = M5Cardputer.Display;
    const uint16_t background = selected ? kSelectedBackground : TFT_BLACK;
    display.fillRect(0, top, display.width(), kRowHeight, background);
    const int right = display.width() - kMargin;

    display.setFont(&fonts::efontCN_16);
    int nameWidth = right - kMargin;
    if (chat.unread > 0) {
        const std::string count = unreadLabel(chat.unread);
        display.setTextDatum(top_right);
        display.setTextColor(TFT_GREEN, background);
        display.drawString(count.c_str(), right, top);
        nameWidth -= display.textWidth(count.c_str()) + kMargin;
    }
    display.setTextDatum(top_left);
    display.setTextColor(TFT_WHITE, background);
    display.drawString(fitWidth(displayable(chat.chat, true), nameWidth, displayWidth()).c_str(), kMargin, top);

    display.setFont(&fonts::efontCN_12);
    display.setTextColor(TFT_LIGHTGREY, background);
    const std::string preview = withSender(chat.group, chat.sender, chat.text);
    display.drawString(fitWidth(displayable(preview, true), right - kMargin, displayWidth()).c_str(), kMargin,
                       top + kPreviewOffset);
}

void drawInbox() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    if (chats.empty()) {
        display.setFont(&fonts::Font2);
        display.setTextDatum(middle_center);
        display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        display.drawString("No messages yet", display.width() / 2, (kStatusHeight + display.height()) / 2);
        return;
    }
    const size_t rows = static_cast<size_t>((display.height() - kStatusHeight) / kRowHeight);
    const size_t first = nav.firstVisible(rows);
    for (size_t i = first; i < chats.size() && i < first + rows; i++) {
        drawInboxEntry(chats[i], kStatusHeight + static_cast<int>(i - first) * kRowHeight, i == nav.selected());
    }
}

void drawChat() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    display.setFont(&fonts::efontCN_16);
    display.setTextDatum(top_left);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.drawString(
        fitWidth(displayable(nav.openedChat(), true), display.width() - 2 * kMargin, displayWidth()).c_str(),
        kMargin, kStatusHeight);
    display.drawFastHLine(0, kChatTextTop - 1, display.width(), TFT_DARKGREY);
    const size_t visible = static_cast<size_t>((display.height() - kChatTextTop) / kLineHeight);
    for (size_t i = 0; i < visible && nav.chatTop() + i < openChatLines.size(); i++) {
        const ChatLine& line = openChatLines[nav.chatTop() + i];
        display.setTextColor(line.header ? TFT_CYAN : TFT_WHITE, TFT_BLACK);
        display.drawString(line.text.c_str(), kMargin, kChatTextTop + static_cast<int>(i) * kLineHeight);
    }
}

// The title, then one row per setting, its name on the left and its value on the right, then one row
// per action, replaced by its prompt while it waits for confirmation.
void drawSettings() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    display.setFont(&fonts::efontCN_16);
    display.setTextDatum(top_left);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.drawString("Settings", kMargin, kStatusHeight);
    display.drawFastHLine(0, kChatTextTop - 1, display.width(), TFT_DARKGREY);
    for (size_t i = 0; i < kSettingCount; i++) {
        const Setting setting = static_cast<Setting>(i);
        const int top = kChatTextTop + static_cast<int>(i) * kLineHeight;
        const uint16_t background = i == settingsNav.selected() ? kSelectedBackground : TFT_BLACK;
        display.fillRect(0, top, display.width(), kLineHeight, background);
        display.setTextDatum(top_left);
        display.setTextColor(TFT_WHITE, background);
        display.drawString(settingName(setting), kMargin, top);
        display.setTextDatum(top_right);
        display.setTextColor(TFT_GREEN, background);
        display.drawString(settings.label(setting).c_str(), display.width() - kMargin, top);
    }
    for (size_t i = 0; i < kActionCount; i++) {
        const size_t row = kSettingCount + i;
        const int top = kChatTextTop + static_cast<int>(row) * kLineHeight;
        const bool selected = row == settingsNav.selected();
        const uint16_t background = selected ? kSelectedBackground : TFT_BLACK;
        display.fillRect(0, top, display.width(), kLineHeight, background);
        display.setTextDatum(top_left);
        if (selected && settingsNav.confirming()) {
            display.setTextColor(TFT_RED, background);
            display.drawString("Press Enter to confirm", kMargin, top);
        } else {
            display.setTextColor(TFT_ORANGE, background);
            display.drawString(kActions[i], kMargin, top);
        }
    }
}

void applyBrightness() {
    if (displayReady) {
        M5Cardputer.Display.setBrightness(settings.brightnessPercent() * 255 / 100);
    }
}

// Rebuilds the open Chat screen's lines from the store and clamps its scroll to them.
void layoutOpenChat() {
    auto& display = M5Cardputer.Display;
    display.setFont(&fonts::efontCN_16);
    openChatLines = chatLines(ancs::messages().messages(), nav.openedChat(), display.width() - 2 * kMargin,
                              displayWidth());
    size_t newestStart = 0;
    for (size_t i = 0; i < openChatLines.size(); i++) {
        if (openChatLines[i].header) {
            newestStart = i;
        }
    }
    nav.setChatLines(openChatLines.size(), static_cast<size_t>((display.height() - kChatTextTop) / kLineHeight),
                     newestStart);
}

void rebuildInbox() {
    chats = buildInbox(ancs::messages().messages());
    nav.sync(chats);
}

// The Chat screen on display shows its Messages: they are read.
void readOpenChat() {
    if (ancs::markChatRead(nav.openedChat())) {
        rebuildInbox();
        statusDirty = true;
    }
}

void openSelectedChat() {
    if (!nav.openChat()) {
        return;
    }
    readOpenChat();
    layoutOpenChat();
    screenDirty = true;
}

// The stored Messages changed: rebuild the Inbox, and the open Chat screen unless all its Messages are gone.
void onMessagesChanged() {
    rebuildInbox();
    statusDirty = true;
    if (nav.inChat()) {
        const bool chatLeft = std::none_of(chats.begin(), chats.end(),
                                           [](const ChatSummary& c) { return c.chat == nav.openedChat(); });
        if (chatLeft) {
            nav.closeChat();
            openChatLines.clear();
        } else {
            if (!keyboardMode && !settingsOpen) {
                readOpenChat();
            }
            layoutOpenChat();
        }
    }
    if (keyboardMode) {
        contextDirty = true;
    } else if (!settingsOpen) {
        screenDirty = true;
    }
}

// Each live Message beeps and can turn the screen on. One that arrived while typing shows as the Banner;
// in Reading mode the screens show it.
void pollArrivals() {
    const MessageStore& store = ancs::messages();
    if (store.liveAdded() == seenLiveAdded) {
        return;
    }
    if (settings.sound()) {
        pendingBeeps += store.liveAdded() - seenLiveAdded;
    }
    seenLiveAdded = store.liveAdded();
    if (settings.wakeOnMessage()) {
        wakeScreen();
    }
    const Message* newest = store.newestLive();
    if (!keyboardMode || newest == nullptr) {
        return;
    }
    bannerChat = newest->chat;
    bannerText = withSender(isGroup(newest->chat, newest->sender), newest->sender, newest->text);
    bannerShown = true;
    bannerUntilMs = millis() + kBannerMs;
    contextDirty = true;
}

void pollBanner() {
    if (bannerShown && static_cast<int32_t>(millis() - bannerUntilMs) >= 0) {
        bannerShown = false;
        contextDirty = true;
    }
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

// The Banner while it lasts; otherwise the last Message of the Chat screen Keyboard mode was entered from.
void drawContext() {
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

void drawScreen() {
    if (keyboardMode) {
        drawContext();
        drawMode();
        drawEcho();
    } else if (settingsOpen) {
        drawSettings();
    } else if (nav.inChat()) {
        drawChat();
    } else {
        drawInbox();
    }
}

std::vector<KeyPos> readPressedKeys() {
    std::vector<KeyPos> pressed;
    for (const auto& point : M5Cardputer.Keyboard.keyList()) {
        pressed.push_back({static_cast<uint8_t>(point.y), static_cast<uint8_t>(point.x)});
    }
    return pressed;
}

// Keys reach the iPhone only on a secured link whose keyboard report the iPhone has enabled.
void updateKeyboardReady(const std::vector<KeyPos>& pressed) {
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
    if (keyboardMode && canDraw()) {
        drawMode();
    }
}

bool sendKeyboardReport(const KeyboardReport& report) {
    if (!ble::sendKeyboardReport(report)) {
        logMessage(LogLevel::Warn, "HID", "keyboard report not sent");
        return false;
    }
    return true;
}

void setKeyboardMode(bool on, const std::vector<KeyPos>& pressed) {
    if (on) {
        keyboard.reset(pressed);
        keyPressesAtModeStart = keyboard.keyPresses();
        logMessage(LogLevel::Info, "HID", "keyboard mode on");
    } else {
        if (keyboardReady && keyboard.report() != KeyboardReport{}) {
            keyboard.reset({});
            sendKeyboardReport(keyboard.report());
        }
        logMessage(LogLevel::Info, "HID", "keyboard mode off: %u keys sent",
                   static_cast<unsigned>(keyboard.keyPresses() - keyPressesAtModeStart));
    }
    echo.clear();
    keyboardMode = on;
    bannerShown = false;
    // Back on the Chat screen: what arrived while typing is now on display.
    if (!on && nav.inChat() && !settingsOpen) {
        readOpenChat();
        layoutOpenChat();
    }
    statusDirty = true;
    screenDirty = true;
}

void pollKeyboardMode(const std::vector<KeyPos>& pressed) {
    // Typed text never reaches the serial log: no KEY lines in this mode. A key that turns the screen on
    // still reaches the iPhone.
    for (const KeyEvent& event : keyTracker.update(pressed)) {
        if (event.kind == KeyEventKind::Down) {
            wakeScreen();
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
}

void restartAfterPairingReset() {
    if (canDraw()) {
        drawStatus();
    }
    logMessage(LogLevel::Info, "SYS", "restarting after pairing reset");
    Serial.flush();
    delay(kResetNoticeMs);
    ESP.restart();
}

void runAction(size_t action) {
    if (action == 0) {
        // With no bond there is nothing to erase: the restart still happens.
        ble::resetPairing();
        restartAfterPairingReset();
    } else {
        ancs::clearMessages();
        onMessagesChanged();
    }
}

void closeSettings() {
    settingsOpen = false;
    // Back on the Chat screen: what arrived meanwhile is now on display.
    if (nav.inChat()) {
        readOpenChat();
    }
    screenDirty = true;
}

void stepSetting(int direction) {
    const Setting setting = static_cast<Setting>(settingsNav.selected());
    if (!settings.step(setting, direction)) {
        return;
    }
    settings_store::save(settings, setting);
    if (setting == Setting::Brightness) {
        applyBrightness();
    } else if (setting == Setting::DisplayTimeout) {
        screenTimer.setTimeout(settings.displayTimeoutMs());
    }
    screenDirty = true;
}

void pollSettingsKey(KeyPos key) {
    if (isKey(key, kOpenKey)) {
        if (settingsNav.enter(millis())) {
            runAction(settingsNav.selected() - kSettingCount);
        }
        screenDirty |= settingsNav.onAction();
        return;
    }
    // Any other key only cancels a prompt.
    if (settingsNav.cancel()) {
        screenDirty = true;
        return;
    }
    if (isKey(key, kUpKey)) {
        screenDirty |= settingsNav.up();
    } else if (isKey(key, kDownKey)) {
        screenDirty |= settingsNav.down();
    } else if (isKey(key, kPrevKey) && !settingsNav.onAction()) {
        stepSetting(-1);
    } else if (isKey(key, kNextKey) && !settingsNav.onAction()) {
        stepSetting(1);
    } else if (isKey(key, kBackKey)) {
        closeSettings();
    }
}

void pollReadingMode(const std::vector<KeyPos>& pressed) {
    if (pairingResetCombo.update(pressed, millis()) && ble::resetPairing()) {
        restartAfterPairingReset();
    }
    for (const KeyEvent& event : keyTracker.update(pressed)) {
        char text[kKeyTextCapacity];
        formatKeyEvent(text, sizeof(text), event);
        logMessage(LogLevel::Debug, "KEY", "%s", text);
        // A key that turns the screen on does nothing else.
        if (event.kind != KeyEventKind::Down || wakeScreen()) {
            continue;
        }
        // Fn is ignored: Fn+; and Fn+. are the arrow legends of the same keys, Fn+, and Fn+/ likewise.
        if (settingsOpen) {
            pollSettingsKey(event.key);
        } else if (isKey(event.key, kSettingsKey) && (event.modifiers & kModOpt) != 0) {
            settingsOpen = true;
            screenDirty = true;
        } else if (nav.inChat()) {
            if (isKey(event.key, kUpKey)) {
                screenDirty |= nav.scrollUp();
            } else if (isKey(event.key, kDownKey)) {
                screenDirty |= nav.scrollDown();
            } else if (isKey(event.key, kBackKey)) {
                nav.closeChat();
                openChatLines.clear();
                screenDirty = true;
            }
        } else if (isKey(event.key, kUpKey)) {
            screenDirty |= nav.up();
        } else if (isKey(event.key, kDownKey)) {
            screenDirty |= nav.down();
        } else if (isKey(event.key, kOpenKey)) {
            openSelectedChat();
        }
    }
}

void pollBattery() {
    if (lastBatteryPollMs != 0 && millis() - lastBatteryPollMs < kBatteryPollMs) {
        return;
    }
    lastBatteryPollMs = millis();
    const int32_t level = M5Cardputer.Power.getBatteryLevel();
    const int32_t known = level < 0 ? -1 : (level > 100 ? 100 : level);
    if (known != batteryLevel) {
        batteryLevel = known;
        statusDirty = true;
    }
}

void pollG0(const std::vector<KeyPos>& pressed) {
    if (M5Cardputer.BtnA.wasPressed()) {
        logMessage(LogLevel::Debug, "BTN", "G0 down");
        // A press that turns the screen on leaves the mode as it is.
        if (!wakeScreen()) {
            setKeyboardMode(!keyboardMode, pressed);
        }
    }
    if (M5Cardputer.BtnA.wasReleased()) {
        logMessage(LogLevel::Debug, "BTN", "G0 up");
    }
}

// The eFuse records in-package PSRAM: when it is set but the PSRAM did not come up,
// the chip has PSRAM that failed to initialize.
String psramStatus() {
    if (psramFound()) {
        return String(ESP.getPsramSize());
    }
    uint8_t capacity = 0;
    esp_efuse_read_field_blob(ESP_EFUSE_PSRAM_CAP, &capacity, esp_efuse_get_field_size(ESP_EFUSE_PSRAM_CAP));
    return capacity != 0 ? "error" : "none";
}

}  // namespace

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    displayReady = M5.getDisplayCount() > 0;
    if (displayReady) {
        M5Cardputer.Display.fillScreen(TFT_BLACK);
    }

    Serial.begin(115200);
    // Give a just-opened serial monitor time to attach, so the boot report is not lost.
    const uint32_t start = millis();
    while (!Serial && millis() - start < kSerialWaitMs) {
        delay(10);
    }

    logMessage(LogLevel::Info, "SYS", "boot version=%s psram=%s heap=%u flash=%u", TASCA_VERSION,
               psramStatus().c_str(), static_cast<unsigned>(ESP.getFreeHeap()),
               static_cast<unsigned>(ESP.getFlashChipSize()));
    if (!displayReady) {
        logMessage(LogLevel::Error, "SYS", "display init failed");
    }
    settings_store::load(settings);
    applyBrightness();
    screenTimer.setTimeout(settings.displayTimeoutMs());
    screenTimer.activity(millis());

    pollBattery();
    ble::begin(static_cast<uint8_t>(batteryLevel < 0 ? 0 : batteryLevel));
}

void loop() {
    M5Cardputer.update();
    const bool bleChanged = ble::poll();
    const bool ancsChanged = ancs::poll();
    statusDirty |= bleChanged || ancsChanged;
    if (ancsChanged) {
        onMessagesChanged();
        pollArrivals();
    }
    pollBanner();
    if (settingsOpen && settingsNav.expire(millis())) {
        screenDirty = true;
    }
    pollBattery();
    const std::vector<KeyPos> pressed = readPressedKeys();
    updateKeyboardReady(pressed);
    pollG0(pressed);
    if (keyboardMode) {
        pollKeyboardMode(pressed);
    } else {
        pollReadingMode(pressed);
    }
    pollScreenTimeout();
    pollBeeps();
    if (!canDraw()) {
        delay(10);
        return;
    }
    if (statusDirty) {
        drawStatus();
    }
    if (screenDirty) {
        drawScreen();
    } else if (contextDirty && keyboardMode) {
        drawContext();
    }
    statusDirty = false;
    screenDirty = false;
    contextDirty = false;
    delay(10);
}
