# Task 04: screen-off-timings

## Objective
With the screen off, the Bluetooth link runs on slower, power-saving timings accepted by iOS; with the screen on it goes back to the usual ones.

## Context
**Satisfies**: REQ-12
The main power saving while the Cardputer sits connected with the screen off, the state Task 06 measures.

## Depends on
03-beep-and-sleep

## Expected Behavior
- When the screen turns off, the Cardputer asks the iPhone for slower link timings.
- When the screen turns on, it asks for the usual timings at once.
- The timings granted are logged, so the change is visible on serial.

## Acceptance Criteria
- [ ] Screen off requests slower timings within iOS limits; the granted values are logged (REQ-12)
- [ ] Screen on requests the usual timings at once (REQ-12)
- [ ] Notifications and keys keep working with the slower timings (REQ-12)

## Edge Cases
- iPhone refuses the slower timings → link keeps the current ones; `W` line; no retry until the next screen change (REQ-12)
- Screen on while slow timings pending → usual timings requested at once (REQ-12)
- Display timeout Never → timings stay usual (REQ-4, REQ-9)

## Notes from implementation

- **Known limitation**: a connection made while the screen is off keeps the usual timings until the screen turns on and off again. Requesting the slow timings at once would slow down the import of the notifications already on the iPhone (about 120 requests), which must finish in under 5 s.
- **Known limitation**: the usual timings are a fixed 15–30 ms window, not the exact timings iOS picked at connection: Apple requires the requested maximum to exceed the minimum by at least 15 ms, so a single value cannot be requested.
