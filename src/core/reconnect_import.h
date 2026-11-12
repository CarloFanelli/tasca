#pragma once

#include <stddef.h>
#include <stdint.h>

// Tracks the notifications the iPhone resends at each connection (PreExisting). Once every one has
// been answered, Messages it did not resend are gone from the iPhone. A request that failed leaves the
// import incomplete: a Message could then be missing only because its answer was lost.
class ReconnectImport {
public:
    // How long without a new PreExisting event before the import can end.
    static constexpr uint32_t kQuietMs = 2000;

    void start(uint32_t nowMs);
    void cancel() { running_ = false; }
    void onPreExistingEvent(uint32_t nowMs) { lastEventMs_ = nowMs; }
    void onFailure() { failed_ = true; }

    // True once, when the import ends: running, no PreExisting request waiting or in flight,
    // and no PreExisting event for kQuietMs.
    bool finished(uint32_t nowMs, size_t preExistingWaiting);
    // Whether the import that just finished had every request answered.
    bool complete() const { return !failed_; }

private:
    bool running_ = false;
    bool failed_ = false;
    uint32_t lastEventMs_ = 0;
};
