# Task 06: autonomy-readme

## Objective
A README tells what Tasca is, how to set up the iPhone, how long the Cardputer lasts connected with the screen off, and that its Messages are readable by anyone holding it.

## Context
**Satisfies**: REQ-14
The project spec asks for the measured standby autonomy and the privacy notice in a README. Measured once Tasks 03–05 run on hardware.

## Depends on
04-screen-off-timings, 05-whatsapp-category-filter

## Expected Behavior
- The README lists the iPhone setup steps (pairing, Share System Notifications, WhatsApp previews, keyboard layout).
- It states the measured connected-standby autonomy, the settings used and the battery state at start.
- It says Messages on the Cardputer are unprotected (no screen lock in v1).

## Acceptance Criteria
- [ ] README with the iPhone setup steps (REQ-14)
- [ ] README states the measured autonomy, from full charge to shutdown, with display timeout and timings used (REQ-14)
- [ ] README says Messages on the device are readable by anyone holding it (REQ-14)

## Edge Cases
- Battery level unknown during the measurement → measured by elapsed time until shutdown (REQ-14)
- N/A — permission: no access control on the device; stated in the README (REQ-14)
