#pragma once

#include <stdint.h>

#include <string>

#include "core/display_text.h"

namespace ui {

constexpr int kStatusHeight = 18;
constexpr int kMargin = 4;
constexpr uint16_t kSelectedBackground = 0x2945;
// Chat screen and Settings screen: a title, then one line per kLineHeight.
constexpr int kLineHeight = 16;
constexpr int kChatTextTop = kStatusHeight + kLineHeight + 2;

// The display's text width in its current font.
TextWidth displayWidth();

// A Message's text as previewed: in groups it starts with its sender.
std::string withSender(bool group, const std::string& sender, const std::string& text);

// The clear prompt or its notice when shown for chat; nullptr otherwise.
const char* clearPromptText(const std::string& chat);

}  // namespace ui
