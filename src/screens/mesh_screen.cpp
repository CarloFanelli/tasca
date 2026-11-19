#include "screens/mesh_screen.h"

#include <M5Cardputer.h>

#include "app.h"
#include "core/mesh_view.h"
#include "mesh/lora_unit.h"
#include "mesh/mesh_storage.h"
#include "screens/node_list_screen.h"
#include "ui/draw.h"

using namespace ui;

MeshScreen meshScreen;

namespace {

constexpr uint32_t kAgeRefreshMs = 60000;
// A Meshtastic text message carries at most 200 bytes.
constexpr size_t kMaxTextBytes = 200;
// The one thing the Mesh screen's clear prompt clears.
const char* const kEveryMessage = "mesh";

size_t screenLines() {
    return static_cast<size_t>((M5Cardputer.Display.height() - kChatTextTop) / kLineHeight);
}

}  // namespace

MeshScreen::MeshScreen()
    : entry_(kMaxTextBytes, kMaxTextBytes, {app::kOpenKey, app::kBackKey}), clearPrompt_(app::kClearConfirmMs) {}

void MeshScreen::open() {
    composing_ = false;
    clearPrompt_.cancel();
    scroll_.reset();
    app::screens.open(this);
    app::screenDirty = true;
}

void MeshScreen::onShown() {
    markRead();
    layout();
}

void MeshScreen::layout() {
    auto& display = M5Cardputer.Display;
    display.setFont(&fonts::efontCN_16);
    laidOutMs_ = millis();
    lines_ = meshLines(lora::messages(), laidOutMs_, display.width() - 2 * kMargin, displayWidth());
    size_t newestStart = 0;
    for (size_t i = 0; i < lines_.size(); i++) {
        if (lines_[i].header) {
            newestStart = i;
        }
    }
    scroll_.setLines(lines_.size(), listLines(), newestStart);
}

void MeshScreen::markRead() {
    lora::messages().markAllRead();
}

void MeshScreen::handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) {
    pressed_ = pressed;
    if (composing_ && entry_.update(pressed)) {
        radioLimit_ = false;
        app::screenDirty = true;
    }
    ReadingScreen::handleKeys(pressed, events);
}

void MeshScreen::update(uint32_t nowMs) {
    ReadingScreen::update(nowMs);
    app::screenDirty |= clearPrompt_.expire(nowMs);
    if (nowMs - laidOutMs_ >= kAgeRefreshMs) {
        layout();
        app::screenDirty = true;
    }
}

// The title, with "Connect the LoRa unit" beside it while the unit is missing, else "not saved" without a usable
// microSD card; the clear prompt in its place. Then the messages and the compose line.
void MeshScreen::draw() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    display.setFont(&fonts::efontCN_16);
    display.setTextDatum(top_left);
    if (clearPrompt_.shown()) {
        display.setTextColor(TFT_RED, TFT_BLACK);
        display.drawString("Press Del again to clear", kMargin, kStatusHeight);
    } else {
        display.setTextColor(TFT_YELLOW, TFT_BLACK);
        display.drawString("Mesh", kMargin, kStatusHeight);
        const char* note = !lora::present() ? "Connect the LoRa unit" : !mesh_storage::saving() ? "not saved" : nullptr;
        if (note != nullptr) {
            display.setTextDatum(top_right);
            display.setTextColor(TFT_ORANGE, TFT_BLACK);
            display.drawString(note, display.width() - kMargin, kStatusHeight);
            display.setTextDatum(top_left);
        }
    }
    display.drawFastHLine(0, kChatTextTop - 1, display.width(), TFT_DARKGREY);
    if (composing_) {
        drawCompose();
    }
    if (lines_.empty()) {
        display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        display.drawString("No messages yet", kMargin, kChatTextTop);
        return;
    }
    for (size_t i = 0; i < listLines() && scroll_.top() + i < lines_.size(); i++) {
        const ChatLine& line = lines_[scroll_.top() + i];
        display.setTextColor(line.header ? TFT_CYAN : TFT_WHITE, TFT_BLACK);
        display.drawString(line.text.c_str(), kMargin, kChatTextTop + static_cast<int>(i) * kLineHeight);
    }
}

// On the last line: the end of the text typed, then the bytes left, or "Radio limit: wait" after a refused Enter.
void MeshScreen::drawCompose() {
    auto& display = M5Cardputer.Display;
    const int top = kChatTextTop + static_cast<int>(listLines()) * kLineHeight;
    display.fillRect(0, top, display.width(), kLineHeight, kSelectedBackground);
    const std::string right = radioLimit_ ? "Radio limit: wait" : std::to_string(kMaxTextBytes - entry_.text().size());
    display.setTextDatum(top_right);
    display.setTextColor(radioLimit_ ? TFT_RED : TFT_LIGHTGREY, kSelectedBackground);
    display.drawString(right.c_str(), display.width() - kMargin, top);
    const int textWidth = display.width() - 3 * kMargin - display.textWidth(right.c_str());
    display.setTextDatum(top_left);
    display.setTextColor(TFT_WHITE, kSelectedBackground);
    display.drawString(lastFitting("> " + displayable(entry_.text(), true), textWidth, displayWidth()).c_str(), kMargin,
                       top);
}

size_t MeshScreen::listLines() const {
    return screenLines() - (composing_ ? 1 : 0);
}

void MeshScreen::openCompose() {
    composing_ = true;
    radioLimit_ = false;
    entry_.start(pressed_);
    layout();
    app::screenDirty = true;
}

void MeshScreen::closeCompose() {
    composing_ = false;
    layout();
    app::screenDirty = true;
}

void MeshScreen::send() {
    if (entry_.text().empty()) {
        return;
    }
    if (!lora::sendText(entry_.text())) {
        radioLimit_ = true;
        app::screenDirty = true;
        return;
    }
    closeCompose();
}

// The first Del asks, a second within 3 s clears every message, on screen and on the microSD card.
void MeshScreen::clearMessages() {
    if (clearPrompt_.del(kEveryMessage, millis())) {
        mesh_storage::clearMessages(lora::messages());
        layout();
    }
    app::screenDirty = true;
}

void MeshScreen::onKeyDown(const KeyEvent& event) {
    const bool del = app::isKey(event.key, app::kDelKey) && event.modifiers == 0;
    if (!del) {
        app::screenDirty |= clearPrompt_.cancel();
    }
    if (composing_) {
        if (app::isKey(event.key, app::kOpenKey)) {
            send();
        } else if (app::isKey(event.key, app::kBackKey)) {
            closeCompose();
        }
    } else if (app::isKey(event.key, app::kUpKey)) {
        app::screenDirty |= scroll_.up();
    } else if (app::isKey(event.key, app::kDownKey)) {
        app::screenDirty |= scroll_.down();
    } else if (app::isKey(event.key, app::kOpenKey)) {
        if (lora::present()) {
            openCompose();
        }
    } else if (app::isKey(event.key, app::kTabKey)) {
        nodeListScreen.open();
    } else if (del) {
        clearMessages();
    } else if (app::isKey(event.key, app::kBackKey)) {
        app::screens.remove(this);
        app::screenDirty = true;
    }
}
