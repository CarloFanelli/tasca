# Task 03: readme-iphone-check

## Objective

The README documents Del in Reading mode, and clearing is checked on the user's iPhone with WhatsApp.

## Context

**Satisfies**: REQ-9

WhatsApp's response to the negative action is the one external unknown of the feature; the README states what the check shows.

## Depends on

Task 02

## Expected Behavior

- The Reading mode key table lists Del, with Del twice clearing the Chat on the iPhone too.
- The README says that Settings "Clear messages" clears the Cardputer only.
- On the iPhone: clearing a WhatsApp Chat, a Chat with Messages from before a reconnection, and a refusal case are checked; the outcome goes in this task's notes.

## Acceptance Criteria

- [ ] The README key table lists Del and what it does on the iPhone (REQ-9)
- [ ] The README distinguishes clearing a Chat from Settings "Clear messages" (REQ-9)
- [ ] The device check of WhatsApp clearing and its outcome are recorded in this task's notes (REQ-9)

## Edge Cases

- Settings "Clear messages" → unchanged: clears the Cardputer only; the README says so (REQ-8, REQ-9)

## Notes from implementation

- **Device check (2026-11-13)**: on the user's iPhone with WhatsApp, Del twice from the Inbox and from a Chat screen cleared the Chat on the iPhone (lock screen and Notification Center) and on the Cardputer; another key removed the prompt without clearing. The serial log showed 2 clearings (2 and 1 notifications, none unlinked), 3 negative actions sent, 0 refused, and no W or E line: WhatsApp accepts the ANCS negative action as "Clear". The "Not cleared on the iPhone" notice was not triggered on the device.
- **Flash**: +6,464 bytes over the build before Task 01 (2,029,815 → 2,036,279). A `std::deque<uint32_t>` for the clear queue cost about 15 KB; a `std::vector` already linked does the same job.
