# Task 03: readme-device-check

## Objective

The README describes the Connection screen, and the whole feature is checked on the Cardputer with the user's iPhone.

## Context

**Satisfies**: REQ-10

Last: the README describes behavior verified on the device, and the checklist confirms Tasks 01 and 02 together.

## Depends on

Task 02

## Expected Behavior

- The README's "Using the Cardputer" part explains the Connection screen and what each of its three lines means and asks the user to do.
- The README says that "Connecting…" that never ends, with a bond the iPhone has forgotten, is fixed with Fn+Del and pairing again.
- The user runs a device checklist covering boot with and without bond, failed pairing, disconnection from each screen, and screen off.

## Acceptance Criteria

- [ ] The README names the Connection screen and its three lines with what each means (REQ-10)
- [ ] The README tells how to recover from "Connecting…" when the iPhone forgot Tasca (REQ-10)
- [ ] The device checklist and its outcome are recorded in this task's notes (REQ-10)

## Edge Cases

- Bond kept on the Cardputer, forgotten on the iPhone → "Connecting…" stays; the README says to use Fn+Del to pair again (REQ-2, REQ-10)

## Notes from implementation

- **Device check (2026-11-13)**: on the user's iPhone, with a bonded iPhone, boot showed "Tasca / Connecting…" then the Inbox; turning Bluetooth off and on reopened and closed the Connection screen from the Inbox, a Chat screen and Keyboard mode, each back as it was; keys and G0 did nothing on it; the screen timed out and a key only woke it. The serial log showed 5 openings and 5 closings matching the phase changes, no W or E line. "Connect your iPhone" and "Forget Tasca on the iPhone" need the bond erased: left to the incoming-calls check with another phone.
- **Flash**: +2,468 bytes over the build before Task 01 (2,027,347 → 2,029,815). A FreeSansBold 18 pt font for "Tasca" cost about 9 KB: efontCN_16 is scaled 5 times instead and drawn 2 × 2 times, one font pixel apart, for strokes 2 font pixels thick: at 3 font pixels the letters merge; the user compared both on the device and chose this one.
