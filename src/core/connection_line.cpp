#include "core/connection_line.h"

const char* connectionLine(bool bonded, bool connected, bool pairingFailed, bool outOfRange) {
    if (pairingFailed) {
        return "Forget Tasca on the iPhone";
    }
    if (outOfRange) {
        return "iPhone out of range";
    }
    return bonded || connected ? "Connecting…" : "Connect your iPhone";
}
