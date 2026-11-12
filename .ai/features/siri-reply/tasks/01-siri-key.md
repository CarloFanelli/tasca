# Task 01: siri-key

## Objective
Fn+Enter asks the iPhone to start Siri from Reading mode or Keyboard mode; the screen stays as it is.

## Context
**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-8
The key comes first so that the spike in Task 02 can try it on the iPhone. The command tried first is the Bluetooth keyboard's assistant (Voice Command) command; Task 02 may change it.

## Depends on
None

## Expected Behavior
- Fn+Enter sends the Siri command to the iPhone, in Reading mode and in Keyboard mode.
- From Reading mode (Inbox, Chat screen or Settings screen) the screen and mode stay as they are.
- Fn+Enter types nothing on the iPhone and never opens a Chat screen or confirms an action.
- Enter without Fn behaves as today.

## Acceptance Criteria
- [ ] Fn+Enter in Keyboard mode sends the Siri command and no key (REQ-1, REQ-3)
- [ ] Fn+Enter in Reading mode sends the Siri command and stays on the same screen (REQ-1, REQ-2)
- [ ] Fn+Enter on the Inbox never opens the selected Chat (REQ-3)
- [ ] Enter without Fn is unchanged in both modes (REQ-3)
- [ ] The first ` after Fn+Enter, within 60 s, sends Esc to the iPhone and nothing else (REQ-8)

## Edge Cases
- iPhone not ready (not connected or keyboard not enabled) → nothing sent; `W` line; Keyboard mode shows "iPhone not ready" as today (REQ-1, REQ-2)
- Fn+Enter held → one request only; no repeat while held (REQ-1)
- Fn+Enter pressed again while Siri is listening → sent again; iOS decides (REQ-1)
- Fn+Enter with the screen off → in Reading mode only wakes the screen; in Keyboard mode wakes it and starts Siri (REQ-1, REQ-2)
- Fn+Enter on the Settings screen or in a Chat screen → starts Siri; the screen stays as it is (REQ-2, REQ-3)
- Fn+Enter while an action waits for confirmation → prompt cancelled; Siri started (REQ-2, REQ-3)
- Enter without Fn → unchanged: opens a Chat, confirms an action, or sends Enter in Keyboard mode (REQ-3)
- Another key before ` after Fn+Enter, or ` after 60 s → ` back to its usual use (REQ-8)
- ` after Fn+Enter with the iPhone not ready → nothing sent; `W` line (REQ-8)
- N/A — concurrent: one user, one key at a time (REQ-1)

## Notes from implementation

- **REQ-2 — divergence**: Fn+Enter no longer switches Reading mode to Keyboard mode. Resolution: the user dropped Type to Siri (it replaces the spoken Siri on the iPhone), so the switch had no use; SPEC 1.1 states the screen stays as it is (2026-11-12).
- **REQ-8 added**: the user asked that ` alone close Siri (2026-11-12); SPEC 1.2 bounds it to the first key after Fn+Enter, within 60 s, since iOS does not tell when Siri closes.
