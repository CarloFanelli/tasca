---
feature: clear-notifications
spec-version: 1.0
status: approved
---

# Architecture Plan: Clear Notifications

## Technical Approach

### Overview

A small core class holds the Del prompt (shown, confirmed, expired), tested on the host like `SettingsNav` (`src/core/settings_nav.h`). On confirmation the Chat's linked UIDs are snapshotted from the `MessageStore` and queued in the ANCS client, which writes one Perform Notification Action (negative) per loop, only while no attribute response is in flight, so keys stay responsive. Messages leave through the existing Removed path (`src/ble/ancs_client.cpp:215`); the Inbox and Chat screen already follow (`ReadingNav::sync`, `app::onMessagesChanged`). Refusals and unlinked Messages are counted per clearing and shown as a notice in the prompt's place.

## Per-Task Technical Guidance

### Task 01: clear-chat

**Files to create:**
- `src/core/clear_prompt.h/.cpp` — prompt for one Chat: Del shows it, second Del within 3 s confirms, cancel, expire. Pattern: `src/core/settings_nav.cpp` `enter`/`expire`.
- `test/test_clear_prompt/test_main.cpp` — confirm, expiry, cancel, wrap-around clock. Pattern: `test/test_settings/`.

**Files to modify:**
- `src/core/message_store.h/.cpp` — linked UIDs of a Chat (and the unlinked count, for Task 02); tests in `test/test_message_store`.
- `src/ble/ancs_client.h/.cpp` — `clearChat(chat)` queues the UIDs; `clearing()`; one write per `poll`, beside `sendNextRequest` (`ancs_client.cpp:256`), with `encodePerformNotificationAction(…, false)`.
- `src/app.h/.cpp` — `kDelKey{0, 13}` and the shared prompt.
- `src/screens/reading_screen.cpp` — any key down other than plain Del cancels the prompt before `onKeyDown`.
- `src/screens/inbox_screen.cpp`, `chat_screen.cpp` — plain Del asks or confirms; draw the prompt in red in place of the selected row's preview / the Chat title (mirror `settings_screen.cpp:96`); `update` expires it.

**Watch out for:**
- Writing an action while a Get Attributes response is decoding mixes the iPhone's answers — `ancs_client.cpp:265` → send clears only when `!response.active()`, same guard.
- Queued UIDs surviving a disconnection clear other notifications next time (UIDs restart) — `ancs_client.cpp:332` → empty the clear queue in `onDisconnected`.
- Del pressed before Fn starts a prompt, then Fn+Del held resets pairing — `app.cpp:34` → accept: the reset restarts the board; nothing is cleared.

---

### Task 02: not-cleared-notice

**Files to modify:**
- `src/ble/ancs_client.cpp` — a refused write logs `W` with `getLastError()` (mirror `ancs_client.cpp:425`) and counts toward the clearing's "not cleared" outcome, with the unlinked count taken at confirmation.
- `src/core/clear_prompt.*` — a notice state shown 3 s, then gone; tests.
- `src/screens/inbox_screen.cpp`, `chat_screen.cpp` — draw "Not cleared on the iPhone" in the prompt's place.

**Key decisions:**
- The notice is raised when the queue empties with at least one refusal or unlinked Message; a removal never received is not detected (REQ-5 edge case).

---

### Task 03: readme-iphone-check

**Files to modify:**
- `README.md` — Del row in the "Reading mode" key table; one sentence in the "Settings" paragraph that "Clear messages" clears the Cardputer only.
- `.ai/features/clear-notifications/tasks/03-readme-iphone-check.md` — device checklist and outcome.
