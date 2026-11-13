#pragma once

#include <stdint.h>

#include <vector>

#include "app.h"
#include "core/message_store.h"
#include "core/screen.h"

// Keyboard mode: every key goes to the iPhone. Opened with G0 over any screen, closed with G0 back to it.
class KeyboardScreen : public Screen {
public:
    void open(const std::vector<KeyPos>& pressed);
    void close();
    // Back on top after a screen above it closed: keys still held are not typed.
    void resume(const std::vector<KeyPos>& pressed);
    // Keys reach the iPhone only on a secured link whose keyboard report the iPhone has enabled. Every loop.
    void updateReady(const std::vector<KeyPos>& pressed);
    // Shows a Message that arrived while typing as the Banner.
    void showBanner(const Message& message);
    // The context Message or the Banner, above the mode line.
    void drawContext();

    void handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) override;
    void update(uint32_t nowMs) override;
    void draw() override;
    // Keys held stay down on the iPhone unless released.
    void onCovered() override;
};

extern KeyboardScreen keyboardScreen;

inline bool inKeyboardMode() {
    return app::screens.top() == &keyboardScreen;
}
