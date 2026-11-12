#pragma once

#include <functional>
#include <string>
#include <vector>

// Drawn in place of a character the display font has no glyph for: emoji, and anything beyond Latin-1.
constexpr const char* kMissingGlyph = "\xE2\x96\xA1";  // U+25A1 □
constexpr const char* kEllipsis = "\xE2\x80\xA6";      // U+2026 …

// Width in pixels of a UTF-8 string, as the display would draw it.
using TextWidth = std::function<int(const std::string&)>;

// Text the display font can draw: Latin-1 is kept, every other character, or emoji sequence (joined,
// flagged, skin-toned), becomes one kMissingGlyph; invisible format characters and invalid bytes
// are dropped. Line breaks and tabs become spaces when flatten is true.
std::string displayable(const std::string& utf8, bool flatten);

// text split into lines no wider than maxWidth: at line breaks, between words, and inside a word
// longer than a line. Spaces where a line is broken are dropped.
std::vector<std::string> wrapText(const std::string& text, int maxWidth, const TextWidth& width);

// text when it fits maxWidth, else its longest prefix that fits with kEllipsis appended.
std::string fitWidth(const std::string& text, int maxWidth, const TextWidth& width);
