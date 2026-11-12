# Task 03: beep-and-sleep

## Objective
A Message arriving live beeps and can wake the screen; the screen turns off after the display timeout and a key or G0 turns it back on.

## Context
**Satisfies**: REQ-8, REQ-9, REQ-10, REQ-11
Uses the sound, display timeout and wake settings of Task 01. The screen-off state is what Task 04 saves power on.

## Depends on
01-settings-screen

## Expected Behavior
- With sound on, each live Message plays one short beep.
- Without a key, G0 or live Message for the display timeout, the screen turns off; Bluetooth stays connected.
- A key or G0 turns the screen on; in Reading mode that key does nothing else, in Keyboard mode it also reaches the iPhone.
- With wake on, a live Message turns the screen on.

## Acceptance Criteria
- [ ] A live Message beeps once when sound is on, never when off; resent notifications never beep (REQ-8)
- [ ] The screen turns off after the display timeout; Never keeps it on (REQ-9)
- [ ] A key turns the screen on within 200 ms; in Reading mode it does nothing else (REQ-10)
- [ ] In Keyboard mode a key typed with the screen off reaches the iPhone (REQ-10)
- [ ] A live Message turns the screen on only when wake on new Message is on (REQ-11)

## Edge Cases
- Several Messages arrive within a second → one beep each, never overlapping (REQ-8)
- Live Message while the screen is off and wake is off → beep if sound on; screen stays off; unread counts update (REQ-8, REQ-11)
- Updated Message (edited on the iPhone) → no beep, no wake (REQ-8, REQ-11)
- Display timeout Never → screen stays on (REQ-4, REQ-9)
- Screen turns off in Keyboard mode → keys still go to the iPhone; echo continues (REQ-9, REQ-10)
- Fn+Del held with the screen off → pairing reset still works after 3 s (REQ-10)
- G0 with the screen off → screen on; mode switches only on the next press (REQ-10)
- Display failed to initialize → beep still works (REQ-9)
