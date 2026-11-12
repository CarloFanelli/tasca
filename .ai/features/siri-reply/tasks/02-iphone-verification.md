# Task 02: iphone-verification

## Objective
The user knows, from a test on their own iPhone, whether Siri can be started from the Cardputer and a WhatsApp reply sent by voice; the README says so.

## Context
**Satisfies**: REQ-4, REQ-5, REQ-6, REQ-7
The spike itself: a hardware test with the user, unlocked and locked, with and without earphones. If the first command does not start Siri, other commands are tried before giving up.

## Depends on
01-siri-key

## Expected Behavior
- With the iPhone unlocked, then locked, Fn+Enter is tried and the result recorded; when it fails, other commands are tried in turn.
- A WhatsApp message is dictated to Siri, with and without earphones; whether it is sent is recorded.
- The README states what works, the iPhone settings needed (Siri when locked), and that the Cardputer cannot act as microphone.
- If no command starts Siri, Fn+Enter is removed and the README says so.

## Acceptance Criteria
- [ ] Result recorded for the iPhone unlocked and locked, with the command that works (REQ-4)
- [ ] Result recorded for a WhatsApp message dictated, with and without earphones (REQ-5)
- [ ] README states what works, the iPhone settings needed, and that the Cardputer is no microphone (REQ-6)
- [ ] With no working command, Fn+Enter is gone and the README says Siri cannot be started (REQ-7)

## Edge Cases
- Siri disabled on the iPhone, or not allowed when locked → nothing visible on the Cardputer; the README lists the setting (REQ-4, REQ-6)
- Siri asks for confirmation ("Send it?") → answered by voice (REQ-5)
- WhatsApp contact name ambiguous for Siri → Siri asks which contact; outcome recorded (REQ-5)
- No earphones and iPhone in a pocket → Siri uses the iPhone's microphone; recorded whether it understands (REQ-4, REQ-5)
- N/A — concurrent: one tester, one request at a time (REQ-4)
- N/A — integrity: no data stored by this feature (REQ-5, REQ-6, REQ-7)

## Notes from implementation

- **Siri command**: a tap of the consumer Voice Command usage only wakes the iPhone; holding it down while Enter is held starts Siri, unlocked and locked (tested 2026-11-12).
- **Known limitation**: with notifications on the iPhone's lock screen, the first Fn+Enter only lights the screen; a second Fn+Enter held starts Siri. A tap before the hold, sent automatically, is the planned fix, not yet built.
- **Known limitation**: sending a WhatsApp message through Siri asks to unlock the iPhone; the passcode typed from the Cardputer after Siri's request is still to be verified in Keyboard mode.
