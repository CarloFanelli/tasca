# Task 04: keyboard-context-banner

## Objective
Keyboard mode keeps the reading context: it shows the last Message of the Chat being answered, flashes a Banner for new Messages, and G0 returns to the screen it came from.

## Context
**Satisfies**: REQ-10, REQ-11, REQ-12
This closes the reply loop: read a Chat, press G0, type the answer on the iPhone while seeing what is being answered, and notice new Messages without leaving Keyboard mode.

## Depends on
03-live-updates

## Expected Behavior
- G0 from the Inbox or a Chat screen enters Keyboard mode; G0 again returns to the same screen, selection and scroll.
- Entered from a Chat screen, the last Message of that Chat is shown above the echo line; entered from the Inbox, that area is empty.
- A Message arriving in Keyboard mode shows a Banner with Chat and text for 3 s, then the previous content returns.
- Typing, the echo line and the iPhone-not-ready notice behave as today.

## Acceptance Criteria
- [ ] G0 toggles Keyboard mode and returns to the same Inbox selection or Chat screen scroll (REQ-10)
- [ ] From a Chat screen, its last Message is shown above the echo line (REQ-11)
- [ ] A new Message in Keyboard mode shows a Banner with Chat and text for 3 s (REQ-12)
- [ ] Keys typed while a Banner is shown reach the iPhone and the echo line unchanged (REQ-12)

## Edge Cases
- Keyboard mode entered from the Inbox → area above the echo line left empty (REQ-11)
- Message arrives in the Chat shown above the echo line → Banner shown and the line updates to the new Message (REQ-11, REQ-12)
- Several Messages arrive within 3 s → newest replaces the Banner; 3 s restart (REQ-12)
- Banner shown while the iPhone is not ready → Banner shown; keys still discarded as today (REQ-12)
- G0 pressed while a Banner is shown → Banner removed; Reading screen shown (REQ-10, REQ-12)
- Message arrives in Reading mode → no Banner; Inbox or Chat screen updates (REQ-9, REQ-12)
- Display failed to initialize → nothing drawn; keys still sent (REQ-10)
- Messages fetched at reconnection (already on the iPhone before the connection) → stored silently; no Banner (REQ-12 · from code)
- Message updated by the iPhone in Keyboard mode → no Banner; the context line shows the new text if it is that Chat's last Message (REQ-11, REQ-12 · from code)
