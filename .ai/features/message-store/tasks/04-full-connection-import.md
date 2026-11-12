# Task 04: full-connection-import

## Objective

At connection the Cardputer fetches every notification present on the iPhone, not only the first 64, within 20 s and without blocking typing.

## Context

**Satisfies**: REQ-3

Today about 45 of 110 PreExisting notifications are skipped because only 64 requests can wait. With Messages stored, skipped notifications mean missing Messages. Last, because it needs the store to verify the result and the soak conditions of the keyboard milestone to check typing.

## Depends on

- Task 02

## Expected Behavior

- Every added or modified notification at connection gets its attributes requested, in arrival order; no "request queue full" line appears.
- The flood of about 110 notifications is processed within 20 s of the subscription.
- Typing in Keyboard mode during the flood keeps working: keys arrive, in order.

## Acceptance Criteria

- [x] At connection with about 110 notifications, no request is dropped and every WhatsApp one becomes a Message (REQ-3)
- [x] The last attribute response of the flood arrives within 20 s of the subscription (REQ-3)
- [x] Keys typed during the flood all reach the iPhone, in order (REQ-3)

## Edge Cases

- More than 200 notifications present at connection → all requested, in arrival order; store keeps the newest 200 WhatsApp ones (REQ-3, REQ-10 from the spec edge-case table)
- Notification removed on the iPhone before its request is sent → iPhone refuses the request; `W` line, nothing stored, next request proceeds (REQ-3 from the spec edge-case table)
- New message arriving during the connection flood → requested after those already waiting; stored like any other (REQ-3 from the spec edge-case table)

## Notes from implementation

- **Verified on hardware**: at connection 114 notifications were all requested and decoded in 11 s, with no `request queue full` line. The 2 unread WhatsApp ones became Messages. After a reset the user typed in Notes during the 10-second flood, and the text arrived complete and in order. No change to the Notification Source queue was needed: 128 packets drained every loop covered 114 events.
