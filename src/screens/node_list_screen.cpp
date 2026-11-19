#include "screens/node_list_screen.h"

#include <M5Cardputer.h>

#include <algorithm>
#include <string>

#include "app.h"
#include "core/mesh_view.h"
#include "mesh/lora_unit.h"
#include "ui/draw.h"

using namespace ui;

NodeListScreen nodeListScreen;

namespace {

constexpr uint32_t kAgeRefreshMs = 60000;

size_t visibleRows() {
    return static_cast<size_t>((M5Cardputer.Display.height() - kChatTextTop) / kLineHeight);
}

}  // namespace

void NodeListScreen::open() {
    top_ = 0;
    app::screens.open(this);
    app::screenDirty = true;
}

void NodeListScreen::update(uint32_t nowMs) {
    ReadingScreen::update(nowMs);
    if (nowMs - drawnMs_ >= kAgeRefreshMs) {
        app::screenDirty = true;
    }
}

// One row per node: short and long name on the left, signal and time since last heard on the right.
void NodeListScreen::draw() {
    auto& display = M5Cardputer.Display;
    drawnMs_ = millis();
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    display.setFont(&fonts::efontCN_16);
    display.setTextDatum(top_left);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.drawString("Nodes", kMargin, kStatusHeight);
    display.drawFastHLine(0, kChatTextTop - 1, display.width(), TFT_DARKGREY);
    const MeshStore& store = lora::messages();
    if (store.nodes().empty()) {
        display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        display.drawString("No nodes heard yet", kMargin, kChatTextTop);
        return;
    }
    top_ = std::min(top_, store.nodes().size() - 1);
    for (size_t i = 0; i < visibleRows() && top_ + i < store.nodes().size(); i++) {
        const MeshNode& node = store.nodes()[top_ + i];
        const int y = kChatTextTop + static_cast<int>(i) * kLineHeight;
        // A node loaded from the microSD card was last heard before the restart, at a time not known.
        const std::string age = node.restored ? "--" : elapsedLabel(drawnMs_ - node.lastHeardMs);
        const std::string right = std::to_string(node.rssi) + " dBm " + age;
        display.setTextDatum(top_right);
        display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        display.drawString(right.c_str(), display.width() - kMargin, y);
        const std::string names = store.shortNameOf(node.id) + " " + node.longName;
        const int namesWidth = display.width() - 3 * kMargin - display.textWidth(right.c_str());
        display.setTextDatum(top_left);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.drawString(fitWidth(displayable(names, true), namesWidth, displayWidth()).c_str(), kMargin, y);
    }
}

void NodeListScreen::onKeyDown(const KeyEvent& event) {
    const size_t count = lora::messages().nodes().size();
    if (app::isKey(event.key, app::kUpKey) && top_ > 0) {
        top_--;
        app::screenDirty = true;
    } else if (app::isKey(event.key, app::kDownKey) && top_ + visibleRows() < count) {
        top_++;
        app::screenDirty = true;
    } else if (app::isKey(event.key, app::kBackKey)) {
        app::screens.remove(this);
        app::screenDirty = true;
    }
}
