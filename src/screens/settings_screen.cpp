#include "screens/settings_screen.h"

#include <M5Cardputer.h>

#include "app.h"
#include "ble/ancs_client.h"
#include "ble/ble_manager.h"
#include "core/mesh_name.h"
#include "core/settings_nav.h"
#include "mesh/lora_unit.h"
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
// The Mesh name row, between the settings and the actions.
constexpr size_t kNameRow = kSettingCount;
constexpr size_t kTextRowCount = 1;
constexpr size_t kRowCount = kSettingCount + kTextRowCount + kActionCount;
SettingsNav settingsNav(kSettingCount, kActionCount, kConfirmMs, kTextRowCount);

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

SettingsScreen::SettingsScreen() : nameEntry_(kMeshNameBytes, kMeshNameChars, {app::kOpenKey, app::kBackKey}) {}

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

void SettingsScreen::handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) {
    pressed_ = pressed;
    if (editing_ && nameEntry_.update(pressed)) {
        app::screenDirty = true;
    }
    ReadingScreen::handleKeys(pressed, events);
}

void SettingsScreen::onCovered() {
    cancelPrompt();
    editing_ = false;
}

void SettingsScreen::update(uint32_t nowMs) {
    if (settingsNav.expire(nowMs)) {
        app::screenDirty = true;
    }
}

// The title, then one row per setting, its name on the left and its value on the right, then the Mesh name, then one
// row per action, replaced by its prompt while it waits for confirmation. Only the rows that fit are drawn,
// scrolled to keep the selected one visible.
void SettingsScreen::draw() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    display.setFont(&fonts::efontCN_16);
    display.setTextDatum(top_left);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.drawString("Settings", kMargin, kStatusHeight);
    display.drawFastHLine(0, kChatTextTop - 1, display.width(), TFT_DARKGREY);
    const size_t visibleRows = static_cast<size_t>((display.height() - kChatTextTop) / kLineHeight);
    const size_t first = settingsNav.firstVisible(visibleRows);
    for (size_t row = first; row < kRowCount && row < first + visibleRows; row++) {
        const int top = kChatTextTop + static_cast<int>(row - first) * kLineHeight;
        const bool selected = row == settingsNav.selected();
        const uint16_t background = selected ? kSelectedBackground : TFT_BLACK;
        display.fillRect(0, top, display.width(), kLineHeight, background);
        display.setTextDatum(top_left);
        if (row == kNameRow) {
            drawNameRow(top, selected);
        } else if (row < kSettingCount) {
            const Setting setting = static_cast<Setting>(row);
            display.setTextColor(TFT_WHITE, background);
            display.drawString(settingName(setting), kMargin, top);
            display.setTextDatum(top_right);
            display.setTextColor(TFT_GREEN, background);
            display.drawString(app::settings.label(setting).c_str(), display.width() - kMargin, top);
        } else if (selected && settingsNav.confirming()) {
            display.setTextColor(TFT_RED, background);
            display.drawString("Press Enter to confirm", kMargin, top);
        } else {
            display.setTextColor(TFT_ORANGE, background);
            display.drawString(kActions[row - kSettingCount - kTextRowCount], kMargin, top);
        }
    }
}

// "Mesh name" and the name announced; while edited, the end of the text typed.
void SettingsScreen::drawNameRow(int top, bool selected) {
    auto& display = M5Cardputer.Display;
    const uint16_t background = selected ? kSelectedBackground : TFT_BLACK;
    display.setTextColor(TFT_WHITE, background);
    if (editing_) {
        display.drawString(
            lastFitting("> " + displayable(nameEntry_.text(), true), display.width() - 2 * kMargin, displayWidth())
                .c_str(),
            kMargin, top);
        return;
    }
    display.drawString("Mesh name", kMargin, top);
    const int valueWidth = display.width() - 3 * kMargin - display.textWidth("Mesh name");
    display.setTextDatum(top_right);
    display.setTextColor(TFT_GREEN, background);
    display.drawString(fitWidth(displayable(lora::meshName(), true), valueWidth, displayWidth()).c_str(),
                       display.width() - kMargin, top);
}

void SettingsScreen::onKeyDown(const KeyEvent& event) {
    const KeyPos key = event.key;
    // Editing the Mesh name: Enter saves it, ` leaves it as it was, other keys type.
    if (editing_) {
        if (app::isKey(key, app::kOpenKey)) {
            settings_store::saveMeshName(nameEntry_.text());
            lora::setMeshName(nameEntry_.text());
        }
        if (app::isKey(key, app::kOpenKey) || app::isKey(key, app::kBackKey)) {
            editing_ = false;
            app::screenDirty = true;
        }
        return;
    }
    if (app::isKey(key, app::kOpenKey) && settingsNav.onText()) {
        nameEntry_.start(pressed_, lora::meshName());
        editing_ = true;
        app::screenDirty = true;
        return;
    }
    if (app::isKey(key, app::kOpenKey)) {
        if (settingsNav.enter(millis())) {
            runAction(settingsNav.selected() - kSettingCount - kTextRowCount);
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
    } else if (app::isKey(key, app::kPrevKey) && settingsNav.selected() < kSettingCount) {
        stepSetting(-1);
    } else if (app::isKey(key, app::kNextKey) && settingsNav.selected() < kSettingCount) {
        stepSetting(1);
    } else if (app::isKey(key, app::kBackKey)) {
        app::screens.close();
        app::screenDirty = true;
    }
}
