#pragma once

#include <string>

namespace ui {

// Mode on the left, link state after it, unread count and battery on the right.
void drawStatus();

// The battery percent, "--%" when unknown.
std::string batteryLabel();

}  // namespace ui
