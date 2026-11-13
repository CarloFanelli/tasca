# Task 01: reading-screens-stack

## Objective
The Inbox, Chat screen and Settings screen run on the **Screen stack**, each in its own source file; in Reading mode everything looks and behaves as before.

## Context
**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-4, REQ-5, REQ-6, REQ-7
The stack and its host-tested rules come first, proven on the three Reading mode screens. Keyboard mode keeps working as today until Task 02 moves it onto the stack. REQ-1 to REQ-6 are shared with Task 02: this task covers Reading mode.

## Depends on
None

## Expected Behavior
- The Inbox is always at the bottom of the Screen stack; Enter opens the Chat screen over it, ` returns to it.
- Opt+s opens the Settings screen over the Inbox or Chat screen; ` closes it back to that screen, unchanged.
- Only the screen on top receives keys and draws; the status bar is drawn as today, whatever screen is on top.
- Every key, beep, wake and timeout in Reading mode behaves as before; Keyboard mode is unchanged.

## Acceptance Criteria
- [ ] Host tests cover open over, close back, top receives keys, Inbox never closed, same screen not opened twice (REQ-5, REQ-6, REQ-7)
- [ ] Inbox, Chat screen and Settings screen each live in their own source file with draw, key and update entry points (REQ-4)
- [ ] Every Reading mode key and combination does what it did before (REQ-1)
- [ ] Inbox, Chat screen, Settings screen and status bar look as before (REQ-2)
- [ ] Beeps, display timeout, wake on new message and Siri behave as before in Reading mode (REQ-3)

## Edge Cases
- Key with the screen off → only turns the screen on, as today (REQ-1, REQ-3)
- Messages arrive while the Settings screen covers a Chat screen → Inbox updated underneath; that Chat marked read only when back on top (REQ-3, REQ-5)
- Every Message of the covered Chat removed meanwhile → closing the Settings screen shows the Inbox (REQ-5)
- Fn+Del held 3 s → pairing reset from Reading mode screens, as today (REQ-1)
- Fn+Enter (Siri) and ` right after it → same on every Reading mode screen; ` closes Siri instead of going back (REQ-1, REQ-3)
- Closing the only screen left (the Inbox) → not possible: the Inbox is always at the bottom (REQ-5, REQ-6)
- Same screen opened twice over itself → opening a screen already on top does nothing (REQ-5, REQ-7)
- Status bar changes (link, battery, unread) under any screen → redrawn as today (REQ-2, REQ-6)
- N/A — concurrent: the main loop alone opens, closes and draws screens (REQ-5, REQ-6)
- N/A — network: BLE handling and its queues are untouched (REQ-3, REQ-4)
