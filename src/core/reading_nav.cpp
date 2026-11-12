#include "core/reading_nav.h"

#include <algorithm>

void ReadingNav::sync(const std::vector<ChatSummary>& chats) {
    chats_.clear();
    for (const ChatSummary& c : chats) {
        chats_.push_back(c.chat);
    }
    const auto it = std::find(chats_.begin(), chats_.end(), selectedChat_);
    if (it != chats_.end()) {
        select(static_cast<size_t>(it - chats_.begin()));
    } else {
        select(chats_.empty() ? 0 : std::min(index_, chats_.size() - 1));
    }
}

bool ReadingNav::up() {
    if (index_ == 0) {
        return false;
    }
    select(index_ - 1);
    return true;
}

bool ReadingNav::down() {
    if (index_ + 1 >= chats_.size()) {
        return false;
    }
    select(index_ + 1);
    return true;
}

size_t ReadingNav::firstVisible(size_t rows) {
    if (rows == 0) {
        return index_;
    }
    if (index_ < top_) {
        top_ = index_;
    } else if (index_ >= top_ + rows) {
        top_ = index_ - rows + 1;
    }
    // No empty rows at the bottom while earlier Chats are hidden.
    if (chats_.size() >= rows) {
        top_ = std::min(top_, chats_.size() - rows);
    } else {
        top_ = 0;
    }
    return top_;
}

bool ReadingNav::openChat() {
    if (chats_.empty()) {
        return false;
    }
    inChat_ = true;
    openedChat_ = selectedChat_;
    chatTop_ = 0;
    chatTotal_ = 0;
    chatVisible_ = 0;
    return true;
}

void ReadingNav::closeChat() {
    inChat_ = false;
}

void ReadingNav::setChatLines(size_t total, size_t visible, size_t newestStart) {
    // Just opened (no lines yet) or at the end: new Messages come into view from their start.
    const bool atEnd = chatTop_ + chatVisible_ >= chatTotal_;
    chatTotal_ = total;
    chatVisible_ = visible;
    const size_t last = total > visible ? total - visible : 0;
    chatTop_ = std::min(atEnd ? newestStart : chatTop_, last);
}

bool ReadingNav::scrollUp() {
    if (chatTop_ == 0) {
        return false;
    }
    chatTop_--;
    return true;
}

bool ReadingNav::scrollDown() {
    if (chatTop_ + chatVisible_ >= chatTotal_) {
        return false;
    }
    chatTop_++;
    return true;
}

void ReadingNav::select(size_t index) {
    index_ = index;
    selectedChat_ = chats_.empty() ? std::string() : chats_[index];
}
