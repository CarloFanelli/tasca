#include "core/line_scroll.h"

#include <algorithm>

void LineScroll::reset() {
    top_ = 0;
    total_ = 0;
    visible_ = 0;
}

void LineScroll::setLines(size_t total, size_t visible, size_t newestStart) {
    // Just opened (no lines yet) or at the end: new entries come into view from their start.
    const bool atEnd = top_ + visible_ >= total_;
    total_ = total;
    visible_ = visible;
    const size_t last = total > visible ? total - visible : 0;
    top_ = std::min(atEnd ? newestStart : top_, last);
}

bool LineScroll::up() {
    if (top_ == 0) {
        return false;
    }
    top_--;
    return true;
}

bool LineScroll::down() {
    if (top_ + visible_ >= total_) {
        return false;
    }
    top_++;
    return true;
}
