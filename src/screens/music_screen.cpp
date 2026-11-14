#include "screens/music_screen.h"

#include <M5Cardputer.h>

#include <string>
#include <vector>

#include "app.h"
#include "ble/ams_client.h"
#include "ui/draw.h"

using namespace ui;

MusicScreen musicScreen;

namespace {

// Title on two lines, artist on one, then play state and time, then the progress bar.
constexpr int kTitleTop = kChatTextTop + 2;
constexpr int kArtistTop = kTitleTop + 2 * kLineHeight + 2;
constexpr int kTimeTop = kArtistTop + kLineHeight + 6;
constexpr int kBarTop = kTimeTop + kLineHeight + 2;
constexpr int kBarHeight = 5;
constexpr int kHintsTop = kBarTop + kBarHeight + 3;

struct KeyHint {
    KeyPos key;
    const char* text;
};
const KeyHint kHints[] = {
    {app::kPrevKey, ",prev"}, {app::kOpenKey, "Enter play"}, {app::kNextKey, "/next"},
    {app::kUpKey, ";vol+"},   {app::kDownKey, ".vol-"},
};

// Nothing playing: Enter asks the player to start and iOS decides.
AmsCommand enterCommand() {
    return ams::nowPlaying().title().empty() ? AmsCommand::Play : AmsCommand::TogglePlayPause;
}

// The command of a key on the Music screen; false for any other key.
bool commandOf(KeyPos key, AmsCommand* command) {
    if (app::isKey(key, app::kOpenKey)) {
        *command = enterCommand();
    } else if (app::isKey(key, app::kPrevKey)) {
        *command = AmsCommand::PreviousTrack;
    } else if (app::isKey(key, app::kNextKey)) {
        *command = AmsCommand::NextTrack;
    } else if (app::isKey(key, app::kUpKey)) {
        *command = AmsCommand::VolumeUp;
    } else if (app::isKey(key, app::kDownKey)) {
        *command = AmsCommand::VolumeDown;
    } else {
        return false;
    }
    return true;
}

const char* hintText(const KeyHint& hint) {
    const bool pause = ams::nowPlaying().state() == PlaybackState::Playing;
    return app::isKey(hint.key, app::kOpenKey) && pause ? "Enter pause" : hint.text;
}

// One hint per key, spread over the bottom line; a key whose command the player does not offer is dimmed.
void drawHints() {
    auto& display = M5Cardputer.Display;
    display.setFont(&fonts::efontCN_12);
    display.setTextDatum(top_left);
    int textWidth = 0;
    for (const KeyHint& hint : kHints) {
        textWidth += display.textWidth(hintText(hint));
    }
    constexpr int kHintCount = sizeof(kHints) / sizeof(kHints[0]);
    const int gap = (display.width() - 2 * kMargin - textWidth) / (kHintCount - 1);
    int left = kMargin;
    for (const KeyHint& hint : kHints) {
        AmsCommand command;
        commandOf(hint.key, &command);
        const bool usable = ams::available() && ams::offered(command);
        display.setTextColor(usable ? TFT_WHITE : TFT_DARKGREY, TFT_BLACK);
        display.drawString(hintText(hint), left, kHintsTop);
        left += display.textWidth(hintText(hint)) + gap;
    }
}

const char* stateText(PlaybackState state) {
    switch (state) {
        case PlaybackState::Playing:        return "Playing";
        case PlaybackState::Rewinding:      return "Rewinding";
        case PlaybackState::FastForwarding: return "Fast forward";
        case PlaybackState::Paused:         break;
    }
    return "Paused";
}

void drawCentered(const char* text) {
    auto& display = M5Cardputer.Display;
    display.setFont(&fonts::Font2);
    display.setTextDatum(middle_center);
    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.drawString(text, display.width() / 2, (kChatTextTop + display.height()) / 2);
}

// Two lines at most; a longer title ends with "…".
void drawTitle(const std::string& title, int width) {
    auto& display = M5Cardputer.Display;
    std::vector<std::string> lines = wrapText(displayable(title, true), width, displayWidth());
    if (lines.size() > 2) {
        lines[1] = fitWidth(lines[1] + " " + lines[2] + kEllipsis, width, displayWidth());
        lines.resize(2);
    }
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    for (size_t i = 0; i < lines.size(); i++) {
        display.drawString(lines[i].c_str(), kMargin, kTitleTop + static_cast<int>(i) * kLineHeight);
    }
}

}  // namespace

