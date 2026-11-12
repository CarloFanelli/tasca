#include <M5Cardputer.h>
#include <esp_efuse.h>
#include <esp_efuse_table.h>

#include "ble/ancs_client.h"
#include "ble/ble_manager.h"
#include "ble/hid_keyboard.h"
#include "core/combo_hold.h"
#include "core/echo_line.h"
#include "core/key_events.h"
#include "core/keymap.h"
#include "core/utf8.h"
#include "log.h"

namespace {

constexpr uint32_t kSerialWaitMs = 1500;
constexpr int kStatusTop = 64;
constexpr int kModeTop = 88;
constexpr int kKeyAreaTop = 106;
// How long "pairing reset" stays on screen before the board restarts.
constexpr uint32_t kResetNoticeMs = 1500;
// Consumer control usage that toggles the iPhone's on-screen keyboard.
constexpr uint16_t kEjectUsage = 0xB8;
// The iPhone tells a keyboard nothing about app or field changes: a pause starts a new echo line.
constexpr uint32_t kEchoIdleMs = 10000;

bool displayReady = false;
KeyTracker keyTracker;
// Fn + Del held for 3 s: forget the bonded iPhone.
ComboHold pairingResetCombo({{2, 0}, {0, 13}}, 3000);

// Reading mode: keys stay on the Cardputer. Keyboard mode: keys go to the iPhone.
bool keyboardMode = false;
bool keyboardReady = false;
KeyboardMapper keyboard;
EchoLine echo;
uint32_t lastReportMs = 0;
uint32_t keyPressesAtModeStart = 0;

void drawKeyLabel(const char* text) {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kKeyAreaTop, display.width(), display.height() - kKeyAreaTop, TFT_BLACK);
    display.setTextDatum(middle_center);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.setFont(&fonts::Font4);
    if (display.textWidth(text) > display.width()) {
        display.setFont(&fonts::Font2);
    }
    display.drawString(text, display.width() / 2, (kKeyAreaTop + display.height()) / 2);
}

// The end of the echo line that fits the screen, left-aligned in the key area.
void drawEcho() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kKeyAreaTop, display.width(), display.height() - kKeyAreaTop, TFT_BLACK);
    display.setTextDatum(middle_left);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setFont(&fonts::efontCN_16);
    constexpr int kMargin = 4;
    const std::string& text = echo.text();
    size_t start = 0;
    while (start < text.size() && display.textWidth(text.c_str() + start) > display.width() - 2 * kMargin) {
        do {
            start++;
        } while (start < text.size() && isUtf8Continuation(text[start]));
    }
    display.drawString(text.c_str() + start, kMargin, (kKeyAreaTop + display.height()) / 2);
}

void drawStatus() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kStatusTop, display.width(), kModeTop - kStatusTop, TFT_BLACK);
    display.setTextDatum(middle_center);
    display.setTextColor(TFT_CYAN, TFT_BLACK);
    display.setFont(&fonts::Font2);
    char text[48];
    switch (ancs::state()) {
        case ancs::State::Active:
            snprintf(text, sizeof(text), "WhatsApp: %u messages, %u unread",
                     static_cast<unsigned>(ancs::messages().size()), static_cast<unsigned>(ancs::messages().unread()));
            break;
        case ancs::State::Denied:
            snprintf(text, sizeof(text), "ANCS denied");
            break;
        case ancs::State::Idle:
            snprintf(text, sizeof(text), "BLE: %s", ble::phaseName(ble::phase()));
            break;
    }
    display.drawString(text, display.width() / 2, (kStatusTop + kModeTop) / 2);
}

void drawMode() {
    auto& display = M5Cardputer.Display;
    display.fillRect(0, kModeTop, display.width(), kKeyAreaTop - kModeTop, TFT_BLACK);
    display.setTextDatum(middle_center);
    display.setFont(&fonts::Font2);
    const char* text = "Reading mode";
    uint16_t color = TFT_LIGHTGREY;
    if (keyboardMode) {
        text = keyboardReady ? "Keyboard mode" : "Keyboard mode - iPhone not ready";
        color = keyboardReady ? TFT_GREEN : TFT_RED;
    }
    display.setTextColor(color, TFT_BLACK);
    display.drawString(text, display.width() / 2, (kModeTop + kKeyAreaTop) / 2);
}

void drawHello() {
    auto& display = M5Cardputer.Display;
    display.fillScreen(TFT_BLACK);
    display.setTextDatum(middle_center);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setFont(&fonts::FreeSansBold18pt7b);
    display.drawString("Hello", display.width() / 2, 26);
    display.setFont(&fonts::Font2);
    display.drawString("Tasca v" TASCA_VERSION, display.width() / 2, 52);
    drawStatus();
    drawMode();
    drawKeyLabel("-");
}

std::vector<KeyPos> readPressedKeys() {
    std::vector<KeyPos> pressed;
    for (const auto& point : M5Cardputer.Keyboard.keyList()) {
        pressed.push_back({static_cast<uint8_t>(point.y), static_cast<uint8_t>(point.x)});
    }
    return pressed;
}

