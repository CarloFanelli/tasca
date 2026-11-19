#pragma once

#include <stddef.h>

// The scroll of a screen of lines that grows at the bottom, as the Chat screen.
class LineScroll {
public:
    // Back to a screen just opened.
    void reset();
    // Takes the screen's size: total lines, how many fit, and the line where the newest entry starts. A screen
    // just opened, or showing its last line, shows the newest entry from its start, with earlier lines above it
    // when it is short; otherwise the scroll stays, clamped to the new size.
    void setLines(size_t total, size_t visible, size_t newestStart);
    // By one line; false at the top or bottom.
    bool up();
    bool down();
    // First line to draw.
    size_t top() const { return top_; }

private:
    size_t top_ = 0;
    size_t total_ = 0;
    size_t visible_ = 0;
};
