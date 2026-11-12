---
feature: reading-screens
spec-version: 1.2
status: approved
---

# Architecture Plan: Reading Screens

## Technical Approach

### Overview

Screen logic that needs no display lives in `src/core/` and is host-tested:
- `inbox` derives the Chat list from the store;
- `display_text` turns UTF-8 into what the efont can show and wraps or cuts it with an injected width function;
- `reading_nav` holds screen, selection and scroll.

`main.cpp` keeps the drawing, as today (`drawStatus`, `drawEcho` in `src/main.cpp:54-106`), and dispatches keys to `reading_nav` in `pollReadingMode` (`src/main.cpp:208`). The Hello screen and the key label go away. The store stays owned by `ancs_client`. Marking read from the Cardputer goes through a new `ancs` call, so `loop()` remains its only writer (CLAUDE.md concurrency rules).

### Shared Concerns

#### Draw straight to the display, no full-screen sprite

- **Appears in**: Task 01, Task 02, Task 04
- **Reference example**: `drawStatus()` in `src/main.cpp:71`, which uses `fillRect` on its band, then draws.
- **Rule**: each band (status bar, list, Banner) clears and redraws its own rectangle on a dirty flag. A 240×135 16-bit sprite costs about 64 KB, which is more than the store's whole budget with no PSRAM.

#### Text goes through `display_text`

- **Appears in**: Task 01, Task 02, Task 04
- **Reference example**: `trimIncompleteUtf8` in `src/core/utf8.cpp`.
- **Rule**: every Chat name, sender and text drawn is first passed through the □ substitution. Fitting is computed with the display's `textWidth` passed as a function, so tests can use a fixed-width fake.

#### Fonts: efontCN for content, Font2 for chrome

- **Appears in**: Task 01, Task 02, Task 04
- **Reference example**: `src/main.cpp:59`, which already uses `efontCN_16` for UTF-8.
- **Rule**: Message content is drawn with efontCN (12 or 16). Fixed English labels use Font2.

## Per-Task Technical Guidance

### Task 01: status-bar-inbox

**Files to create:**
- `src/core/display_text.{h,cpp}`:
  - substitutes □ for each codepoint above U+00FF;
  - swallows emoji modifiers (U+FE0F, U+200D, U+1F3FB–1F3FF) so a composed emoji is one □;
  - cuts with "…" to a width;
  - pattern: `src/core/echo_line.cpp`.
- `src/core/inbox.{h,cpp}`:
  - builds the Chats from `MessageStore::messages()`: name, last Message, its sender, unread count, group flag (sender ≠ Chat);
  - sorted by newest `date` (lexically sortable), ties by store position.
- `src/core/reading_nav.{h,cpp}`:
  - screen and selection, kept by Chat name rather than index;
  - up/down with clamping;
  - pattern: `src/core/combo_hold.*`.
- Tests: `test/test_display_text`, `test/test_inbox`, `test/test_reading_nav`, with synthetic names.

