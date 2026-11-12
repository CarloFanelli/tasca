---
feature: ancs-capture
status: APPROVED
reviewed: 2026-11-12
head: 0e50a998eea5e6a7ed00aa26dea5b6c572db2216
---

# Review — ancs-capture

**Scope**: 23 files (delta since review head `0e50a998`, working tree included — no commit since, so the same files as the first pass; 16 workflow artifacts under `.ai/` excluded)
**Findings**: 0 must-fix · 0 should-fix · 0 nit

## Findings

### F-1 · should-fix · bug · stale events re-queue requests after a disconnection   [ADDRESSED]
**File**: `src/ble/ancs_client.cpp:230`
**Issue**: `queueRequest` runs for every Notification Source packet drained, including packets of a connection that `onDisconnected` has just cleared; their UIDs are requested on the next connection, where iOS restarts UIDs from 0, so they hit an error or another notification.
**Change**: Queue a request only while a Control Point is known (`controlPoint != nullptr`), the same guard `sendNextRequest` uses at `src/ble/ancs_client.cpp:163`.
**Why**: the spec's edge case "Disconnection with requests pending → queue cleared".

### F-2 · nit · convention · pairing reset bypasses the phase log   [ADDRESSED]
**File**: `src/ble/ble_manager.cpp:180`
**Issue**: `resetPairing` assigns `currentPhase` directly, so the change to "pairing reset" is the only phase change without its `phase …` log line.
**Change**: Return `setPhase(Phase::PairingReset)` as every other transition does (see `src/ble/ble_manager.cpp:84`).
**Why**: REQ-14 asks every BLE phase change to be logged.

### F-3 · nit · dup · Added event id defined twice   [ADDRESSED]
**File**: `src/ble/ancs_client.cpp:26`
**Issue**: `kEventIdAdded` repeats `kEventAdded` from `src/core/ancs_request.cpp:6`, and the client reads the event id by hand next to `ancsEventNeedsAttributes`.
**Change**: Expose the Added check from `src/core/ancs_request.h` (beside `ancsEventNeedsAttributes`) and use it for the counter, removing the local constant.
