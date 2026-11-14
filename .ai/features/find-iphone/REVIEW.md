---
feature: find-iphone
status: DRAFT
reviewed: 2026-11-14
head: affe47721650bd37fd4d9c41ed51e45a334a14c4
---

# Review — find-iphone

**Scope**: 28 files (primary — diff since base `e1ed3af4`; 11 workflow artifacts under `.ai/` excluded)
**Findings**: 0 must-fix · 1 should-fix · 1 nit

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
