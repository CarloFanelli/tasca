---
feature: music-menu
status: APPROVED
reviewed: 2026-11-14
head: a47f428db192640232aae8018bcd7443aefa2a51
---

# Review — music-menu

**Scope**: re-verification of `src/core/ams.cpp` and `test/test_ams/test_main.cpp`, the files changed since the review of `9bfecd33` (the other 21 files of the feature were covered by that review, as uncommitted work, and are unchanged since; 11 workflow artifacts under `.ai/` excluded)
**Findings**: 0 must-fix · 0 should-fix · 0 nit

## Findings

### F-1 · should-fix · bug · long AMS values cut mid-character   [ADDRESSED]
**File**: `src/ble/ams_client.cpp:47`
**Issue**: `onEntityUpdate` copies at most 3 + 128 bytes and stores the copied length, so `decodeAmsUpdate` (`src/core/ams.cpp:86`) never sees a value longer than 128 bytes and never trims: a title or artist the iPhone sends whole but longer than 128 bytes (possible with a 185-byte MTU) is cut at byte 128, possibly inside a multi-byte character, which then shows as a broken glyph; the host test `test_decode_cuts_the_value_at_128_bytes_on_a_whole_character` covers a path the firmware never takes.
**Change**: keep the length as received in `UpdatePacket`, as `Packet::length` does in `src/ble/ancs_client.cpp:28`, and in `poll` pass the copied length to `decodeAmsUpdate`, then call `trimIncompleteUtf8` on the value when the received length was longer.
**Why**: REQ-6 edge case — a long title is shown up to two lines then "…", never with a broken character.
**Resolution**: fixed in `src/core/ams.cpp:88` instead: the trim runs from exactly 128 bytes, where the client's cut lands, and is a no-op on a whole value; covered by `test_decode_drops_a_character_cut_at_exactly_128_bytes`.
