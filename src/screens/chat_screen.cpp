#include "screens/chat_screen.h"

#include <M5Cardputer.h>

#include "app.h"
#include "ble/ancs_client.h"
#include "screens/settings_screen.h"
#include "ui/draw.h"

using namespace ui;

ChatScreen chatScreen;

void ChatScreen::open() {
    if (!app::nav.openChat()) {
        return;
    }
    app::screens.open(this);
    layout();
    app::screenDirty = true;
}

void ChatScreen::close() {
    app::nav.closeChat();
    lines_.clear();
    app::screens.remove(this);
}

void ChatScreen::layout() {
    auto& display = M5Cardputer.Display;
    display.setFont(&fonts::efontCN_16);
    lines_ = chatLines(ancs::messages().messages(), app::nav.openedChat(), display.width() - 2 * kMargin,
                       displayWidth());
    size_t newestStart = 0;
    for (size_t i = 0; i < lines_.size(); i++) {
        if (lines_[i].header) {
            newestStart = i;
        }
    }
    app::nav.setChatLines(lines_.size(), static_cast<size_t>((display.height() - kChatTextTop) / kLineHeight),
                          newestStart);
}

void ChatScreen::markRead() {
    if (ancs::markChatRead(app::nav.openedChat())) {
        app::rebuildInbox();
        app::statusDirty = true;
    }
}

void ChatScreen::draw() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    display.setFont(&fonts::efontCN_16);
    display.setTextDatum(top_left);
    // The clear prompt or notice takes the place of the Chat's name.
    if (const char* clearText = clearPromptText(app::nav.openedChat())) {
        display.setTextColor(TFT_RED, TFT_BLACK);
        display.drawString(clearText, kMargin, kStatusHeight);
    } else {
        display.setTextColor(TFT_YELLOW, TFT_BLACK);
        display.drawString(
            fitWidth(displayable(app::nav.openedChat(), true), display.width() - 2 * kMargin, displayWidth()).c_str(),
            kMargin, kStatusHeight);
    }
    display.drawFastHLine(0, kChatTextTop - 1, display.width(), TFT_DARKGREY);
    const size_t visible = static_cast<size_t>((display.height() - kChatTextTop) / kLineHeight);
    for (size_t i = 0; i < visible && app::nav.chatTop() + i < lines_.size(); i++) {
        const ChatLine& line = lines_[app::nav.chatTop() + i];
        display.setTextColor(line.header ? TFT_CYAN : TFT_WHITE, TFT_BLACK);
        display.drawString(line.text.c_str(), kMargin, kChatTextTop + static_cast<int>(i) * kLineHeight);
    }
}

void ChatScreen::onKeyDown(const KeyEvent& event) {
    if (app::isKey(event.key, app::kSettingsKey) && (event.modifiers & kModOpt) != 0) {
        settingsScreen.open();
    } else if (app::isKey(event.key, app::kUpKey)) {
        app::screenDirty |= app::nav.scrollUp();
    } else if (app::isKey(event.key, app::kDownKey)) {
        app::screenDirty |= app::nav.scrollDown();
    } else if (app::isKey(event.key, app::kBackKey)) {
        close();
        app::screenDirty = true;
    } else if (app::isKey(event.key, app::kDelKey) && event.modifiers == 0) {
        app::delOnChat(app::nav.openedChat());
    }
}