**Files to modify:**
- `src/main.cpp`:
  - remove `drawHello`/`drawKeyLabel`; add a status bar (top band) and an Inbox list.
  - In `pollReadingMode`, map key presses from `keyTracker` events: `;` (2,11), `.` (3,11), Enter (2,13), `` ` `` (0,0), ignoring the Fn modifier. Keep the KEY Debug log and the Fn+Del combo.
  - Battery: `M5Cardputer.Power.getBatteryLevel()` (`src/main.cpp:277`), sampled every 30 s, redrawn only on change.

**Key decisions:**
- Rebuilding the Chat list on every store change rather than keeping an index: 200 Messages are cheap to scan, and nothing goes stale.

**Watch out for:**
- efontCN may lack the □ glyph (U+25A1) — `src/main.cpp:59` → check on hardware in this task; if missing, draw a small rectangle where the substitute stands; test: none (glyph data).

---

### Task 02: chat-screen

**Files to modify:**
- `src/core/message_store.{h,cpp}`: mark every Message of a Chat read, returning how many changed. Pattern: `markRead` at `src/core/message_store.cpp:56`. Add a test in `test/test_message_store`.
- `src/ble/ancs_client.{h,cpp}`: a non-const call that marks a Chat read and logs the counts (`logCounts`, `src/ble/ancs_client.cpp:97`).
- `src/core/display_text.*`: word wrap to a width, breaking inside words longer than a line.
- `src/core/reading_nav.*`: Chat screen state, scroll line, clamping, and the back key.
- `src/main.cpp`: draw the Chat screen from the wrapped lines; `HH:MM` from `date` characters 9–12.

**Key decisions:**
- Read on the Cardputer reuses the existing `read` flag: status, Inbox and a later Removed event agree without a second flag.

**Watch out for:**
- A Removed event for a Message read on the Cardputer: `markRead` must return false and leave the counts unchanged (`src/core/message_store.cpp:58`). Test: read by Chat, then `markRead(uid)` returns false.
- Wrapping 200 Messages on every draw is slow. Wrap only the open Chat's Messages, once per content change, and keep the lines. Test: none (timing).

---

### Task 03: live-updates

**Files to modify:**
- `src/main.cpp`, where `ancsChanged` is used (`src/main.cpp:284`):
  - rebuild the Inbox and redraw the current screen;
  - with a Chat screen open, mark that Chat read again;
  - re-wrap it, clamp the scroll, and fall back to the Inbox when the Chat is gone.
- `src/core/reading_nav.*`: re-find the selection by Chat name after a rebuild. When that Chat is gone, select the nearest index, clamped.
- Tests: `test/test_reading_nav` for the selection following a reorder, a vanished Chat and scroll clamping.

**Watch out for:**
- `ancs::poll()` returns true for state changes too (`src/ble/ancs_client.cpp:267`). The connection flood (~110 Messages in 11 s) would redraw the list on every packet. Redraw at most once per loop, not per Message. Test: none (hardware: flood stays under 20 s).

---

### Task 04: keyboard-context-banner

**Files to modify:**
- `src/core/message_store.{h,cpp}`: a monotonic count of Messages added live (`Added` with `preExisting` false: reconnection imports and updates never raise a Banner), so `main` can detect an arrival and show `messages().back()`. After an add, the newest Message is always the back (`src/core/message_store.cpp:51-52`). Test in `test/test_message_store`.
- `src/main.cpp`:
  - `setKeyboardMode` (line 155) keeps `reading_nav` state untouched and redraws the Reading screen on exit;
  - the area above the echo line shows the Chat's last Message, from the shared `display_text` helpers;
  - the Banner is a 3 s timer, restarted by each arrival and cleared by G0.

**Watch out for:**
- The echo line redraw (`drawEcho`, `src/main.cpp:54`) clears only its band. When the Banner ends, the context area must redraw, or the Banner text stays on screen. Check on hardware.

---

### Task 05: mirror-iphone-removals

**Files to modify:**
- `src/core/message_store.{h,cpp}` — a removal by UID and a removal of every unlinked Message replace mark-read by UID (`src/core/message_store.cpp:56`); tests in `test/test_message_store`.
- `src/ble/ancs_client.cpp` — a Removed event deletes (`handleEvent`, line 180); the import tracker starts at subscription, fails on timeout, malformed or dropped answers and full queue, is cancelled on disconnection; on its end the unlinked Messages are removed.

**Files to create:**
- `src/core/reconnect_import.{h,cpp}` — when the PreExisting import has ended (no request waiting, 2 s without a PreExisting event) and whether it was complete; pattern: `src/core/combo_hold.*`. Test: `test/test_reconnect_import`.

**Watch out for:**
- A Message whose answer was lost looks gone from the iPhone — `src/ble/ancs_client.cpp:sendNextRequest` timeout → any failure during the import skips the cleanup; test: failure finishes without cleanup.
