#include <M5Cardputer.h>
#include <esp_efuse.h>
#include <esp_efuse_table.h>

#include "app.h"
#include "ble/ams_client.h"
#include "ble/ancs_client.h"
#include "ble/ble_manager.h"
#include "ble/cts_client.h"
#include "core/key_events.h"
#include "log.h"
#include "mesh/lora_unit.h"
#include "mesh/mesh_storage.h"
#include "screens/call_screen.h"
#include "screens/connection_screen.h"
#include "screens/keyboard_screen.h"
#include "screens/mesh_screen.h"
#include "screens/node_list_screen.h"
#include "screens/menu_screen.h"
#include "screens/music_screen.h"
#include "settings_store.h"
#include "siri.h"
#include "ui/status_bar.h"

using namespace app;

namespace {

constexpr uint32_t kSerialWaitMs = 1500;
constexpr uint32_t kBatteryPollMs = 30000;
KeyTracker keyTracker;
uint32_t lastBatteryPollMs = 0;
// Messages added live so far: one more means a new arrival.
size_t seenLiveAdded = 0;
// Mesh messages received so far, likewise.
size_t seenMeshReceived = 0;

void pollScreenTimeout() {
    if (!screenTimer.expire(millis())) {
        return;
    }
    if (displayReady) {
        M5Cardputer.Display.sleep();
    }
    logMessage(LogLevel::Debug, "SYS", "screen off");
    ble::requestTimings(true);
}

// count messages arrived: a beep each when sound is on, and the screen on with wake on new message. During a call
// only the ring is heard.
void announceArrivals(size_t count, bool sound) {
    if (sound && !screens.isOpen(&callScreen)) {
        queueBeeps(count);
    }
    if (settings.wakeOnMessage()) {
        wakeScreen();
    }
}

// Each live Message beeps and can turn the screen on. One that arrived while typing shows as the Banner;
// in Reading mode the screens show it.
void pollArrivals() {
    const MessageStore& store = ancs::messages();
    if (store.liveAdded() == seenLiveAdded) {
        return;
    }
    announceArrivals(store.liveAdded() - seenLiveAdded, settings.sound());
    seenLiveAdded = store.liveAdded();
    const Message* newest = store.newestLive();
    if (inKeyboardMode() && newest != nullptr) {
        keyboardScreen.showBanner(*newest);
    }
}

// Find iPhone: a link lost by signal starts the Out-of-range alert, which beeps even with sound off; the
// iPhone's return or Find iPhone turned off ends it.
// Each Mesh message received beeps with Mesh sound on, whatever sound says.
void pollMeshArrivals() {
    const size_t received = lora::messages().received();
    if (received == seenMeshReceived) {
        return;
    }
    announceArrivals(received - seenMeshReceived, settings.meshSound());
    seenMeshReceived = received;
}

void pollOutOfRange() {
    const bool lost = ble::takeLinkLost();
    if (lost && settings.findIphone()) {
        outOfRangeAlert.start(millis());
        wakeScreen();
        screenDirty = true;
        logMessage(LogLevel::Info, "SYS", "out-of-range alert started");
    } else if (outOfRangeAlert.active() &&
               (ble::phase() == ble::Phase::Connected || ble::phase() == ble::Phase::Secured || !settings.findIphone())) {
        outOfRangeAlert.stop();
        screenDirty = true;
        logMessage(LogLevel::Info, "SYS", "out-of-range alert ended");
    }
    if (outOfRangeAlert.due(millis())) {
        queueBeeps(3);
    }
}

std::vector<KeyPos> readPressedKeys() {
    std::vector<KeyPos> pressed;
    for (const auto& point : M5Cardputer.Keyboard.keyList()) {
        pressed.push_back({static_cast<uint8_t>(point.y), static_cast<uint8_t>(point.x)});
    }
    return pressed;
}

void pollBattery() {
    if (lastBatteryPollMs != 0 && millis() - lastBatteryPollMs < kBatteryPollMs) {
        return;
    }
    lastBatteryPollMs = millis();
    const int32_t level = M5Cardputer.Power.getBatteryLevel();
    const int32_t known = level < 0 ? -1 : (level > 100 ? 100 : level);
    if (known != batteryLevel) {
        batteryLevel = known;
        statusDirty = true;
    }
}

void pollG0(const std::vector<KeyPos>& pressed) {
    if (M5Cardputer.BtnA.wasPressed()) {
        logMessage(LogLevel::Debug, "BTN", "G0 down");
        silenceOutOfRangeAlert();
        const bool fn = containsKey(pressed, kFnKey);
        // A press that turns the screen on leaves the mode as it is; during a call G0 does nothing, Fn+G0
        // included. Without the iPhone there is no Keyboard mode: only Fn+G0 works.
        if (!wakeScreen() && !screens.isOpen(&callScreen) && (fn || ble::phase() == ble::Phase::Secured)) {
            const bool menuShown = screens.top() == &menuScreen;
            // Fn+G0 opens or closes the Menu; G0 alone closes it, then switches the mode.
            if (menuShown) {
                menuScreen.close(pressed);
            }
            if (fn) {
                if (!menuShown) {
                    menuScreen.open();
                }
            } else if (inKeyboardMode()) {
                keyboardScreen.close();
            } else {
                keyboardScreen.open(pressed);
            }
        }
    }
    if (M5Cardputer.BtnA.wasReleased()) {
        logMessage(LogLevel::Debug, "BTN", "G0 up");
    }
}

// The eFuse records in-package PSRAM: when it is set but the PSRAM did not come up,
// the chip has PSRAM that failed to initialize.
String psramStatus() {
    if (psramFound()) {
        return String(ESP.getPsramSize());
    }
    uint8_t capacity = 0;
    esp_efuse_read_field_blob(ESP_EFUSE_PSRAM_CAP, &capacity, esp_efuse_get_field_size(ESP_EFUSE_PSRAM_CAP));
    return capacity != 0 ? "error" : "none";
}

}  // namespace

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    displayReady = M5.getDisplayCount() > 0;
    if (displayReady) {
        M5Cardputer.Display.fillScreen(TFT_BLACK);
    }

    Serial.begin(115200);
    // Give a just-opened serial monitor time to attach, so the boot report is not lost.
    const uint32_t start = millis();
    while (!Serial && millis() - start < kSerialWaitMs) {
        delay(10);
    }

    logMessage(LogLevel::Info, "SYS", "boot version=%s psram=%s heap=%u flash=%u", TASCA_VERSION,
               psramStatus().c_str(), static_cast<unsigned>(ESP.getFreeHeap()),
               static_cast<unsigned>(ESP.getFlashChipSize()));
    if (!displayReady) {
        logMessage(LogLevel::Error, "SYS", "display init failed");
    }
    settings_store::load(settings);
    applyBrightness();
    screenTimer.setTimeout(settings.displayTimeoutMs());
    screenTimer.activity(millis());

    pollBattery();
    ble::begin(static_cast<uint8_t>(batteryLevel < 0 ? 0 : batteryLevel));
    lora::begin();
    lora::messages().setClock(&cts::clock());
    mesh_storage::begin(lora::messages());
    lora::setMeshName(settings_store::loadMeshName());
}

