---
feature: message-store
status: APPROVED
reviewed: 2026-11-12
head: 0e50a998eea5e6a7ed00aa26dea5b6c572db2216
---

# Review — message-store

**Scope**: re-verification of the 3 addressed findings over the same 18 files from the diff since base `0e50a998`, narrowed to this feature's files. Nothing has been committed since base, so the diff also carries the already-reviewed firmware-skeleton, ancs-capture and hid-keyboard work, which was left out. Workflow artifacts under `.ai/` were excluded. Files: `CLAUDE.md`, `src/ble/ancs_client.{h,cpp}`, `src/core/ancs_request.{h,cpp}`, `src/core/utf8.{h,cpp}`, `src/core/chat_key.{h,cpp}`, `src/core/message_store.{h,cpp}`, `src/core/echo_line.{h,cpp}`, `src/main.cpp`, and the tests `test_ancs_request`, `test_utf8`, `test_chat_key`, `test_message_store`. The scan was done in one context: the feature's changes are below the size that warrants agents.
**Findings**: 0 must-fix · 0 should-fix · 0 nit

## Findings

### F-1 · should-fix · bug · store memory can exceed the 64 KB budget   [ADDRESSED]
**File**: `src/core/message_store.cpp:5-7`
**Issue**: The 56 KB limit counts only the bytes of the four strings. Each Message also costs its own struct (four `std::string` objects, a UID and two flags, about 110 bytes on the ESP32) plus the deque's storage. 200 Messages near the text limit therefore take about 77 KB, while the spec's performance requirement says the store stays under 64 KB.
**Change**: Count `sizeof(Message)` in `bytesOf()` alongside the string sizes, and rename the counter and constant to say "memory" (for example `memoryBytes()` / `kMaxStoreBytes`). The same 56 KB limit then bounds the whole store; update `test_text_over_the_byte_limit_drops_the_oldest_first` accordingly.
**Why**: Non-functional requirement "the store's memory stays under 64 KB at 200 Messages"; the limit the user chose was meant to keep memory predictable.

### F-2 · nit · bug · a group named "?" is taken for a single chat   [ADDRESSED]
**File**: `src/core/chat_key.cpp:26-34`
**Issue**: `trimmed()` returns the placeholder "?" for an empty name, and `chatKeyOf` then compares the result with "?" to detect an empty subtitle. A group actually named "?" is therefore treated as a single chat, and its Chat becomes the sender.
**Change**: Have `trimmed()` return an empty string. In `chatKeyOf`, test the trimmed subtitle for emptiness and substitute "?" only at the end, for each of Chat and sender.

### F-3 · nit · dup · log_format keeps its own UTF-8 continuation test   [ADDRESSED]
**File**: `src/core/log_format.cpp:17-19`
**Issue**: The private `isUtf8Continuation` repeats the function this feature added to `src/core/utf8.h:6`.
**Change**: Remove the local copy and include `core/utf8.h`, as `src/core/echo_line.cpp` now does.
