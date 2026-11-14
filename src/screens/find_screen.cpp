#include "screens/find_screen.h"

#include <M5Cardputer.h>

#include "app.h"
#include "ble/ble_manager.h"
#include "log.h"
#include "settings_store.h"
#include "ui/draw.h"

using namespace ui;

FindScreen findScreen;

namespace {

constexpr uint32_t kReadPeriodMs = 250;
constexpr uint32_t kLogPeriodMs = 1000;
// A Find iPhone screen left open with the screen off stops beeping after this long without a key.
constexpr uint32_t kQuietAfterMs = 5 * 60 * 1000;
constexpr int kBarTop = kChatTextTop + 10;
constexpr int kBarHeight = 16;
constexpr int kWordTop = kBarTop + kBarHeight + 10;

const char* levelWord(SignalLevel level) {
    switch (level) {
        case SignalLevel::Far:       return "Far";
        case SignalLevel::Near:      return "Near";
        case SignalLevel::VeryClose: return "Very close";
        case SignalLevel::None:      break;
    }
    return "Searching…";
}

}  // namespace

void FindScreen::open() {
    if (!app::settings.findIphone()) {
        app::settings.step(Setting::FindIphone, 1);
        settings_store::save(app::settings, Setting::FindIphone);
        logMessage(LogLevel::Info, "SYS", "find iPhone turned on");
    }
    meter_.clear();
    shownLevel_ = SignalLevel::None;
    shownBar_ = 0;
    const uint32_t now = millis();
    nextReadMs_ = now;
    nextBeepMs_ = now;
    lastKeyMs_ = now;
    app::screens.open(this);
    app::screenDirty = true;
}

void FindScreen::handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) {
    // Any key counts, also one that only turns the screen on.
    for (const KeyEvent& event : events) {
        if (event.kind == KeyEventKind::Down) {
            lastKeyMs_ = millis();
        }
    }
    ReadingScreen::handleKeys(pressed, events);
}

void FindScreen::update(uint32_t nowMs) {
    ReadingScreen::update(nowMs);
    if (static_cast<int32_t>(nowMs - nextReadMs_) >= 0) {
        meter_.add(ble::linkRssi(), nowMs);
        nextReadMs_ = nowMs + kReadPeriodMs;
    }
    int rssi = 0;
    if (static_cast<int32_t>(nowMs - nextLogMs_) >= 0 && meter_.average(nowMs, &rssi)) {
        logMessage(LogLevel::Debug, "BLE", "find signal avg=%d dBm", rssi);
        nextLogMs_ = nowMs + kLogPeriodMs;
    }
    const SignalLevel level = meter_.level(nowMs);
    if (level != shownLevel_ || meter_.barPercent(nowMs) != shownBar_) {
        app::screenDirty = true;
    }
    const uint32_t period = beepPeriodMs(level);
    if (period == 0 || nowMs - lastKeyMs_ >= kQuietAfterMs || static_cast<int32_t>(nowMs - nextBeepMs_) < 0) {
        return;
    }
    app::queueBeeps(1);
    nextBeepMs_ = nowMs + period;
}

void FindScreen::draw() {
    auto& display = M5Cardputer.Display;
    const uint32_t now = millis();
    shownLevel_ = meter_.level(now);
    shownBar_ = meter_.barPercent(now);
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    display.setFont(&fonts::efontCN_16);
    display.setTextDatum(top_left);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.drawString("Find iPhone", kMargin, kStatusHeight);
    display.drawFastHLine(0, kChatTextTop - 1, display.width(), TFT_DARKGREY);

    const int width = display.width() - 2 * kMargin;
    display.fillRect(kMargin, kBarTop, width, kBarHeight, TFT_DARKGREY);
    display.fillRect(kMargin, kBarTop, width * shownBar_ / 100, kBarHeight, TFT_GREEN);
    display.setTextDatum(top_center);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString(levelWord(shownLevel_), display.width() / 2, kWordTop);
}

void FindScreen::onKeyDown(const KeyEvent& event) {
    if (app::isKey(event.key, app::kBackKey)) {
        app::screens.remove(this);
        app::screenDirty = true;
    }
}
