---
feature: mesh-chat
status: APPROVED
reviewed: 2026-11-19
head: 34eab6e9005a8092099e5d979dbc3c44c9137e29
---

# Review — mesh-chat

**Scope**: 69 files (re-verification — delta since review head `34eab6e9`, nothing committed since, so the whole uncommitted feature; 17 workflow artifacts under `.ai/` excluded)
**Findings**: 0 must-fix · 0 should-fix · 0 nit

## Findings

### F-1 · should-fix · bug · message sent during an announcement reads not sent   [ADDRESSED]
**File**: `src/mesh/lora_unit.cpp:162-176`
**Issue**: `sendText` lists the message and calls `transmit`, which fails whenever the unit is still busy with the name announcement (`AtLink::send` refuses while a send is in flight), so a message typed right after the unit is found, after a Mesh name change or at the 3-hour announcement reads "not sent" although the unit is plugged in.
**Change**: When the link is present but busy, keep the message's packet pending and hand it to `transmit` from `poll()` once the announcement is done, as `pollAnnounce` already retries a due announcement; mark it "not sent" only when the unit is absent.
**Why**: the spec reads "not sent" only for a unit unplugged while sending (REQ-10, REQ-12 edge cases).
