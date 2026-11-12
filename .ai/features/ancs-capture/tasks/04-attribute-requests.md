# Task 04: attribute-requests

## Objective

For every added or modified notification the Cardputer asks the iPhone for app, title, subtitle, message and date, one request at a time, and dumps every Data Source packet of the answer on serial.

## Context

**Satisfies**: REQ-9, REQ-10, REQ-11, REQ-12

This closes the M1 acceptance criterion: a WhatsApp message shows up on serial with its content, and the dumps become M3's test fixtures. Requests are serialized because Data Source answers carry no correlation beyond the notification UID.

## Depends on

- Task 03

## Expected Behavior

- Each added or modified event, PreExisting included, queues a Get Notification Attributes request for AppIdentifier, Title (64), Subtitle (64), Message (512), Date; removed events queue nothing.
- Requests go out one at a time; the next starts when the previous response is complete or after 2 s.
- Every Data Source packet is dumped in the Task 03 format, channel `DS`.
- With an empty queue, a new notification's content is dumped within 3 s of its arrival.

## Acceptance Criteria

- [ ] A WhatsApp message produces Data Source dumps containing its app id, title and text (REQ-9, REQ-11)
- [ ] Only one request is in flight at any time; the next waits for completion or 2 s (REQ-10)
- [ ] Removed events produce no request (REQ-9)
- [ ] Data Source dumps use the shared format with channel `DS` (REQ-11, REQ-12)
- [ ] Host tests cover request encoding and response completion across fragments, empty attributes and errors (REQ-9, REQ-10)

## Edge Cases

- Notification removed before its attributes are read → iOS answers with an error: `W` line, next request starts (REQ-9 from the spec edge-case table)
- Notification without subtitle or message → request identical; empty attributes appear in the dump (REQ-9 from the spec edge-case table)
- No response within 2 s → `W` timeout line, next request starts (REQ-10 from the spec edge-case table)
- New notifications while a request is pending → queued in arrival order (REQ-10 from the spec edge-case table)
- More than 64 pending requests → requests beyond 64 dropped with a `W` line; NS dumps continue (REQ-10 from the spec edge-case table)
- Disconnection with requests pending → queue cleared, `I` line with how many were dropped (REQ-10 from the spec edge-case table)
- Response split across several Data Source packets → each packet dumped separately, in arrival order (REQ-11 from the spec edge-case table)
- Disconnection mid-response → packets already received stay dumped; nothing else printed for that request (REQ-11 from the spec edge-case table)

## Notes from implementation

- Verified on hardware: a WhatsApp notification (Meta AI chat, used for lack of a second sender) was answered in the same second as its Notification Source event, split over two Data Source packets (253 + 72 bytes) and recognized as complete: app id `net.whatsapp.WhatsApp`, title, empty subtitle, 259-byte message, date. At connection 109 PreExisting events gave 66 complete responses with no timeout; the other 43 were dropped by the 64-request cap with `W` lines, as the spec states.
- **Known limitation**: not exercised on hardware: the ANCS error for a notification removed before its request, the 2 s timeout, and a disconnection with requests pending or mid-response. They are covered by the response-tracker host tests and the code paths, not by a device run.
