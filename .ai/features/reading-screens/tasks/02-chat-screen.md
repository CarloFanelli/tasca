# Task 02: chat-screen

## Objective
Enter on a Chat opens its Chat screen: every Message of the Chat, readable line by line, and marked read on the Cardputer.

## Context
**Satisfies**: REQ-5, REQ-6, REQ-7, REQ-8
The Chat screen is where Messages are actually read; opening it is what clears their unread state on the Cardputer (iOS offers no way to mark them read on the iPhone).

## Depends on
01-status-bar-inbox

## Expected Behavior
- Enter on the selected Chat opens its Chat screen; `` ` `` returns to the Inbox with the same Chat selected.
- Messages appear oldest to newest, each with its time (HH:MM); in groups each starts with its sender.
- Text wraps at word boundaries; the screen opens with the start of the newest Message visible, earlier lines above it when it is short.
- `;` scrolls up one line, `.` down one line.
- Opening the Chat screen marks all its Messages read; the status bar and Inbox counts drop accordingly.
- Emoji and non-Latin-1 characters show as □, as in the Inbox.

## Acceptance Criteria
- [ ] Enter opens the selected Chat's screen; `` ` `` returns to the Inbox (REQ-6)
- [ ] Messages show oldest to newest, word-wrapped, each with HH:MM, sender first in groups (REQ-5)
- [ ] The screen opens showing the start of the newest Message; `;` and `.` scroll one line (REQ-6)
- [ ] After opening, the Chat has no unread Messages and the total unread drops by that count (REQ-7)
- [ ] A word longer than the line is broken inside the word, never overflowing the screen (REQ-5)

## Edge Cases
- Chat screen already at its top or bottom → further scroll ignored (REQ-6)
- Message longer than the screen → wrapped; reachable line by line with scrolling (REQ-5, REQ-6)
- Fn held with `;` or `.` → same as without Fn (REQ-6)
- Other keys in a Chat screen (letters, Enter) → ignored; nothing sent to the iPhone (REQ-6)
- Removed event for a Message already read on the Cardputer → no visible change (REQ-7)
- Emoji in a Chat name or text → shown as □ (REQ-8)

## Notes from implementation

- **REQ-6 — divergence**: the Chat screen first opened on the newest Message's last line, so a long Message had to be read scrolling up. Agreed with the user on hardware: it opens showing the newest Message from its start, with earlier lines above when it is short; a screen at its end follows a new Message from its start. SPEC bumped to 1.1.
