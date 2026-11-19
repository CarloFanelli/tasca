# Task 06: iphone-clock

## Objective

Mesh messages show the time of day they arrived, taken from the iPhone's clock at connection; without it they keep showing the time elapsed.

## Context

**Satisfies**: REQ-19, REQ-20

The Cardputer has no clock; iOS offers its time to accessories. Reading it adds nothing to the Bluetooth services Tasca offers, so no new pairing is needed.

## Depends on

- Task 01

## Expected Behavior

- At each secured connection Tasca reads the iPhone's local time and keeps it running until the Cardputer restarts.
- Mesh messages that arrive with the clock known show "HH:MM"; those without show the elapsed time.

## Acceptance Criteria

- [ ] After the iPhone connects, a new Mesh message shows its arrival time "HH:MM" (REQ-19, REQ-20)
- [ ] The clock keeps running after the iPhone disconnects, until restart (REQ-19)
- [ ] With no clock known, messages show elapsed time (REQ-20)

## Edge Cases

- iPhone does not give its time → elapsed time (REQ-19, REQ-20)
- Messages received before the clock was known → keep elapsed time (REQ-20)

## Notes from implementation

- **Known limitation**: reading the iPhone's time was not tried on the device yet: whether iOS offers it on Tasca's connection is checked with the user in Task 08; without it messages keep showing the elapsed time. /dev, 2026-11-19.
