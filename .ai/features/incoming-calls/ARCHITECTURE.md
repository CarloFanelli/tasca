---
feature: incoming-calls
spec-version: 1.0
status: approved
---

# Architecture Plan: Incoming Calls

## Technical Approach

### Overview

Calls ride the existing ANCS client: the IncomingCall category (1) is requested like Social, but its attributes go to a host-tested call tracker in `src/core/` instead of the Message store, and a Removed event ends the call. The Call screen is a new screen on the Screen stack (`src/screens/call_screen`), opened over whatever is on top when a call starts ringing and closed when none rings, so "back where you were" comes from the stack (`src/core/screen_stack.h`). Answer and decline are the ANCS Perform Notification Action command (CommandID 2, ActionID 0 positive / 1 negative), written to the Control Point from the main loop like attribute requests. No GATT table change and no new library.

### Shared Concerns

None: every pattern appears in at most two tasks.

## Per-Task Technical Guidance

### Task 01: call-screen

**Files to create:**
- `src/core/incoming_call.h/.cpp` + `test/test_incoming_call/test_main.cpp` — ringing calls by UID (newest shown), title, action flags, double-beep schedule; pattern `src/core/siri_press.h`, tests like `test/test_siri_press`.
- `src/screens/call_screen.h/.cpp` — draw, keys ignored, ring in `update`; pattern `src/screens/settings_screen.cpp`.

**Files to modify:**
- `src/core/ancs_request.h/.cpp` — `AncsEvent` gains PositiveAction (1<<3) and NegativeAction (1<<4), beside PreExisting (`ancs_request.cpp:13`).
- `src/ble/ancs_client.h/.cpp` — accept category 1 (`:47`); call requests jump the queue (`:220`) and carry their category; `handleAttributes` (`:118`) hands a call's title to the tracker whatever the app; Removed (`:193`) and `onDisconnected` (`:297`) end calls; read access to the shown call.
- `src/main.cpp` — open/close the Call screen from the tracker each loop; no Message beep or Banner (`:54`) and no display timeout (`:169`) while it is open.
- `src/siri.h/.cpp` — release a held Voice Command on demand.
- `src/screens/keyboard_screen.h/.cpp`, `src/ui/status_bar.cpp:37` — see Watch out.

**Key decisions:**
- The Call screen opens on the Added event; the name fills in when attributes arrive.
- Status bar mode stays "Keyboard" while Keyboard mode is open under the Call screen.

**Watch out for:**
- A call stored as a Message breaks import linking — `ancs_client.cpp:118` → route by category before `store.receive`; test: a call never changes `messages()`.
- Keys held in Keyboard mode stay down on the iPhone — `keyboard_screen.cpp:110` → covered hook sends an empty report and cancels Siri; on return reset the mapper with the pressed keys (`:136`).
- Caller name logged like Message text — `ancs_client.cpp:118` → log only uid and event.

---

### Task 02: answer-decline

**Files to modify:**
- `src/core/ancs_request.h/.cpp` — Perform Notification Action encoder (6 bytes: 2, UID little-endian, ActionID) beside `encodeGetNotificationAttributes`; tests in `test/test_ancs_request`.
- `src/core/incoming_call.h/.cpp` — may-act by flag, acted and failed states; tests in `test/test_incoming_call`.
- `src/ble/ancs_client.h/.cpp` — loop-only call action, written with response like `sendNextRequest` (`:238`), refusal logged.
- `src/screens/call_screen.cpp` — Enter and `, hint line by flags, "Failed — use the iPhone".

**Patterns to reuse:**
- Refused write logging: `ancs_client.cpp:238-243`.

**Key decisions:**
- The action targets the shown call, so call waiting acts on the newest.
- The write is not queued behind attribute requests: it gets no Data Source reply.

**Watch out for:**
- An action written while an attribute response is in flight — the spec sends no Data Source reply for it, so `response` stays untouched; verify on the iPhone during an import.

---

### Task 03: iphone-verification

**Files to modify:**
- `README.md` — a "Calls" section after "Siri": keys, which calls work, audio stays on the iPhone.
- `.ai/features/incoming-calls/tasks/03-iphone-verification.md` — outcome per kind of call in its notes.

**Key decisions:**
- Serial capture as in siri-reply; the log shows call events and actions, never names.
