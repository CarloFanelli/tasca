---
feature: screen-stack
spec-version: 1.0
status: approved
---

# Architecture Plan: Screen Stack

## Technical Approach

### Overview

A hardware-free `Screen` interface and `ScreenStack` go in `src/core/` (host-tested); the four screens become hardware classes under a new `src/screens/` folder, each moving its existing draw and key code from `src/main.cpp` verbatim. State several screens share (settings, `ReadingNav`, Inbox, dirty flags, screen timer, beeps) moves to `src/app.h/.cpp`; cross-screen services move beside it: status bar and draw helpers in `src/ui/`, Siri in `src/siri.h/.cpp`. `main.cpp` keeps `setup()`, `loop()`, G0 and the dispatch to the stack's top. Code moves, it is not rewritten: every behavior change is a defect.

### Shared Concerns

#### Move, don't rewrite

- **Appears in**: Task 01, Task 02, Task 03
- **Reference example**: `src/main.cpp` (current functions are the reference behavior)
- **Rule**: each function moves with its body and comments unchanged; only the state it reads changes address (`app::…`). A fix met on the way goes in the backlog, not in the move.

#### Screen interface

- **Appears in**: Task 01, Task 02, Task 03
- **Reference example**: `src/core/siri_press.h` (core class shape), `test/test_siri_press/test_main.cpp` (tests)
- **Rule**: a screen receives the pressed snapshot and its key events (Keyboard mode needs the snapshot, `main.cpp:624`; Reading screens the events, `main.cpp:735`), an update with the time, a draw, and shown/covered hooks; core types only (`core/key_events.h`).

## Per-Task Technical Guidance

### Task 01: reading-screens-stack

**Files to create:**
- `src/core/screen.h`, `src/core/screen_stack.h/.cpp`, `test/test_screen_stack/test_main.cpp` — push (no-op when already on top), pop (never the bottom), remove a covered screen, top; hooks called on shown/covered; tests with fake screens.
- `src/screens/inbox_screen`, `chat_screen`, `settings_screen` (.h/.cpp) — from `drawInbox`, `drawChat`, `drawSettings`, `pollReadingMode`, `pollSettingsKey`.
- `src/app.h/.cpp` — shared state of `main.cpp:67-111`, `wakeScreen`, `onMessagesChanged`, `pollArrivals`, beeps.
- `src/ui/status_bar`, `src/ui/draw` (.h/.cpp) — `drawStatus`, `fitWidth` wrappers, `withSender`, `drawMessageBox`.
- `src/siri.h/.cpp` — `main.cpp:546-622`.

**Key decisions:**
- "Back on top" hook replaces the scattered `readOpenChat()` calls (`main.cpp:393`, `:541`, `:690`).
- Covered Chat emptied: `onMessagesChanged` removes the Chat screen from the stack.
- `src/screens/` compiles only in `cardputer`: native `build_src_filter` (`platformio.ini`) keeps `core/` only.

**Watch out for:**
- Wake key must do nothing else in Reading screens but still reach the iPhone in Keyboard mode — `main.cpp:630` vs `:745` → keep the wake check inside each screen's key handling, not in the dispatcher.
- Siri ` window spans all screens — `main.cpp:633`, `:748` → check it once in the dispatcher, before the top screen, after the wake check.

---

### Task 02: keyboard-mode-screen

**Files to create:**
- `src/screens/keyboard_screen.h/.cpp` — `drawContext`, `drawMode`, `drawEcho`, Banner, `pollKeyboardMode`, `setKeyboardMode`, `updateKeyboardReady`.

**Files to modify:**
- `src/main.cpp` — G0 pushes or pops the Keyboard mode screen (`pollG0`, `:796`).
- `src/app.h/.cpp` — `contextDirty` stays for the partial redraw of the context area.
- `src/screens/settings_screen.cpp` — covered hook cancels a waiting prompt.

**Key decisions:**
- `updateKeyboardReady` runs every loop whatever the top screen, as today (`main.cpp:867`): exposed by the Keyboard mode module, called from `loop()`.
- `keyboardMode` becomes "Keyboard mode screen on top", read by the status bar.

**Watch out for:**
- Leaving Keyboard mode must release held keys on the iPhone — `main.cpp:530-533` → keep it in the covered/closed path; test on device with a key held during G0.
- Banner only when Keyboard mode is on top — `main.cpp:425` → query the stack, not a flag.

---

### Task 03: slim-main-checks

**Files to modify:**
- `src/main.cpp` — down to `setup()`, `loop()`, G0, key dispatch, `psramStatus`; at most 300 lines.
- `.ai/features/screen-stack/tasks/03-slim-main-checks.md` — regression checklist and outcomes in its notes.

**Key decisions:**
- Flash baseline: 2,013,903 bytes (`pio run -e cardputer`, before Task 01).
- Checklist derives from the README key tables and the spec edge cases; run with a serial capture as in siri-reply.
