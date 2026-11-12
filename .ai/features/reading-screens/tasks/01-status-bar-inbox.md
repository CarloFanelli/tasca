# Task 01: status-bar-inbox

## Objective
At boot the Cardputer shows a status bar and the Inbox: every Chat newest first, two lines each, browsable with `;` and `.`.

## Context
**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-4, REQ-8
The Inbox and the status bar replace the Hello screen and are the base every other screen builds on. The □ substitution lands here because Chat names already carry emoji.

## Depends on
None

## Expected Behavior
- A status bar at the top shows link state, mode, total unread and battery percent, in English.
- Reading mode shows the Inbox: one entry per Chat, the Chat with the newest Message first.
- Each entry has two lines: the Chat name with its unread count, then the last Message in grey; in groups the preview starts with its sender.
- `;` moves the selection up, `.` down; the list scrolls to keep the selection visible.
- Enter on a selected Chat is accepted (it opens the Chat screen once Task 02 exists; until then it does nothing).
- Accented letters show as themselves; each emoji, and any character outside Latin-1, shows as one □.

## Acceptance Criteria
- [ ] The status bar shows link state, mode, total unread and battery percent on every screen (REQ-1)
- [ ] The Inbox lists every Chat once, ordered by its newest Message, newest first (REQ-2)
- [ ] Each Chat shows name, unread count and a grey preview prefixed by the sender in groups (REQ-3)
- [ ] `;` and `.` move the selection and scroll; Enter on a Chat is recognized (REQ-4)
- [ ] An emoji, including a multi-codepoint one, shows as a single □; à è é ì ò ù show correctly (REQ-8)
- [ ] The screen is redrawn only when its content changes (REQ-1)

## Edge Cases
- Battery level unknown → status bar shows "--%" (REQ-1)
- Link not ready (advertising, connecting, pairing failed) → status bar shows the state; the Inbox still shows stored Messages (REQ-1, REQ-2)
- ANCS denied → status bar shows "ANCS denied" (REQ-1)
- No Messages yet → Inbox shows "No messages yet" (REQ-2)
- Total unread above 99 → shown as "99+" (REQ-1, REQ-3)
- Chat name too long for the line → cut with "…" before the unread count (REQ-3)
- Chat with no unread Messages → no count shown (REQ-3)
- Selection on the first or last Chat and key pressed further → selection stays; no wrap-around (REQ-4)
- Fn held with `;` or `.` → same as without Fn (REQ-4)
- Other keys in Reading mode → ignored; nothing sent to the iPhone (REQ-4)
- Emoji in a Chat name → shown as □ (REQ-8)
- Character outside Latin-1 that is not an emoji (e.g. Cyrillic) → shown as □ (REQ-8)
- Display failed to initialize → nothing drawn; Messages still stored (REQ-1)
