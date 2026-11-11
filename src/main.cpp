#include <M5Cardputer.h>

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);

    auto& display = M5Cardputer.Display;
    display.fillScreen(TFT_BLACK);
    display.setTextDatum(middle_center);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setFont(&fonts::FreeSansBold18pt7b);
    display.drawString("Hello", display.width() / 2, display.height() / 2 - 14);
    display.setFont(&fonts::Font2);
    display.drawString("Tasca v" TASCA_VERSION, display.width() / 2, display.height() / 2 + 22);
}

void loop() {
    M5Cardputer.update();
    delay(10);
}
