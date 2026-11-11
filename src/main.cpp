#include <M5Cardputer.h>
#include <esp_efuse.h>
#include <esp_efuse_table.h>

#include "log.h"

namespace {

constexpr uint32_t kSerialWaitMs = 1500;

void drawHello() {
    auto& display = M5Cardputer.Display;
    display.fillScreen(TFT_BLACK);
    display.setTextDatum(middle_center);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setFont(&fonts::FreeSansBold18pt7b);
    display.drawString("Hello", display.width() / 2, display.height() / 2 - 14);
    display.setFont(&fonts::Font2);
    display.drawString("Tasca v" TASCA_VERSION, display.width() / 2, display.height() / 2 + 22);
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
    const bool displayReady = M5.getDisplayCount() > 0;
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
}

void loop() {
    M5Cardputer.update();
    delay(10);
}
