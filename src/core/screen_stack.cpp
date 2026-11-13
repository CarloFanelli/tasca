#include "core/screen_stack.h"

bool ScreenStack::open(Screen* screen) {
    if (isOpen(screen) || count_ == kMaxScreens) {
        return false;
    }
    top()->onCovered();
    screens_[count_++] = screen;
    screen->onShown();
    return true;
}

bool ScreenStack::close() {
    if (count_ == 1) {
        return false;
    }
    count_--;
    top()->onShown();
    return true;
}

bool ScreenStack::remove(Screen* screen) {
    if (screen == top()) {
        return close();
    }
    for (size_t i = 1; i < count_; i++) {
        if (screens_[i] == screen) {
            for (size_t j = i + 1; j < count_; j++) {
                screens_[j - 1] = screens_[j];
            }
            count_--;
            return true;
        }
    }
    return false;
}

bool ScreenStack::isOpen(const Screen* screen) const {
    for (size_t i = 0; i < count_; i++) {
        if (screens_[i] == screen) {
            return true;
        }
    }
    return false;
}