void loop() {
    M5Cardputer.update();
    const bool bleChanged = ble::poll();
    const bool ancsChanged = ancs::poll();
    statusDirty |= bleChanged || ancsChanged;
    pollOutOfRange();
    if (ancsChanged) {
        onMessagesChanged();
        pollArrivals();
    }
    // Only the Menu and the Music screen show what the iPhone plays.
    if (ams::poll() && (screens.top() == &menuScreen || screens.top() == &musicScreen)) {
        screenDirty = true;
    }
    // The Mesh screen shows the messages as they arrive, the Node list the nodes heard, the Menu the unread count.
    if (lora::poll()) {
        if (screens.top() == &meshScreen) {
            meshScreen.markRead();
            meshScreen.layout();
            screenDirty = true;
        } else if (screens.top() == &menuScreen || screens.top() == &nodeListScreen) {
            screenDirty = true;
        }
        pollMeshArrivals();
    }
    mesh_storage::save(lora::messages(), millis());
    screens.top()->update(millis());
    pollBattery();
    const std::vector<KeyPos> pressed = readPressedKeys();
    // Before the Call screen: a call ended by a disconnection closes under the Connection screen.
    connectionScreen.sync(pressed);
    callScreen.sync(pressed);
    keyboardScreen.updateReady(pressed);
    pollG0(pressed);
    screens.top()->handleKeys(pressed, keyTracker.update(pressed));
    pollSiri();
    // The screen stays on while a call rings.
    if (!screens.isOpen(&callScreen)) {
        pollScreenTimeout();
    }
    pollBeeps();
    if (!canDraw()) {
        delay(10);
        return;
    }
    // The Connection screen draws the whole display, battery included.
    const bool connectionShown = screens.top() == &connectionScreen;
    if (statusDirty && !connectionShown) {
        ui::drawStatus();
    }
    if (screenDirty || (statusDirty && connectionShown)) {
        screens.top()->draw();
    } else if (contextDirty && inKeyboardMode()) {
        keyboardScreen.drawContext();
    }
    statusDirty = false;
    screenDirty = false;
    contextDirty = false;
    delay(10);
}
