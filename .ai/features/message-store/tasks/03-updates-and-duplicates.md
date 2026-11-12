# Task 03: updates-and-duplicates

## Objective

Messages follow the iPhone: an updated notification updates its Message, a removed one marks it read, and reconnecting does not duplicate Messages already stored.

## Context

**Satisfies**: REQ-7, REQ-8, REQ-9

Without this every reconnection would double the store with the ~110 PreExisting notifications, and unread counts would never go down. Notification UIDs restart at each connection, so a Message is matched by its content when it comes back.

## Depends on

- Task 02

## Expected Behavior

- A modified event for a notification already stored updates its Message; the read state does not change.
- A removed event marks the matching Message read; the unread count goes down.
- After a reconnection, a PreExisting notification matching a stored Message (same date, Chat, sender, text) is not stored again but linked to that Message.
- A notification arriving live is always stored, even if identical to an earlier one.

## Acceptance Criteria

- [x] Reading a WhatsApp message on the iPhone marks it read on the Cardputer: unread count goes down (REQ-8)
- [x] After restarting the iPhone's Bluetooth, the Message count is unchanged (REQ-9)
- [x] After a reconnection, reading on the iPhone a Message received before it still marks it read (REQ-8, REQ-9)
- [x] Host tests cover modified updates, removed marking read, PreExisting deduplication and live duplicates (REQ-7, REQ-8, REQ-9)

## Edge Cases

- Modified event for a notification never stored (e.g. its request failed) → attributes requested; stored as a new Message if WhatsApp (REQ-7 from the spec edge-case table)
- Modified event changes the text → Message text updated; read state unchanged (REQ-7 from the spec edge-case table)
- Store full and a Message updated → updated in place; nothing dropped (REQ-7, REQ-10 from the spec edge-case table)
- Removed event for a notification never stored → ignored (REQ-8 from the spec edge-case table)
- Removed event for a Message already read → no change (REQ-8 from the spec edge-case table)
- Reconnection: unread Message still on the iPhone → not duplicated; linked to its new notification so a later removal marks it read (REQ-9 from the spec edge-case table)
- Two different messages with the same date, Chat, sender and text (e.g. "ok" twice in one second) → second one stored too when both arrive live; merged only when both are PreExisting duplicates (REQ-9 from the spec edge-case table)
- Message read on the iPhone while the Cardputer was disconnected → not resent at reconnection; stays unread on the Cardputer (REQ-8, REQ-9 from the spec edge-case table)

## Notes from implementation

- **Verified on hardware**: a WhatsApp message read on the iPhone went to read on the Cardputer. After switching the iPhone's Bluetooth off and on, a group message received before the reconnection was recognised as already stored, so the count stayed at 2. Opening that chat afterwards marked it read through its new notification.
- **Status line counts every stored Message, read ones included**: "WhatsApp: 2 messages, 0 unread" after both were read. This is intended: the store keeps the history for the next milestone's screens, while the iPhone drops a notification once it is read.
