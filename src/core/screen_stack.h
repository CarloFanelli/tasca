#pragma once

#include <stddef.h>

#include "core/screen.h"

// The screens open, one over another. The bottom screen is always there; only the top one receives
// keys and draws.
class ScreenStack {
public:
    // Every screen open at once fits: a screen is open at most once.
    static constexpr size_t kMaxScreens = 8;

    explicit ScreenStack(Screen* bottom) : screens_{bottom}, count_(1) {}

    // Opens screen over the top one; false when it is already open or the stack is full.
    bool open(Screen* screen);
    // Closes the top screen, back to the one below; false at the bottom.
    bool close();
    // Closes screen wherever it is; false when it is not open or is the bottom.
    bool remove(Screen* screen);

    Screen* top() const { return screens_[count_ - 1]; }
    bool isOpen(const Screen* screen) const;

private:
    Screen* screens_[kMaxScreens];
    size_t count_;
};
