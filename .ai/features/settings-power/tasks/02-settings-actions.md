# Task 02: settings-actions

## Objective
Settings offers "Reset pairing" and "Clear Messages", each run only after a second Enter on a confirmation prompt.

## Context
**Satisfies**: REQ-6, REQ-7
The two destructive actions of the project spec, guarded against a stray key. Reset pairing does what Fn+Del already does; Clear Messages keeps the pairing.

## Depends on
01-settings-screen

## Expected Behavior
- Enter on an action shows "Press Enter to confirm"; Enter again runs it.
- Any other key, or 3 s without Enter, cancels the prompt.
- Reset pairing erases the bond and restarts the Cardputer.
- Clear Messages empties every Message; the Inbox is empty when leaving Settings.

## Acceptance Criteria
- [ ] An action runs only after Enter, then Enter again on the prompt (REQ-6)
- [ ] Another key or 3 s without Enter cancels the prompt (REQ-6)
- [ ] Reset pairing erases the bond and restarts, like Fn+Del (REQ-7)
- [ ] Clear Messages empties the Messages and keeps the pairing (REQ-7)

## Edge Cases
- `,`/`/` on an action → ignored (REQ-2, REQ-6)
- Reset pairing with no bond → restart anyway; nothing to erase (REQ-7)
- Live Message during the confirmation prompt → prompt stays; Message handled as usual (REQ-6, REQ-8)
- Clear Messages entered from a Chat screen → back to the empty Inbox on leaving Settings (REQ-7)
- Cleared Messages still open on the iPhone → they return at the next reconnection (REQ-7)
