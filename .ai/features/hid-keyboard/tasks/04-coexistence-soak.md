# Task 04: coexistence-soak

## Objective

Typing and notification capture work together on one connection for 30 minutes without a disconnection.

## Context

**Satisfies**: REQ-14, REQ-15

The milestone's acceptance test and its main technical risk: iOS might renegotiate or drop a link that is both a keyboard and an ANCS client. It needs both earlier tasks in place and a long hardware run.

## Depends on

- Task 01, Task 03

## Expected Behavior

- While the user types in Keyboard mode, every notification is still dumped on serial with its content.
- A 30-minute session of typing with notifications arriving ends without any disconnection in the log.

## Acceptance Criteria

- [x] Notifications arriving while typing are dumped on serial, typing and echo unaffected (REQ-14)
- [x] 30 minutes of typing with notifications arriving: no `disconnected` line in the serial log (REQ-15)
- [x] Keys typed during the connection-time notification flood all arrive, in order (REQ-14)

## Edge Cases

- Typing during the PreExisting notification flood at connection → keys may be delayed up to 1 s; none lost or reordered (REQ-14 from the spec edge-case table)
- Notification arrives mid-word → dumped on serial; typing and echo unaffected (REQ-14 from the spec edge-case table)
- iOS renegotiates the connection during the 30-minute run → allowed if the link stays up; a disconnection fails the test (REQ-15 from the spec edge-case table)

## Notes from implementation

- **Soak result**: the link stayed up from 11:07 to 11:44 (37 minutes) with no `disconnected` line. Keyboard mode was on from 11:09, and the three WhatsApp notifications that arrived while typing (11:10, 11:22, 11:34) were all dumped, with their attributes fetched. During an earlier run the user typed through the connection-time flood with no lost or reordered keys. No code change was needed.
- **Known limitation**: during the connection-time flood (about 110 PreExisting notifications), attribute requests beyond the 64-request cap are dropped and logged as `request queue full`. Every Notification Source event is still dumped (REQ-14). This behaviour comes from the ancs-capture feature and does not affect typing.
