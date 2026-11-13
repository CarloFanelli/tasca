# Task 02: keyboard-mode-screen

## Objective
Keyboard mode runs as a screen on the **Screen stack**, in its own source file: G0 opens it over the current screen and closes it back there, with everything behaving as before.

## Context
**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-4, REQ-5, REQ-6
Keyboard mode is the screen that opens over any other, as the Call screen and Menu will: moving it proves the stack for them. Shares REQ-1 to REQ-6 with Task 01, for Keyboard mode.

## Depends on
01-reading-screens-stack

## Expected Behavior
- G0 opens Keyboard mode over the Inbox, Chat screen or Settings screen, and G0 again returns to that screen, unchanged.
- Keyboard mode looks as before: context Message or Banner, mode line, echo line.
- Keys, Fn+Space, Fn+Enter, the ` after Siri, the echo timeout, beeps and the Banner behave as before.
- No typed text reaches the serial log.

## Acceptance Criteria
- [ ] Keyboard mode lives in its own source file with draw, key and update entry points (REQ-4)
- [ ] G0 opens Keyboard mode over the current screen and closes it back to that screen (REQ-5, REQ-6)
- [ ] Every Keyboard mode key, combination and G0 does what it did before (REQ-1)
- [ ] Keyboard mode looks as before, status bar included (REQ-2)
- [ ] Banner, beeps, display timeout and Siri behave as before in Keyboard mode (REQ-3)

## Edge Cases
- Key or G0 with the screen off → in Keyboard mode the key also reaches the iPhone; G0 only turns the screen on (REQ-1, REQ-3)
- G0 on the Settings screen → Keyboard mode opens over it; G0 again returns to the Settings screen (REQ-1, REQ-5)
- G0 on a Chat screen → Keyboard mode shows that Chat's last Message as context; G0 returns to the Chat screen (REQ-1, REQ-5)
- Settings action waiting for confirmation when Keyboard mode opens over it → prompt cancelled (REQ-1, REQ-5)
- Messages arrive while Keyboard mode covers a Chat screen → that Chat's Messages marked read only when back on top (REQ-3, REQ-5)
- Live Message in Keyboard mode → Banner and beep as today (REQ-3)
- Fn+Enter (Siri) and ` right after it → same as today; ` closes Siri instead of being typed (REQ-1, REQ-3)
- N/A — permission: no new action reaches the iPhone (REQ-1, REQ-4)
