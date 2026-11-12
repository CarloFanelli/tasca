---
feature: reading-screens
status: APPROVED
reviewed: 2026-11-12
head: 47a84f80975d47f3ba0cd26de0865e042a3c5a44
---

# Review — reading-screens

**Scope**: 28 files (delta since review head `47a84f80`, nothing committed since, so the whole feature diff; 12 workflow artifacts under `.ai/` excluded)
**Findings**: 0 must-fix · 0 should-fix · 0 nit

## Findings

### F-1 · should-fix · bug · Banner can show an old Message instead of the new one   [ADDRESSED]
**File**: `src/main.cpp:308`
**Issue**: The Banner takes `messages().back()` whenever the live count grew, but one `ancs::poll()` can store a live Message and then a PreExisting one (reconnection flood), so the Banner shows the PreExisting Message.
**Change**: Have the store remember the UID of the last Message added live next to `liveAdded_` (`src/core/message_store.cpp`, in `receive`) and build the Banner from the Message linked to that UID; add the mixed live-then-PreExisting case to `test/test_message_store/test_main.cpp`.
**Why**: REQ-12 and the task 04 edge case: Messages fetched at reconnection never raise a Banner.

### F-2 · nit · dead · Reading mode branch of the mode line is unreachable   [ADDRESSED]
**File**: `src/main.cpp:165-166`
**Issue**: `drawMode()` is now called only in Keyboard mode (`drawScreen`, `updateKeyboardReady`), so its "Reading mode" text and grey color are never drawn.
**Change**: Drop the Reading defaults and pick only between the two Keyboard mode texts and colors, as the status bar does at `src/main.cpp:138`.

### F-3 · nit · dup · group rule repeated in three places   [ADDRESSED]
**File**: `src/core/inbox.cpp:34`, `src/core/chat_view.cpp:25`, `src/main.cpp:310`
**Issue**: "a Message belongs to a group when its sender is not its Chat" is written three times.
**Change**: Add one helper beside the Chat rule in `src/core/chat_key.h` and call it from the three places.
