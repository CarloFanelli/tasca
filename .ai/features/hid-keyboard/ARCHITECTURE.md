---
feature: hid-keyboard
spec-version: 1.2
status: approved
---

# Architecture Plan: HID Keyboard

## Technical Approach

### Overview

The HID service already exists in the GATT table (`src/ble/hid_keyboard.cpp:30`), inert: this feature only sends its input reports, never changes the table. Pure logic goes in `src/core/` with host tests, following `src/core/combo_hold.*` and `test/test_combo_hold/`: `keymap` (Cardputer position + Fn layer → HID usages and the 8-byte keyboard report; module name from the Tasca spec §5.2) and `echo_line` (key position + modifiers → typed character, accented letters, line editing). `src/ble/hid_keyboard` gains the sending side (report notify, readiness from the input reports' subscription). `src/main.cpp` owns the mode, routes the pressed-key snapshot either to the Reading-mode path (key log, Fn+Del) or to the keyboard, and draws mode, readiness and echo.

### Shared Concerns

#### Keyboard readiness

- **Appears in**: Task 01, Task 02, Task 04
- **Reference example**: `src/ble/ble_manager.cpp:44` (callbacks only enqueue or set an atomic)
- **Rule**: "ready" = link secured **and** the keyboard input report subscribed by the iPhone; the subscription arrives through a characteristic callback on the host task and is stored atomically, read only from `loop()`.
- **Watch out for**: NimBLE restores the CCCD of a bonded peer with a subscribe event (`ble_gatts.c:2577`, reason RESTORE) and clears it on disconnect — both must update readiness.

## Per-Task Technical Guidance

### Task 01: keyboard-mode

**Files to modify:**
- `src/ble/hid_keyboard.{h,cpp}` — keep the input report characteristics; add send keyboard report / release all, and readiness via `NimBLECharacteristicCallbacks::onSubscribe` on report 1.
- `src/main.cpp` — mode state; `pollG0()` (line 99) toggles it; `pollKeyboard()` (line 71) splits: Reading mode keeps the `KEY` log and `pairingResetCombo`, Keyboard mode builds and sends reports and counts keys; `drawStatus()` (line 36) shows mode and readiness.

**Files to create:**
- `src/core/keymap.{h,cpp}` — position → HID usage (US positions), modifier bits (Alt → GUI), Fn layer (arrows, Esc), accent keys (Fn+vowel, Ctrl+e) as Option dead-key report sequences, report of up to 6 keys; pattern `src/core/key_events.cpp:12` table.
- `test/test_keymap/test_main.cpp` — pattern `test/test_combo_hold/test_main.cpp`.

**Patterns to reuse:**
- `KeyPos`, `kKeyNames` layout in `src/core/key_events.cpp:12-17`; `containsKey` (`src/core/key_events.h:14`).

**Key decisions:**
- Report sent only when its 8 bytes change (press/release diff), from `loop()`; an accent press yields a short sequence of reports, sent in order.
- Keys held at the switch to Keyboard mode are masked until released.
- Leaving Keyboard mode sends an all-zero report before the mode changes.

**Watch out for:**
- `notify()` sends even with no subscriber (`NimBLECharacteristic.cpp:296`, no CCCD check) → gate sending on readiness, not on the BLE phase; test: type before iOS enables the keyboard.
- `pollKeyboard()` logs every key at Debug (`src/main.cpp:89`) → that log must not run in Keyboard mode; test: Debug build, type, grep the log.

---

### Task 02: onscreen-keyboard-toggle

**Files to modify:**
- `src/ble/hid_keyboard.{h,cpp}` — send a consumer report (report 2, 16-bit usage) and its release.
- `src/core/keymap.{h,cpp}` + `test/test_keymap/test_main.cpp` — Fn+Space → Eject (0xB8), not a keyboard key.

**Patterns to reuse:**
- Report map consumer collection in `src/ble/hid_keyboard.cpp:23-25` (logical max 0x3FF covers 0xB8).

**Key decisions:**
- Fired on the press edge only, so a held Fn+Space toggles once.

---

### Task 03: italian-echo-line

**Files to modify:**
- `src/main.cpp` — feed sent key presses to the echo; draw it in the key area (`drawKeyLabel`, line 24) in Keyboard mode, last part of the line when too long.
- `CLAUDE.md` — iPhone setting: Settings > General > Keyboard > Hardware Keyboard > "Automatic" (or U.S.).

**Files to create:**
- `src/core/echo_line.{h,cpp}` — keycap legends (plain, Shift) and accented letters, UTF-8 line with Backspace/Enter, placeholder for hand-typed Option keys; pattern `src/core/log_format.cpp` (UTF-8-safe cutting).
- `test/test_echo_line/test_main.cpp`.

**Key decisions:**
- Font: a built-in U8g2 font with Latin-1 glyphs (an `efont`/`lgfxJapanGothic` size, `M5GFX lgfx_fonts.hpp:424-477`), chosen on the device by drawing "àèéìòù".

---

### Task 04: coexistence-soak

**Files to modify:**
- `src/ble/ancs_client.cpp` — only if the soak shows key latency: attribute writes block `loop()` (`sendNextRequest`, line 157), one at a time.

**Key decisions:**
- A hardware run of 30 minutes with the serial capture; pass = no `disconnected` line, every notification dumped.

**Watch out for:**
- During the connection-time flood `loop()` blocks on each Control Point write (`src/ble/ancs_client.cpp:168`) → keys queue in the keyboard scan; check they arrive in order within 1 s.
