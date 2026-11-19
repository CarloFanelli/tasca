# Task 02: menu-without-iphone

## Objective

Without the iPhone, Fn+G0 opens the Menu over the Connection screen so the Mesh screen and Settings stay reachable, and the Mesh screen is not covered when the iPhone leaves.

## Context

**Satisfies**: REQ-4, REQ-5, REQ-7

Meshtastic matters most when the iPhone is away; today the Connection screen blocks every screen and Fn+G0. This task changes only how the Menu and the Connection screen meet.

## Depends on

- Task 01

## Expected Behavior

- On the Connection screen, Fn+G0 opens the Menu; G0 alone still does nothing there.
- From the Menu over the Connection screen, Mesh and Settings open; Music, Find iPhone and WhatsApp do nothing.
- `` ` `` on a screen opened there goes back to the Connection screen.
- While the Mesh screen or the Node list is on top, a lost iPhone link does not bring up the Connection screen; it appears once the user leaves them.

## Acceptance Criteria

- [ ] Fn+G0 on the Connection screen opens the Menu; G0 alone does nothing (REQ-4)
- [ ] Without the iPhone only Mesh and Settings open from the Menu (REQ-4)
- [ ] `` ` `` from the Mesh screen opened without the iPhone shows the Connection screen (REQ-7)
- [ ] Losing the iPhone link while on the Mesh screen keeps the Mesh screen on top (REQ-5)
- [ ] Leaving the Mesh screen with no link shows the Connection screen (REQ-5)

## Edge Cases

- Mesh screen opened from the Connection screen, then the iPhone connects → Mesh screen stays; `` ` `` goes back to the Inbox (REQ-4, REQ-7)
- Link lost on the Mesh screen → Out-of-range alert beeps as usual (REQ-5)
- A key on the Mesh screen while the Out-of-range alert beeps → silences it, as on the Connection screen (REQ-5 · from code)
- Music, Find iPhone or WhatsApp chosen without the iPhone → nothing happens (REQ-4)
