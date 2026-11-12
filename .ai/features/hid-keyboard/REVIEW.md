---
feature: hid-keyboard
status: APPROVED
reviewed: 2026-11-12
head: 0e50a998eea5e6a7ed00aa26dea5b6c572db2216
---

# Review — hid-keyboard

**Scope**: re-verification of the 3 addressed findings over the same 9 files (diff since base `0e50a998`, narrowed to this feature's files; nothing is committed since base, so the diff also carries the already-reviewed firmware-skeleton and ancs-capture work, left out; 24 workflow artifacts under `.ai/` excluded): `CLAUDE.md`, `src/ble/hid_keyboard.{h,cpp}`, `src/core/keymap.{h,cpp}`, `src/core/echo_line.{h,cpp}`, `src/main.cpp`, `test/test_keymap/test_main.cpp`, `test/test_echo_line/test_main.cpp`
**Findings**: 0 must-fix · 0 should-fix · 0 nit

## Findings

### F-1 · should-fix · dup · HID modifier and dead-key constants defined twice   [ADDRESSED]
**File**: `src/core/echo_line.cpp:7-15`
**Issue**: `kHidCtrl`, `kHidShift`, `kHidOption`, `kHidCommand`, `kDeadGrave` and `kDeadAcute` repeat the constants of `src/core/keymap.cpp:13-16,51-52`; the modifier masks even differ (the echo also accepts right-hand bits the mapper never sends), so the two files can drift apart silently.
**Change**: Move the four modifier bits and the two dead-key usages into `src/core/keymap.h` beside `KeyboardReport`, and use them from both `keymap.cpp` and `echo_line.cpp`, dropping the right-hand bits.

### F-2 · nit · dup · UTF-8 continuation test written inline in main   [ADDRESSED]
**File**: `src/main.cpp:65`
**Issue**: The `(c & 0xC0) == 0x80` test repeats `isContinuation` from `src/core/echo_line.cpp:48` (and `src/core/log_format.cpp:18`) to cut the echo at a character boundary.
**Change**: Declare a `bool isUtf8Continuation(char c)` in `src/core/echo_line.h`, define it once in `echo_line.cpp`, and use it in `drawEcho()`.

### F-3 · nit · convention · keyPresses comment no longer true   [ADDRESSED]
**File**: `src/core/keymap.h:33`
**Issue**: The comment says "Key usages pressed since construction, modifiers excluded", but the counter also counts accented letters and Fn+Space Eject presses, which are not single key usages.
**Change**: Reword to "Keys sent since construction (letters, accented letters, Eject), modifiers excluded."
