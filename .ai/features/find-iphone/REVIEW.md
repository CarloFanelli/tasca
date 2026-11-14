---
feature: find-iphone
status: APPROVED
reviewed: 2026-11-14
head: 1007e02c9f6a4bb9dbcb9c08dfbe30166e348db2
---

# Review — find-iphone

**Scope**: re-verification of `src/ble/ble_manager.cpp` and `src/screens/connection_screen.h`, the 2 files changed since the review of `affe4772` (the other files of the feature were covered by that review and are unchanged since; 1 workflow artifact under `.ai/` excluded)
**Findings**: 0 must-fix · 0 should-fix · 0 nit

## Findings

### F-1 · should-fix · bug · a later disconnection erases a lost link   [ADDRESSED]
**File**: `src/ble/ble_manager.cpp:186`
**Issue**: `linkLost` is assigned on every `Disconnect` event, so when one `ble::poll` drains a supervision timeout of the secured link followed by another disconnection (e.g. a reconnection attempt that fails before it is secured), the second event resets it to false and no Out-of-range alert starts.
**Change**: keep the flag set until `takeLinkLost` reads it — `linkLost = linkLost || (…)` — so only `takeLinkLost` clears it.
**Why**: REQ-3 — a link lost by signal starts the alert.

### F-2 · nit · convention · Connection screen comment says keys do nothing   [ADDRESSED]
**File**: `src/screens/connection_screen.h:10`
**Issue**: the class comment says keys do nothing but the pairing reset, while `handleKeys` now also silences the Out-of-range alert.
**Change**: say that a key also silences the Out-of-range alert.
