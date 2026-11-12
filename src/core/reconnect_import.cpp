#include "core/reconnect_import.h"

void ReconnectImport::start(uint32_t nowMs) {
    running_ = true;
    failed_ = false;
    lastEventMs_ = nowMs;
}

bool ReconnectImport::finished(uint32_t nowMs, size_t preExistingWaiting) {
    if (!running_ || preExistingWaiting > 0 || nowMs - lastEventMs_ < kQuietMs) {
        return false;
    }
    running_ = false;
    return true;
}
