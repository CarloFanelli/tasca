#include "screens/inbox_screen.h"

#include <M5Cardputer.h>

#include "app.h"
#include "screens/chat_screen.h"
#include "screens/settings_screen.h"
#include "ui/draw.h"

using namespace ui;

InboxScreen inboxScreen;

namespace {

// Inbox entry: Chat name line, then preview line.
constexpr int kRowHeight = 30;
constexpr int kPreviewOffset = 16;

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
    // The clear prompt or notice takes the place of the selected Chat's preview.
    const char* clearText = clearPromptText(chat.chat);
    if (selected && clearText != nullptr) {
        display.setTextColor(TFT_RED, background);
        display.drawString(clearText, kMargin, top + kPreviewOffset);
        return;
    }
    display.setTextColor(TFT_LIGHTGREY, background);
    const std::string preview = withSender(chat.group, chat.sender, chat.text);
    display.drawString(fitWidth(displayable(preview, true), right - kMargin, displayWidth()).c_str(), kMargin,
                       top + kPreviewOffset);
}

}  // namespace

void InboxScreen::draw() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    if (app::chats.empty()) {
        display.setFont(&fonts::Font2);
        display.setTextDatum(middle_center);
        display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        display.drawString("No messages yet", display.width() / 2, (kStatusHeight + display.height()) / 2);
        return;
    }
    const size_t rows = static_cast<size_t>((display.height() - kStatusHeight) / kRowHeight);
    const size_t first = app::nav.firstVisible(rows);
    for (size_t i = first; i < app::chats.size() && i < first + rows; i++) {
        drawInboxEntry(app::chats[i], kStatusHeight + static_cast<int>(i - first) * kRowHeight,
                       i == app::nav.selected());
    }
}

void InboxScreen::onKeyDown(const KeyEvent& event) {
    if (app::isKey(event.key, app::kSettingsKey) && (event.modifiers & kModOpt) != 0) {
        settingsScreen.open();
    } else if (app::isKey(event.key, app::kUpKey)) {
        app::screenDirty |= app::nav.up();
    } else if (app::isKey(event.key, app::kDownKey)) {
        app::screenDirty |= app::nav.down();
    } else if (app::isKey(event.key, app::kOpenKey)) {
        chatScreen.open();
    } else if (app::isKey(event.key, app::kDelKey) && event.modifiers == 0 && !app::chats.empty()) {
        app::delOnChat(app::chats[app::nav.selected()].chat);
    }
}
