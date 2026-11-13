#include "core/connection_line.h"

const char* connectionLine(bool bonded, bool connected, bool pairingFailed) {
    if (pairingFailed) {
        return "Forget Tasca on the iPhone";
    }
    return bonded || connected ? "Connecting…" : "Connect your iPhone";
}
