#include "screens/settings_screen.h"

#include <M5Cardputer.h>

#include "app.h"
#include "ble/ancs_client.h"
#include "ble/ble_manager.h"
#include "core/settings_nav.h"
#include "settings_store.h"
#include "ui/draw.h"

using namespace ui;

SettingsScreen settingsScreen;

namespace {

// How long an action's prompt waits for the second Enter.
constexpr uint32_t kConfirmMs = 3000;
// The actions listed after the settings.
constexpr const char* kActions[] = {"Reset pairing", "Clear messages"};
constexpr size_t kActionCount = sizeof(kActions) / sizeof(kActions[0]);
SettingsNav settingsNav(kSettingCount, kActionCount, kConfirmMs);

void runAction(size_t action) {
    if (action == 0) {
        // With no bond there is nothing to erase: the restart still happens.
        ble::resetPairing();
        app::restartAfterPairingReset();
    } else {
        ancs::clearMessages();
        app::onMessagesChanged();
    }
}

void stepSetting(int direction) {
    const Setting setting = static_cast<Setting>(settingsNav.selected());
    if (!app::settings.step(setting, direction)) {
        return;
    }
    settings_store::save(app::settings, setting);
    if (setting == Setting::Brightness) {
        app::applyBrightness();
    } else if (setting == Setting::DisplayTimeout) {
        app::screenTimer.setTimeout(app::settings.displayTimeoutMs());
    }
    app::screenDirty = true;
}

}  // namespace

void SettingsScreen::open(bool fromStart) {
    if (fromStart) {
        settingsNav.reset();
    }
    app::screens.open(this);
    app::screenDirty = true;
}

bool SettingsScreen::cancelPrompt() {
    return settingsNav.cancel();
}

void SettingsScreen::update(uint32_t nowMs) {
    if (settingsNav.expire(nowMs)) {
        app::screenDirty = true;
    }
}

// The title, then one row per setting, its name on the left and its value on the right, then one row
// per action, replaced by its prompt while it waits for confirmation.
void SettingsScreen::draw() {
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
        display.drawString(app::settings.label(setting).c_str(), display.width() - kMargin, top);
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

void SettingsScreen::onKeyDown(const KeyEvent& event) {
    const KeyPos key = event.key;
    if (app::isKey(key, app::kOpenKey)) {
        if (settingsNav.enter(millis())) {
            runAction(settingsNav.selected() - kSettingCount);
        }
        app::screenDirty |= settingsNav.onAction();
        return;
    }
    // Any other key only cancels a prompt.
    if (settingsNav.cancel()) {
        app::screenDirty = true;
        return;
    }
    if (app::isKey(key, app::kUpKey)) {
        app::screenDirty |= settingsNav.up();
    } else if (app::isKey(key, app::kDownKey)) {
        app::screenDirty |= settingsNav.down();
    } else if (app::isKey(key, app::kPrevKey) && !settingsNav.onAction()) {
        stepSetting(-1);
    } else if (app::isKey(key, app::kNextKey) && !settingsNav.onAction()) {
        stepSetting(1);
    } else if (app::isKey(key, app::kBackKey)) {
        app::screens.close();
        app::screenDirty = true;
    }
}
