# Task 06: readme

## Objective
A README tells what Tasca is, how to set up the iPhone, and that its Messages are readable by anyone holding the Cardputer.

## Context
**Satisfies**: REQ-14
The project spec asks for the iPhone setup and the privacy notice in a README. The measured standby autonomy was postponed to the backlog (B-6).

## Depends on
None

## Expected Behavior
- The README lists the iPhone setup steps (pairing, Share System Notifications, WhatsApp previews, keyboard layout).
- It lists the Cardputer's keys: Reading mode, Settings, Keyboard mode, pairing reset.
- It says Messages on the Cardputer are unprotected (no screen lock in v1).

## Acceptance Criteria
- [ ] README with the iPhone setup steps, in the order they are done (REQ-14)
- [ ] README says Messages on the device are readable by anyone holding it (REQ-14)

## Edge Cases
- README read before pairing → setup steps in the order they are done: pair, share notifications, keyboard layout (REQ-14)
- N/A — permission: no access control on the device; stated in the README (REQ-14)
