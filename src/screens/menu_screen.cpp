#include "screens/menu_screen.h"

#include <M5Cardputer.h>

#include "app.h"
#include "ble/ams_client.h"
#include "ble/ancs_client.h"
#include "core/inbox.h"
#include "core/settings_nav.h"
#include "screens/chat_screen.h"
#include "screens/find_screen.h"
#include "screens/keyboard_screen.h"
#include "screens/music_screen.h"
#include "screens/settings_screen.h"
#include "ui/draw.h"

using namespace ui;

MenuScreen menuScreen;

namespace {

enum Item : size_t { kMusic, kFindIphone, kWhatsApp, kSettings, kItemCount };
constexpr const char* kItemNames[kItemCount] = {"Music", "Find iPhone", "WhatsApp", "Settings"};
// Rows only, no actions: no prompt to confirm.
SettingsNav menuNav(kItemCount, 0, 0);

// Every screen above the Inbox closes, Keyboard mode included: an item opens in Reading mode, from its start.
void closeToInbox() {
    app::screens.remove(&menuScreen);
    if (inKeyboardMode()) {
        keyboardScreen.close();
    }
    app::screens.remove(&settingsScreen);
    app::screens.remove(&musicScreen);
    app::screens.remove(&findScreen);
    if (app::screens.isOpen(&chatScreen)) {
        chatScreen.close();
    }
}

std::string itemValue(size_t item) {
    if (item == kWhatsApp) {
        return unreadLabel(ancs::messages().unread()) + " unread";
    }
    if (item == kMusic) {
        const std::string& title = ams::nowPlaying().title();
        return title.empty() ? "Nothing playing" : displayable(title, true);
    }
    return "";
}

}  // namespace

void MenuScreen::open() {
    menuNav.reset();
    app::screens.open(this);
    app::screenDirty = true;
}

void MenuScreen::close(const std::vector<KeyPos>& pressed) {
    app::screens.remove(this);
    if (inKeyboardMode()) {
        keyboardScreen.resume(pressed);
    }
    app::screenDirty = true;
}

void MenuScreen::handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) {
    pressed_ = pressed;
    ReadingScreen::handleKeys(pressed, events);
}

// The title, then one row per item, its name on the left and what it holds on the right.
void MenuScreen::draw() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    display.setFont(&fonts::efontCN_16);
    display.setTextDatum(top_left);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.drawString("Menu", kMargin, kStatusHeight);
    display.drawFastHLine(0, kChatTextTop - 1, display.width(), TFT_DARKGREY);
    for (size_t i = 0; i < kItemCount; i++) {
        const int top = kChatTextTop + static_cast<int>(i) * kLineHeight;
        const uint16_t background = i == menuNav.selected() ? kSelectedBackground : TFT_BLACK;
        display.fillRect(0, top, display.width(), kLineHeight, background);
        display.setTextDatum(top_left);
        display.setTextColor(TFT_WHITE, background);
        display.drawString(kItemNames[i], kMargin, top);
        const int valueWidth = display.width() - 3 * kMargin - display.textWidth(kItemNames[i]);
        display.setTextDatum(top_right);
        display.setTextColor(i == kWhatsApp ? TFT_GREEN : TFT_LIGHTGREY, background);
        display.drawString(fitWidth(itemValue(i), valueWidth, displayWidth()).c_str(), display.width() - kMargin, top);
    }
}

void MenuScreen::onKeyDown(const KeyEvent& event) {
    if (app::isKey(event.key, app::kUpKey)) {
        app::screenDirty |= menuNav.up();
    } else if (app::isKey(event.key, app::kDownKey)) {
        app::screenDirty |= menuNav.down();
    } else if (app::isKey(event.key, app::kBackKey)) {
        close(pressed_);
    } else if (app::isKey(event.key, app::kOpenKey)) {
        const size_t item = menuNav.selected();
        closeToInbox();
        if (item == kMusic) {
            musicScreen.open();
        } else if (item == kFindIphone) {
            findScreen.open();
        } else if (item == kSettings) {
            settingsScreen.open(true);
        }
        app::statusDirty = true;
        app::screenDirty = true;
    }
}
