#pragma once

#include <stdint.h>

namespace ble {

enum class Phase { Advertising, Connected, Secured, PairingFailed, PairingReset };

// Starts the BLE stack, the GATT server and advertising as "Tasca".
void begin(uint8_t batteryLevel);

// Handles the events queued by the BLE host task, then sets up the iPhone's services (ANCS, AMS) once
// the link is secured. Call from loop() only. Returns true when the phase or the ANCS state changed.
bool poll();

// Disconnects and erases every stored bond. Call from loop() only.
// Returns true when a bond was erased: the caller then restarts the board, because iOS does not
// list a device it has just forgotten until that device starts over.
bool resetPairing();

// Asks the iPhone for slow, power-saving link timings (slow) or the usual ones, on the secured
// connection. iOS may grant other values: they are logged. Call from loop() only.
void requestTimings(bool slow);

Phase phase();
// An iPhone is bonded. Call from loop() only.
bool bonded();
// True once after the secured link was lost by signal (supervision timeout), not closed by either side.
// Call from loop() only.
bool takeLinkLost();
// The secured link's signal strength in dBm, read from the controller with no traffic to the iPhone; 0 with
// no secured link or when the read fails. Call from loop() only.
int8_t linkRssi();
const char* phaseName(Phase phase);

}  // namespace ble
