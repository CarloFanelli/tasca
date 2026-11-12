---
feature: message-store
version: 1.1
status: approved
completed: true
base: 0e50a998eea5e6a7ed00aa26dea5b6c572db2216
---

# Message Store (M3)

## Overview

Turns the iPhone's notifications into WhatsApp **Messages** kept on the Cardputer. Every
attribute response from the iPhone is decoded; only WhatsApp notifications are kept; each one
becomes a Message with its **Chat**, sender, text, date and read state. Updates and removals on
the iPhone update or mark read the matching Message, and the notifications the iPhone resends at
every connection are not duplicated. Every notification present at connection is fetched, not only
the first 64. Decoded Messages replace the raw hex dumps on serial; the status line counts Messages
and unread ones. Showing Messages on screen is the next milestone.

## Project Context

- Builds on ancs-capture: ANCS subscription, attribute requests one at a time with a 2 s timeout.
- Requested attributes: app, title (64 bytes), subtitle (64), message (512), date; one answer can span several Data Source packets.
- Observed on WhatsApp: single chat has title = contact and empty subtitle; group has title = sender, subtitle = group name.
- Names may contain emoji and a leading invisible direction mark (seen on "Meta AI").
- Notification UIDs restart at 0 on every connection; about 110 PreExisting notifications arrive at each connection.
- No PSRAM: the Message store must stay within about 60 KB.
- Decoding and the store are hardware-independent and host-tested; test data must be synthetic, never real captured content.
- Concurrency rules in CLAUDE.md apply: only the main loop writes the store and draws.

## Functional Requirements

### Decoding

- REQ-1: An attribute response is decoded into app, title, subtitle, message and date, including responses split across several Data Source packets.
- REQ-2: Only notifications from `net.whatsapp.WhatsApp` become Messages; the accepted apps are one list in the code.
- REQ-3: Every notification present at connection gets its attributes requested; none is skipped because too many are waiting.

### Messages

- REQ-4: Each WhatsApp notification becomes a Message with Chat, sender, text, date and time, unread when created.
- REQ-5: With an empty subtitle, Chat and sender are the title; otherwise Chat is the subtitle and sender the title.
- REQ-6: Invisible direction marks at the start or end of title and subtitle are removed before use.
- REQ-7: A modified event for a notification already stored updates that Message instead of creating a new one.
- REQ-8: A removed event marks the matching Message as read.
- REQ-9: A PreExisting notification matching a stored Message (same date, Chat, sender, text) is not stored again.
- REQ-10: The store keeps at most 200 Messages and 56 KB of text; going over either drops the oldest.

### Serial and status

- REQ-11: At Debug level each new or updated Message is logged with Chat, sender and text; above Debug only counts appear.
- REQ-12: Raw hex dumps of Notification Source and Data Source packets are no longer logged.
- REQ-13: While ANCS is active the status line reads "WhatsApp: N messages, U unread".

## Data & Dependencies

### Information the feature must retain

- Message — one WhatsApp notification: Chat, sender, text, date, read flag, and the notification it currently belongs to. Created when its attributes arrive; updated by a modified event; marked read by a removed event; dropped when the store is over 200 Messages or 56 KB of text, or at restart (RAM only).

### External capabilities relied on

- Apple Notification Center Service on the iPhone — delivers notification events and their attributes.

## Non-Functional Requirements

- **Performance** — A new WhatsApp notification is stored and logged within 1 s of its event, outside the connection flood.
- **Performance** — The connection flood of about 110 notifications is fully processed within 20 s; typing in Keyboard mode keeps working meanwhile.
- **Performance** — The store's memory stays under 64 KB at 200 Messages and does not grow beyond.
- **Security** — Message content lives in RAM only; never written to flash or SD. On serial only at Debug level.
- **Validation** — Text is stored as valid UTF-8: a character cut by the iPhone's length limit is dropped whole.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| Response split over 2 or more Data Source packets | Decoded once complete, same result as a single packet | REQ-1 |
| Attribute with length 0 (e.g. empty subtitle or message) | Decoded as empty | REQ-1, REQ-5 |
| Response longer than declared or malformed (length past the end) | Discarded with a `W` line; next request proceeds | REQ-1 |
| No response within 2 s | Request abandoned with a `W` line, next one sent; no Message created | REQ-1 |
| Disconnection mid-response | Partial response discarded; nothing stored | REQ-1 |
| Notification from another app | Not stored; counted only | REQ-2 |
| WhatsApp Business or other app ids | Not stored (not in the list) | REQ-2 |
| More than 200 notifications present at connection | All requested, in arrival order; store keeps the newest 200 WhatsApp ones | REQ-3, REQ-10 |
| Notification removed on the iPhone before its request is sent | iPhone refuses the request; `W` line, nothing stored, next request proceeds | REQ-3 |
| New message arriving during the connection flood | Requested after those already waiting; stored like any other | REQ-3 |
| Title empty and subtitle empty | Chat and sender shown as "?" | REQ-5 |
| Title empty, subtitle present | Chat is the subtitle, sender "?" | REQ-5 |
| Name made only of invisible marks | Treated as empty | REQ-6 |
| Invisible mark in the middle of a name | Kept: only start and end are trimmed | REQ-6 |
| Modified event for a notification never stored (e.g. its request failed) | Attributes requested; stored as a new Message if WhatsApp | REQ-7 |
| Modified event changes the text | Message text updated; read state unchanged | REQ-7 |
| Removed event for a notification never stored | Ignored | REQ-8 |
| Removed event for a Message already read | No change | REQ-8 |
| Reconnection: unread Message still on the iPhone | Not duplicated; linked to its new notification so a later removal marks it read | REQ-9 |
| Two different messages with the same date, Chat, sender and text (e.g. "ok" twice in one second) | Second one stored too when both arrive live; merged only when both are PreExisting duplicates | REQ-9 |
| Message read on the iPhone while the Cardputer was disconnected | Not resent at reconnection; stays unread on the Cardputer | REQ-8, REQ-9 |
| Store full and a Message updated | Updated in place; nothing dropped | REQ-7, REQ-10 |
| 201st Message arrives | Oldest Message dropped, counts updated | REQ-10 |
| Many long Messages (e.g. 120 of 500 bytes) | Oldest dropped once text passes 56 KB, even under 200 Messages | REQ-10 |
| Text longer than the 256-character log line | Log line cut with `...`; stored Message complete | REQ-11 |
| Log level Info | No Chat, sender or text on serial; counts only | REQ-11 |
| Text truncated by the iPhone mid-character | Partial character dropped; rest stored | REQ-4 |
| Emoji in Chat, sender or text | Stored and logged as received | REQ-4 |
| Display failed to initialize | Status not shown; Messages still stored and logged | REQ-13 |
| ANCS not active (connecting, denied) | Status line shows the BLE or ANCS state as today | REQ-13 |
| N/A — concurrent: one iPhone, one main loop writes the store | — | REQ-4, REQ-10 |
| N/A — permission: notification sharing off is already reported by ancs-capture | — | REQ-2, REQ-13 |

## Out of Scope

- **Inbox and Chat screens, new-message banner** — next milestone (UI).
- **Beep and display wake on a new Message** — needs settings to turn them off; milestone M5.
- **Emoji placeholder on screen** — a display concern; handled with the UI.
- **Messages kept across restarts (SD log)** — optional milestone M6.
- **Deleting a notification on the iPhone from the Cardputer** — optional, not needed to read and reply.
- **WhatsApp Business** — the user does not use it; adding it is one entry in the app list.
