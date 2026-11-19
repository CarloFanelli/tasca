#pragma once

#include <stdint.h>

#include <vector>

#include "core/chat_view.h"
#include "core/clear_prompt.h"
#include "core/line_scroll.h"
#include "core/text_entry.h"
#include "screens/reading_screen.h"

// The public channel's text messages heard by the LoRa unit, oldest to newest, and a line to write one. Opened from
// the Menu; ` goes back.
class MeshScreen : public ReadingScreen {
public:
    MeshScreen();

    void open();
    // Rebuilds the lines from the store and clamps the scroll to them.
    void layout();
    // The Mesh screen on display shows every message: they are read.
    void markRead();

    // While the compose line is open, keys type in it.
    void handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) override;
    // The ages shown advance once a minute; the clear prompt goes after a while.
    void update(uint32_t nowMs) override;
    void draw() override;
    // On top, also back from the Node list: what arrived meanwhile is now on display.
    void onShown() override;

protected:
    void onKeyDown(const KeyEvent& event) override;

private:
    // Lines of messages that fit: one less while the compose line is open.
    size_t listLines() const;
    void openCompose();
    void closeCompose();
    void send();
    void drawCompose();
    void clearMessages();

    std::vector<ChatLine> lines_;
    // The keys held this loop, for openCompose().
    std::vector<KeyPos> pressed_;
    bool composing_ = false;
    TextEntry entry_;
    // The last Enter was refused: the radio was on air 10% of the last hour.
    bool radioLimit_ = false;
    LineScroll scroll_;
    // Del twice clears every Mesh message.
    ClearPrompt clearPrompt_;
    uint32_t laidOutMs_ = 0;
};

extern MeshScreen meshScreen;
