#pragma once

#include <stddef.h>
#include <stdint.h>

#include "screens/reading_screen.h"

// The nodes the LoRa unit heard, the most recent first: names, signal strength and time since last heard. Opened
// with Tab from the Mesh screen; ` goes back to it.
class NodeListScreen : public ReadingScreen {
public:
    void open();

    // The times shown advance once a minute.
    void update(uint32_t nowMs) override;
    void draw() override;

protected:
    void onKeyDown(const KeyEvent& event) override;

private:
    // First node shown.
    size_t top_ = 0;
    uint32_t drawnMs_ = 0;
};

extern NodeListScreen nodeListScreen;