// Keys reach the iPhone only on a secured link whose keyboard report the iPhone has enabled.
void updateKeyboardReady(const std::vector<KeyPos>& pressed) {
    const bool ready = ble::phase() == ble::Phase::Secured && ble::keyboardReportsEnabled();
    if (ready == keyboardReady) {
        return;
    }
    keyboardReady = ready;
    logMessage(LogLevel::Info, "HID", ready ? "keyboard ready" : "keyboard not ready");
    // Keys held while the iPhone was away are not replayed: they count once released and pressed again.
    keyboard.reset(pressed);
    // The iPhone released every key; the echo line itself is kept.
    echo.apply(KeyboardReport{});
    if (keyboardMode && displayReady) {
        drawMode();
    }
}

bool sendKeyboardReport(const KeyboardReport& report) {
    if (!ble::sendKeyboardReport(report)) {
        logMessage(LogLevel::Warn, "HID", "keyboard report not sent");
        return false;
    }
    return true;
}

void setKeyboardMode(bool on, const std::vector<KeyPos>& pressed) {
    if (on) {
        keyboard.reset(pressed);
        keyPressesAtModeStart = keyboard.keyPresses();
        logMessage(LogLevel::Info, "HID", "keyboard mode on");
    } else {
        if (keyboardReady && keyboard.report() != KeyboardReport{}) {
            keyboard.reset({});
            sendKeyboardReport(keyboard.report());
        }
        logMessage(LogLevel::Info, "HID", "keyboard mode off: %u keys sent",
                   static_cast<unsigned>(keyboard.keyPresses() - keyPressesAtModeStart));
    }
    echo.clear();
    keyboardMode = on;
    if (displayReady) {
        drawMode();
        if (on) {
            drawEcho();
        } else {
            drawKeyLabel("-");
        }
    }
}

void pollKeyboardMode(const std::vector<KeyPos>& pressed) {
    // Typed text never reaches the serial log: no KEY lines in this mode.
    keyTracker.update(pressed);
    pairingResetCombo.update({}, millis());
    if (!keyboardReady) {
        return;
    }
    bool echoChanged = false;
    for (const KeyboardReport& report : keyboard.update(pressed)) {
        // The echo follows only what reached the iPhone.
        if (sendKeyboardReport(report)) {
            echoChanged |= echo.apply(report);
            lastReportMs = millis();
        }
    }
    // Only with every key up: a held key (e.g. the iOS accent popup) is not a pause.
    if (!echo.text().empty() && keyboard.report() == KeyboardReport{} && millis() - lastReportMs >= kEchoIdleMs) {
        echo.clear();
        echoChanged = true;
    }
    if (echoChanged && displayReady) {
        drawEcho();
    }
    if (keyboard.takeEject() && !ble::sendConsumerUsage(kEjectUsage)) {
        logMessage(LogLevel::Warn, "HID", "eject not sent");
    }
}

void pollReadingMode(const std::vector<KeyPos>& pressed) {
    if (pairingResetCombo.update(pressed, millis()) && ble::resetPairing()) {
        if (displayReady) {
            drawStatus();
        }
        logMessage(LogLevel::Info, "SYS", "restarting after pairing reset");
        Serial.flush();
        delay(kResetNoticeMs);
        ESP.restart();
    }
    bool newPress = false;
    for (const KeyEvent& event : keyTracker.update(pressed)) {
        char text[kKeyTextCapacity];
        formatKeyEvent(text, sizeof(text), event);
        logMessage(LogLevel::Debug, "KEY", "%s", text);
        newPress |= event.kind == KeyEventKind::Down;
    }
    if (newPress && displayReady) {
        char label[kKeyTextCapacity];
        formatKeyLabel(label, sizeof(label), keyTracker.lastPressed().key, keyTracker.lastPressed().modifiers);
        drawKeyLabel(label);
    }
}

void pollG0(const std::vector<KeyPos>& pressed) {
    if (M5Cardputer.BtnA.wasPressed()) {
        logMessage(LogLevel::Debug, "BTN", "G0 down");
        setKeyboardMode(!keyboardMode, pressed);
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
        drawHello();
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

    const int32_t battery = M5Cardputer.Power.getBatteryLevel();
    ble::begin(static_cast<uint8_t>(battery < 0 ? 0 : (battery > 100 ? 100 : battery)));
}

void loop() {
    M5Cardputer.update();
    const bool bleChanged = ble::poll();
    const bool ancsChanged = ancs::poll();
    if ((bleChanged || ancsChanged) && displayReady) {
        drawStatus();
    }
    const std::vector<KeyPos> pressed = readPressedKeys();
    updateKeyboardReady(pressed);
    pollG0(pressed);
    if (keyboardMode) {
        pollKeyboardMode(pressed);
    } else {
        pollReadingMode(pressed);
    }
    delay(10);
}