void MusicScreen::open() {
    app::screens.open(this);
    app::screenDirty = true;
}

void MusicScreen::update(uint32_t nowMs) {
    ReadingScreen::update(nowMs);
    if (ams::available() && ams::nowPlaying().elapsedS(nowMs) != shownElapsedS_) {
        app::screenDirty = true;
    }
}

void MusicScreen::draw() {
    auto& display = M5Cardputer.Display;
    const NowPlaying& playing = ams::nowPlaying();
    shownElapsedS_ = playing.elapsedS(millis());
    display.fillRect(0, kStatusHeight, display.width(), display.height() - kStatusHeight, TFT_BLACK);
    const int width = display.width() - 2 * kMargin;

    display.setFont(&fonts::efontCN_16);
    display.setTextDatum(top_right);
    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    int titleWidth = width;
    if (ams::available() && playing.volumePercent() >= 0) {
        const std::string volume = "Vol " + std::to_string(playing.volumePercent()) + "%";
        display.drawString(volume.c_str(), display.width() - kMargin, kStatusHeight);
        titleWidth -= display.textWidth(volume.c_str()) + kMargin;
    }
    const std::string player = ams::available() && !playing.player().empty() ? playing.player() : "Music";
    display.setTextDatum(top_left);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.drawString(fitWidth(displayable(player, true), titleWidth, displayWidth()).c_str(), kMargin,
                       kStatusHeight);
    display.drawFastHLine(0, kChatTextTop - 1, display.width(), TFT_DARKGREY);

    drawHints();
    if (!ams::available()) {
        drawCentered("iPhone not connected");
        return;
    }
    if (playing.title().empty()) {
        drawCentered("Nothing playing");
        return;
    }
    drawTitle(playing.title(), width);
    display.setTextColor(TFT_CYAN, TFT_BLACK);
    display.drawString(fitWidth(displayable(playing.artist(), true), width, displayWidth()).c_str(), kMargin,
                       kArtistTop);

    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.drawString(stateText(playing.state()), kMargin, kTimeTop);
    std::string time = formatPlayTime(shownElapsedS_);
    // Live radio: no duration, so no bar.
    if (playing.durationS() > 0) {
        time += " / " + formatPlayTime(playing.durationS());
        display.fillRect(kMargin, kBarTop, width, kBarHeight, TFT_DARKGREY);
        display.fillRect(kMargin, kBarTop, static_cast<int>(static_cast<uint64_t>(width) * shownElapsedS_ / playing.durationS()),
                         kBarHeight, TFT_GREEN);
    }
    display.setTextDatum(top_right);
    display.drawString(time.c_str(), display.width() - kMargin, kTimeTop);
}

void MusicScreen::handleKeys(const std::vector<KeyPos>& pressed, const std::vector<KeyEvent>& events) {
    ReadingScreen::handleKeys(pressed, events);
    if (volumeRepeat_.update(pressed, millis())) {
        ams::send(app::isKey(volumeRepeat_.key(), app::kUpKey) ? AmsCommand::VolumeUp : AmsCommand::VolumeDown);
    }
}

void MusicScreen::onKeyDown(const KeyEvent& event) {
    if (app::isKey(event.key, app::kBackKey)) {
        app::screens.remove(this);
        app::screenDirty = true;
        return;
    }
    AmsCommand command;
    if (!commandOf(event.key, &command)) {
        return;
    }
    // A key the player does not offer does nothing; without AMS, send() says why.
    if (ams::available() && !ams::offered(command)) {
        return;
    }
    ams::send(command);
    if (ams::available() && (command == AmsCommand::VolumeUp || command == AmsCommand::VolumeDown)) {
        volumeRepeat_.start(event.key, millis());
    }
}
