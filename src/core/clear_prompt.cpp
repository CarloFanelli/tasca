#include "core/clear_prompt.h"

bool ClearPrompt::del(const std::string& chat, uint32_t nowMs) {
    if (shown_ && chat == chat_) {
        shown_ = false;
        return true;
    }
    shown_ = true;
    notice_ = false;
    chat_ = chat;
    sinceMs_ = nowMs;
    return false;
}

void ClearPrompt::showNotice(uint32_t nowMs) {
    shown_ = false;
    notice_ = true;
    sinceMs_ = nowMs;
}

bool ClearPrompt::cancel() {
    const bool was = shown_ || notice_;
    shown_ = false;
    notice_ = false;
    return was;
}

bool ClearPrompt::expire(uint32_t nowMs) {
    return (shown_ || notice_) && nowMs - sinceMs_ >= confirmMs_ && cancel();
}
