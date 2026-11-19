#pragma once

#include <vector>

#include "screens/reading_screen.h"

// Music, Find iPhone, Mesh, WhatsApp and Settings, opened with Fn+G0 over any screen in either mode; without the
// iPhone only Mesh and Settings open.
class MenuScreen : public ReadingScreen {
public:
    void open();
    // Back to the screen and mode under the Menu; keys held then are not typed in Keyboard mode.
    void close(const std::vector<KeyPos>& pressed);

    void handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) override;
    void draw() override;

protected:
    void onKeyDown(const KeyEvent& event) override;

private:
    // The keys held in the current loop, for close().
    std::vector<KeyPos> pressed_;
};

extern MenuScreen menuScreen;
