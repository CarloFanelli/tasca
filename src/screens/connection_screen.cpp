#include "screens/connection_screen.h"

#include <M5Cardputer.h>

#include "app.h"
#include "ble/ble_manager.h"
#include "core/connection_line.h"
#include "log.h"
#include "screens/keyboard_screen.h"
#include "siri.h"
#include "ui/draw.h"
#include "ui/status_bar.h"

using namespace app;
using namespace ui;

ConnectionScreen connectionScreen;

namespace {

// "Tasca": the font scaled kNameScale times, strokes kNameStroke font pixels thick.
constexpr int kNameScale = 5;
constexpr int kNameStroke = 2;
constexpr int kNameTop = 10;
constexpr int kLineTop = kNameTop + (16 + kNameStroke - 1) * kNameScale + 8;

}  // namespace

void ConnectionScreen::sync(const std::vector<KeyPos>& pressed) {
    const bool secured = ble::phase() == ble::Phase::Secured;
    const bool open = screens.isOpen(this);
    if (!secured && !open) {
        // A Voice Command held when the iPhone left would stay down once it is back.
        cancelSiri();
        screens.open(this);
        logMessage(LogLevel::Info, "SYS", "connection screen opened");
    } else if (secured && open) {
        screens.remove(this);
        if (inKeyboardMode()) {
            keyboardScreen.resume(pressed);
        }
        logMessage(LogLevel::Info, "SYS", "connection screen closed");
    } else {
        return;
    }
    statusDirty = true;
    screenDirty = true;
}

void ConnectionScreen::handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) {
    if (pairingResetCombo.update(pressed, millis()) && ble::resetPairing()) {
        restartAfterPairingReset();
    }
    // A key only turns the screen on, or keeps it on.
    for (const KeyEvent& event : events) {
        if (event.kind == KeyEventKind::Down) {
            wakeScreen();
        }
    }
}

void ConnectionScreen::draw() {
    auto& display = M5Cardputer.Display;
    display.fillScreen(TFT_BLACK);
    display.setFont(&fonts::Font2);
    display.setTextDatum(middle_right);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString(batteryLabel().c_str(), display.width() - 2, (kStatusHeight - 1) / 2);

    display.setTextDatum(top_center);
    // The font the screens already use, scaled: a font of its own would cost about 9 KB of flash.
    display.setFont(&fonts::efontCN_16);
    display.setTextSize(kNameScale);
    // Drawn again one font pixel apart, across and down, to thicken the strokes.
    display.setTextColor(TFT_WHITE);
    const int left = display.width() / 2 - (kNameStroke - 1) * kNameScale / 2;
    for (int dy = 0; dy < kNameStroke; dy++) {
        for (int dx = 0; dx < kNameStroke; dx++) {
            display.drawString("Tasca", left + dx * kNameScale, kNameTop + dy * kNameScale);
        }
    }
    display.setTextSize(1);
    const ble::Phase phase = ble::phase();
    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.drawString(connectionLine(ble::bonded(), phase == ble::Phase::Connected, phase == ble::Phase::PairingFailed),
                       display.width() / 2, kLineTop);
}
