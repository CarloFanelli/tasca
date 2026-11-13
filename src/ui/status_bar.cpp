#include "ui/status_bar.h"

#include <M5Cardputer.h>

#include "app.h"
#include "ble/ancs_client.h"
#include "ble/ble_manager.h"
#include "core/inbox.h"
#include "screens/keyboard_screen.h"
#include "ui/draw.h"

namespace ui {

namespace {

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

}  // namespace

std::string batteryLabel() {
    char battery[8];
    if (app::batteryLevel < 0) {
        snprintf(battery, sizeof(battery), "--%%");
    } else {
        snprintf(battery, sizeof(battery), "%d%%", static_cast<int>(app::batteryLevel));
    }
    return battery;
}

void drawStatus() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, 0, display.width(), kStatusHeight, TFT_BLACK);
    display.drawFastHLine(0, kStatusHeight - 1, display.width(), TFT_DARKGREY);
    display.setFont(&fonts::Font2);
    constexpr int y = (kStatusHeight - 1) / 2;

    // Still Keyboard mode under a screen opened over it.
    const bool keyboardMode = app::screens.isOpen(&keyboardScreen);
    const char* mode = keyboardMode ? "Keyboard" : "Reading";
    display.setTextDatum(middle_left);
    display.setTextColor(keyboardMode ? (app::keyboardReady ? TFT_GREEN : TFT_RED) : TFT_LIGHTGREY, TFT_BLACK);
    display.drawString(mode, 2, y);
    const int linkLeft = 2 + display.textWidth(mode) + 8;

    char right[32];
    snprintf(right, sizeof(right), "%s unread  %s", unreadLabel(ancs::messages().unread()).c_str(),
             batteryLabel().c_str());
    display.setTextDatum(middle_right);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString(right, display.width() - 2, y);

    const int linkWidth = display.width() - 2 - display.textWidth(right) - 8 - linkLeft;
    display.setTextDatum(middle_left);
    display.setTextColor(TFT_CYAN, TFT_BLACK);
    display.drawString(fitWidth(linkText(), linkWidth, displayWidth()).c_str(), linkLeft, y);
}

}  // namespace ui
