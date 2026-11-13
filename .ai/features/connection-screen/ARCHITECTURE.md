---
feature: connection-screen
spec-version: 1.0
status: approved
---

# Architecture Plan: Connection Screen

## Technical Approach

### Overview

The Connection screen is one more Screen on the Screen stack, opened and closed by a `sync` called every loop right after `ble::poll()`, the way `callScreen.sync` follows the ringing calls (`src/screens/call_screen.cpp:44`). It is open whenever the BLE phase is not Secured, so the same code covers boot (Task 01) and later disconnections (Task 02). The choice of line is a pure function in `src/core/`, tested on the host; the screen draws the whole display itself, battery included, and the main loop skips the status bar while it is on top. No GATT change, no new library.

## Per-Task Technical Guidance

### Task 01: connection-screen

**Files to create:**
- `src/core/connection_line.h/.cpp` — the line text from three facts: bonded, connected, pairing failed. Pattern: `src/core/incoming_call.cpp` `callerName()`.
- `test/test_connection_line/test_main.cpp` — one test per line and per precedence (failed over bonded). Pattern: `test/test_incoming_call/test_main.cpp`.
- `src/screens/connection_screen.h/.cpp` — `sync()`, `handleKeys` (pairing combo only), `draw`. Pattern: `src/screens/call_screen.cpp`.

**Files to modify:**
- `src/ble/ble_manager.h/.cpp` — `bonded()`: `NimBLEDevice::getNumBonds() > 0`, read from loop only.
- `src/ui/status_bar.h/.cpp` — extract the battery label (`status_bar.cpp:41-46`) into a shared function, reused by the Connection screen.
- `src/main.cpp` — call `connectionScreen.sync()` after `ble::poll()`; while it is on top, skip `ui::drawStatus()` and fold `statusDirty` into `screenDirty`; `pollG0` does nothing but wake while it is open (mirror `main.cpp:83`).

**Key decisions:**
- Large "Tasca": a font already linked (`efontCN_16` or `Font2` with a text size) before any new font; measure flash against the 4 KB budget.
- `handleKeys` runs `pairingResetCombo.update(pressed)` and wakes on key down; no KEY log, no Siri.

**Watch out for:**
- Phase Secured can arrive before the Connection screen ever drew (bonded iPhone reconnects fast) — `ble_manager.cpp:143` → `sync` just never opens; test on device with a bonded iPhone near.
- `restartAfterPairingReset` draws the full status bar over the Connection screen — `src/app.cpp:122-125` → acceptable: it shows "pairing reset" for the restart notice.

---

### Task 02: reopen-on-disconnect

**Files to modify:**
- `src/screens/connection_screen.cpp` — on close: if Keyboard mode is back on top, `keyboardScreen.resume(pressed)`, as `call_screen.cpp:58-60`; on open, `cancelSiri()` (`call_screen.cpp:50`). Neither path calls `wakeScreen()`.
- `src/main.cpp` — pass `pressed` to `sync`, so read the keys before it or move `sync` after `readPressedKeys()` but before `callScreen.sync`.

**Patterns to reuse:**
- Settings prompt cancel and Keyboard mode key release come free from `onCovered` (`ScreenStack::open`, `src/core/screen_stack.cpp:7`).
- Chat gone during the disconnection: `onMessagesChanged` already closes it (`src/app.cpp:83-87`).

**Watch out for:**
- Disconnect during a call: `ancs::onDisconnected` clears calls (`ancs_client.cpp:344`), then `callScreen.sync` removes a covered Call screen and calls `wakeScreen()` (`call_screen.cpp:57-62`) → order the two syncs so the Connection screen ends on top; device check.
- `ScreenStack::open` fails when 8 screens are open (`screen_stack.cpp:4`) → today's maximum is 5 (Inbox, Chat, Settings, Keyboard, Call), plus this one: fine; no guard needed.
- Waking the screen on reconnection would break the edge case → `sync` sets only dirty flags; test: reconnection with screen off.

---

### Task 03: readme-device-check

**Files to modify:**
- `README.md` — a short "Connection screen" paragraph in "Using the Cardputer" (`README.md:33`): the three lines, and Fn+Del for a "Connecting…" that never ends; mirror the tone of the pairing paragraph at `README.md:31`.
- `.ai/features/connection-screen/tasks/03-readme-device-check.md` — the checklist and its outcome in the notes.

**Key decisions:**
- Flash measured once more here against the build before Task 01.
