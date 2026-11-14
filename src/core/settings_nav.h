#pragma once

#include <stddef.h>
#include <stdint.h>

// The selected row of the Settings screen, the settings first, then the actions. An action runs
// only when Enter is pressed again on its confirmation prompt.
class SettingsNav {
public:
    // confirmMs: how long the prompt waits for the second Enter.
    SettingsNav(size_t settingRows, size_t actionRows, uint32_t confirmMs)
        : settingRows_(settingRows), rows_(settingRows + actionRows), confirmMs_(confirmMs) {}

    // Moves the selection by one; false at the first or last row.
    bool up();
    bool down();
    size_t selected() const { return selected_; }
    // The selected row is an action.
    bool onAction() const { return selected_ >= settingRows_; }

    // Enter at nowMs, a millisecond clock that may wrap around: on an action, shows the prompt, or
    // returns true when it is already shown, meaning run the action. Does nothing on a setting.
    bool enter(uint32_t nowMs);
    bool confirming() const { return confirming_; }
    // Removes the prompt; false when none was shown.
    bool cancel();
    // Removes a prompt shown for confirmMs or longer; true when it did.
    bool expire(uint32_t nowMs);
    // Back to the first row, with no prompt.
    void reset();
    // The first row shown when visibleRows fit: the list scrolls only as far as the selection needs.
    size_t firstVisible(size_t visibleRows);

private:
    size_t settingRows_;
    size_t rows_;
    uint32_t confirmMs_;
    size_t selected_ = 0;
    size_t firstVisible_ = 0;
    bool confirming_ = false;
    uint32_t promptSinceMs_ = 0;
};
