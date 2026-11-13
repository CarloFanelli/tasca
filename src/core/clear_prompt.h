#pragma once

#include <stdint.h>

#include <string>

// "Press Del again to clear" for one Chat: the first Del shows it, a second Del on the same Chat while
// it is shown clears that Chat. Then "Not cleared on the iPhone" when some of its Messages stayed.
class ClearPrompt {
public:
    // confirmMs: how long the prompt waits for the second Del.
    explicit ClearPrompt(uint32_t confirmMs) : confirmMs_(confirmMs) {}

    // Del at nowMs, a millisecond clock that may wrap around, on chat: shows the prompt, or returns true
    // when it is already shown for chat, meaning clear it.
    bool del(const std::string& chat, uint32_t nowMs);
    bool shown() const { return shown_; }
    // The Chat the prompt is shown for.
    const std::string& chat() const { return chat_; }
    // Shows from nowMs the notice that some Messages of the last cleared Chat stayed.
    void showNotice(uint32_t nowMs);
    bool noticeShown() const { return notice_; }
    // Removes the prompt or the notice; false when none was shown.
    bool cancel();
    // Removes a prompt or notice shown for confirmMs or longer; true when it did.
    bool expire(uint32_t nowMs);

private:
    uint32_t confirmMs_;
    bool shown_ = false;
    bool notice_ = false;
    std::string chat_;
    uint32_t sinceMs_ = 0;
};
