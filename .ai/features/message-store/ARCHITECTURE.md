---
feature: message-store
spec-version: 1.1
status: approved
---

# Architecture Plan: Message Store

## Technical Approach

### Overview

The ANCS link, request queue and response tracking already exist (`src/ble/ancs_client.cpp`, `src/core/ancs_request.*`). The new logic lives in `src/core/` and is host-tested:
- The response tracker grows into a decoder that keeps the attribute values.
- `chat_key` turns title and subtitle into a **Chat** and a sender (module name from the Tasca spec §5.2).
- `message_store` holds the **Messages**.
- A small `utf8` helper is shared by echo, decoding and store.

`ancs_client` stays the only module that talks to the iPhone. It owns the store, feeds it from `loop()` (CLAUDE.md concurrency rules) and exposes counts to `main.cpp`'s status line. Strings are heap-allocated to their real size: fixed buffers would cost about 130 KB for 200 Messages. Memory is bounded by the store's 200-Message and 56 KB-text limits.

### Shared Concerns

#### Test data is synthetic

- **Appears in**: Task 01, Task 02, Task 03
- **Reference example**: `test/test_ancs_request/test_main.cpp`, where responses are built byte by byte.
- **Rule**: fixtures are hand-built responses with invented names and texts. Captured serial logs hold the user's real messages and never enter the repo.

## Per-Task Technical Guidance

### Task 01: attribute-decoding

**Files to modify:**
- `src/core/ancs_request.{h,cpp}` — `AncsResponseTracker` becomes a decoder that keeps app, title, subtitle, message and date (rename it, keep the byte state machine at `ancs_request.cpp:68`). It rejects a value longer than the requested maximum or an attribute id that was not requested, and trims a trailing partial UTF-8 character.
- `src/ble/ancs_client.cpp` — drop `dump()` (line 79) and its calls (lines 225, 239). On `Complete`, check the app id against a one-entry accepted-app list: WhatsApp is logged at `D` (title, subtitle, text), any other app only increments a counter. A `Malformed` result logs `W` and clears the decoder.
- `test/test_ancs_request/test_main.cpp` — tests for values across packets, empty values, malformed lengths and a UTF-8 cut.

**Files to create:**
- `src/core/utf8.{h,cpp}` — `isUtf8Continuation` (moved from `src/core/echo_line.cpp`, its callers updated) and trimming of a trailing incomplete sequence.

**Key decisions:**
- Apply the decoder in place rather than add a parallel parser: the tracker already handles fragmentation and stale fragments.
- `hex_dump` loses its only caller: delete `src/core/hex_dump.*` and `test/test_hex_dump`.

**Watch out for:**
- Data Source carries 256-byte packets (`ancs_client.cpp:33`). A 512-byte message spans 3 packets, and a DS queue overflow (depth 16) silently corrupts a response. Guard: drop the decode when `droppedDs` grew mid-response. Test: length check.

---

### Task 02: messages-and-chats

**Files to create:**
- `src/core/chat_key.{h,cpp}` — trims direction marks (U+200E, U+200F, U+202A–U+202E) at both ends, then applies the Chat and sender rule, with "?" for an empty value. Pattern: `src/core/combo_hold.*`.
- `src/core/message_store.{h,cpp}` — an oldest-first container of Messages: Chat, sender, text, date, read flag, current UID plus a "linked this connection" flag. It enforces the 200-Message and 56 KB-text limits and keeps counts.
- `test/test_chat_key/`, `test/test_message_store/`.

**Files to modify:**
- `src/ble/ancs_client.{h,cpp}` — own the store, add a Message on every accepted response, log it at `D` (Chat, sender, text) and the counts at `I`. Expose read-only counts. `notificationCount()` loses its caller: remove it.
- `src/main.cpp` — `drawStatus()` (line 70): the Active case prints "WhatsApp: N messages, U unread".

**Key decisions:**
- The store is owned by `ancs_client`, so the main loop remains its only writer, and the interface milestone reads it later through a const accessor.

**Watch out for:**
- `logMessage` cuts at 256 characters (`src/core/log_format.cpp`). A 512-byte text logs truncated; that is expected, and the stored Message must stay complete. Test: store keeps 512 bytes.

---

### Task 03: updates-and-duplicates

**Files to modify:**
- `src/core/ancs_request.{h,cpp}` — event helpers for Removed and the PreExisting flag (EventFlags bit 2, ANCS spec).
- `src/ble/ancs_client.cpp` — a pending request carries the UID and the PreExisting flag (`pendingRequests`, line 53). A Removed event calls the store's mark-read. `onDisconnected()` (line 202) unlinks every Message, because UIDs restart.
- `src/core/message_store.{h,cpp}` and its tests:
  - a response for a linked UID updates that Message in place;
  - a PreExisting response matching an unlinked Message (date, Chat, sender, text) links it instead of adding;
  - otherwise a new Message is added.

**Watch out for:**
- Two identical PreExisting notifications would both match one stored Message. Match only Messages not yet linked in this connection, so the second one is stored. Test: two identical PreExisting entries give two Messages.

---

### Task 04: full-connection-import

**Files to modify:**
- `src/ble/ancs_client.cpp` — raise `kMaxPendingRequests` (line 38) from 64 to 512 (8 bytes per entry, about 4 KB) and keep the `W` line beyond it. Check `kNsQueueDepth` (128, line 36), which drains every loop, against a flood larger than 128.

**Key decisions:**
- A larger bounded queue instead of an unbounded one: memory stays predictable, and 512 is well above the ~110 seen.

**Watch out for:**
- Each Control Point write blocks `loop()` (`ancs_client.cpp:168`) while keys are being typed. Measure the flood duration and key latency on hardware during the 20 s window; no code change unless keys lag more than 1 s.
