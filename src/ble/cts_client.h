#pragma once

#include "core/wall_clock.h"

class NimBLEClient;

// Current Time Service client: the iPhone's time of day, read once per connection.
namespace cts {

// Blocking read on the connection's GATT client: loop() only. A failed read leaves the clock as it was.
void setUp(NimBLEClient* client);
// The time of day last read, kept running until restart.
const WallClock& clock();

}  // namespace cts
