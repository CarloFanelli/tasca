---
feature: siri-reply
status: APPROVED
reviewed: 2026-11-13
head: 400cb1e089af2a71fe909db5e4829ced1c88588a
---

# Review — siri-reply

**Scope**: 6 files (re-verification — delta since review head `400cb1e0`, working tree included; 10 workflow artifacts under `.ai/` excluded)
**Findings**: 0 must-fix · 0 should-fix · 0 nit

## Findings

### F-1 · should-fix · convention · Siri close window has no host test   [ADDRESSED]
**File**: `src/main.cpp:582-598`
**Issue**: The rule "the first key after Fn+Enter ends the window; only a plain ` within 60 s closes Siri" lives in `main.cpp`, so no host test covers it, although it is pure logic stated by an acceptance criterion.
**Change**: Move the window decision (opened at a time, first key-down closes it, true only for ` without modifiers before 60 s, clock-wrap safe) into `src/core/siri_press.h` or a sibling core class with tests, as `src/core/screen_timer.h` does for the display timeout; `main.cpp` keeps only the Esc send.
**Why**: Task 01 acceptance criterion: "The first ` after Fn+Enter, within 60 s, sends Esc to the iPhone and nothing else".

### F-2 · nit · dup · consumer report built twice   [ADDRESSED]
**File**: `src/ble/hid_keyboard.cpp:73-78`
**Issue**: `sendConsumerUsage` rebuilds the same two-byte press report that `setConsumerUsage` builds just above it.
**Change**: Implement `sendConsumerUsage` as `setConsumerUsage(usage)` followed by `setConsumerUsage(0)`, keeping its return value semantics.
