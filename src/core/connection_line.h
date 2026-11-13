#pragma once

// The line under "Tasca" on the Connection screen while the link is not secured.
// A failed pairing comes first: the iPhone keeps an old Tasca until it forgets it.
const char* connectionLine(bool bonded, bool connected, bool pairingFailed);
