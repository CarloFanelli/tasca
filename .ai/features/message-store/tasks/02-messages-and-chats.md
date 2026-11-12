# Task 02: messages-and-chats

## Objective

Each WhatsApp notification becomes a **Message** in its **Chat**, kept in memory, and the status line counts Messages and unread ones.

## Context

**Satisfies**: REQ-4, REQ-5, REQ-6, REQ-10, REQ-11, REQ-13

Turns the decoded notifications of task 01 into the Messages the interface milestone will show. Chat and sender follow the title/subtitle format observed on WhatsApp; the store is bounded so memory stays predictable.

## Depends on

- Task 01

## Expected Behavior

- Each decoded WhatsApp notification is stored as an unread Message with Chat, sender, text, date and time.
- With an empty subtitle, Chat and sender are both the title; otherwise Chat is the subtitle and sender the title.
- Invisible direction marks at the start or end of title and subtitle are removed.
- The store keeps at most 200 Messages and 56 KB of text; going over either drops the oldest.
- At Debug level each new Message is logged with Chat, sender and text; above Debug only counts.
- While ANCS is active the status line reads "WhatsApp: N messages, U unread".

## Acceptance Criteria

- [x] A message from a single chat is logged with Chat and sender equal to the contact name (REQ-4, REQ-5)
- [x] A group message is logged with the group as Chat and the author as sender (REQ-5)
- [x] The "Meta AI" name is logged without its leading invisible mark (REQ-6)
- [x] The status line shows Message and unread counts, updated when a Message arrives (REQ-13)
- [x] With the log level at Info, no Chat, sender or text appears on serial (REQ-11)
- [x] Host tests cover Chat/sender rules, mark trimming and dropping the oldest at 200 Messages or 56 KB (REQ-5, REQ-6, REQ-10)

## Edge Cases

- Title empty and subtitle empty → Chat and sender shown as "?" (REQ-5 from the spec edge-case table)
- Title empty, subtitle present → Chat is the subtitle, sender "?" (REQ-5 from the spec edge-case table)
- Name made only of invisible marks → treated as empty (REQ-6 from the spec edge-case table)
- Invisible mark in the middle of a name → kept: only start and end are trimmed (REQ-6 from the spec edge-case table)
- 201st Message arrives → oldest Message dropped, counts updated (REQ-10 from the spec edge-case table)
- Many long Messages (e.g. 120 of 500 bytes) → oldest dropped once text passes 56 KB, even under 200 Messages (REQ-10 from the spec edge-case table)
- Text longer than the 256-character log line → log line cut with `...`; stored Message complete (REQ-11 from the spec edge-case table)
- Log level Info → no Chat, sender or text on serial; counts only (REQ-11 from the spec edge-case table)
- Display failed to initialize → status not shown; Messages still stored and logged (REQ-13 from the spec edge-case table)
- ANCS not active (connecting, denied) → status line shows the BLE or ANCS state as today (REQ-13 from the spec edge-case table)
- N/A — concurrent: one iPhone, one main loop writes the store (REQ-4, REQ-10 from the spec edge-case table)
- N/A — permission: notification sharing off is already reported by ancs-capture (REQ-2, REQ-13 from the spec edge-case table)

## Notes from implementation

- **Verified on hardware**: at connection the two WhatsApp notifications within the first 64 requests became Messages. The group one is logged with the group as Chat and the author as sender; the Meta AI one has Chat and sender "Meta AI" with no leading mark. Message lines use the new `MSG` log tag (added to CLAUDE.md): content at `D`, counts at `I`.
- **REQ-10 — divergence**: the 56 KB limit counts each Message's own memory (about 110 bytes on the ESP32) on top of its Chat, sender, text and date, so the whole store stays under the 64 KB budget. With short messages the 200-Message limit still applies first; with long ones a few fewer are kept than under a text-only count. Fixed after the review.
