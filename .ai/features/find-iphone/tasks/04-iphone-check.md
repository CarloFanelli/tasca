# Task 04: iphone-check

## Objective

The Far, Near and Very close thresholds are tuned on the user's iPhone, both behaviors are checked on the device, and the README describes them.

## Context

**Satisfies**: REQ-12

Signal strength depends on the iPhone, the case and the room: only the device can set the thresholds. The disconnection reasons iOS reports when it goes out of range, switches Bluetooth off or powers off are also confirmed here.

## Depends on

Task 02, Task 03

## Expected Behavior

- Next to the iPhone the Find iPhone screen reads Very close; about 5 m away across a room it reads Far.
- Walking away with Find iPhone on starts the Out-of-range alert; switching Bluetooth off on the iPhone does not.
- The README describes the Find iPhone row, the Out-of-range alert and the Find iPhone screen.

## Acceptance Criteria

- [ ] Next to the iPhone the screen reads Very close; about 5 m away it reads Far (REQ-12)
- [ ] Walking out of range starts the alert within 2 s of the reported loss; Bluetooth off on the iPhone does not (REQ-3, REQ-6)
- [ ] The checklist, the thresholds chosen and the reasons seen are recorded in this task's notes (REQ-12)
- [ ] The README describes Find iPhone in Settings, the alert and the Find iPhone screen (REQ-1, REQ-3, REQ-7)

## Edge Cases

- iPhone in a pocket or a bag → readings lower: thresholds chosen so the iPhone in a bag nearby is not Far (REQ-12)
- iPhone powered off → recorded which reason iOS sends; no alert expected (REQ-6)

## Notes from implementation

- **Readings on the user's iPhone** (averaged over 2 s, 2026-11-14): -33 dBm touching its top edge, -55 at 30 cm, -66 at 1 m, -75 to -81 at 3 m and at 5 m alike. The signal is strongest near the top edge, and beyond about 3 m it stops falling.
- **REQ-12 / AC 1 — divergence**: the first thresholds (Very close from -55) made 1 m read Near; at the user's request Very close now starts at -70 (up to about 1 m), Near at -76 (about 2 m), Far below (3 m and beyond); 5 m reads Far, but so does 3 m, since the signal no longer separates them. The new scale is not yet checked on the device: moved to backlog B-12 by the user.
- **REQ-3, REQ-6 — divergence**: the Out-of-range alert and the reasons iOS sends were not tested on the device; moved to backlog B-12 by the user (2026-11-14).

