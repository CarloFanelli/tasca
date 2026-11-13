---
feature: clear-notifications
status: APPROVED
reviewed: 2026-11-13
head: 400cb1e089af2a71fe909db5e4829ced1c88588a
---

# Review — clear-notifications

**Scope**: re-verification of `src/ble/ancs_client.cpp`, the file of the addressed finding (no commit since the previous review; 12 workflow artifacts under `.ai/` excluded)
**Findings**: 0 must-fix · 0 should-fix · 0 nit

## Findings

### F-1 · must-fix · bug · clearing without ANCS skips the notice   [ADDRESSED]
**File**: `src/ble/ancs_client.cpp:468-470`
**Issue**: when ANCS is not subscribed (the seconds after a reconnection, or notification sharing off), `clearChat` returns before counting, so the second Del consumes the prompt and nothing is shown, although every Message of the Chat is unlinked and stays.
**Change**: count `store.unlinkedCount(chat)` and set `notClearedReady` before the `controlPoint` check, so the notice shows; send nothing when `controlPoint` is null.
**Why**: REQ-7 — Messages not linked stay and "Not cleared on the iPhone" shows for 3 s.
