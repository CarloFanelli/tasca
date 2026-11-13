# Task 02: reopen-on-disconnect

## Objective

When the iPhone disconnects after the link was secured, the Connection screen opens again over whatever was showing; when the link is secured again, the user is back where they were.

## Context

**Satisfies**: REQ-6, REQ-7

The user chose that the Connection screen covers everything while the iPhone is away. This task handles what it covers: Keyboard mode, a Chat screen, the Settings screen with a pending confirmation, the Call screen.

## Depends on

Task 01

## Expected Behavior

- A disconnection opens the Connection screen over the screen on top, with "Connecting…".
- When the link is secured again, the Connection screen closes and the screen that was under it is shown again.
- Keyboard mode under it releases held keys when covered and is back, ready, after reconnection.
- A Call screen ringing at the disconnection closes; the Connection screen shows.
- The disconnection and the reconnection never turn the screen on.

## Acceptance Criteria

- [ ] A disconnection on the Inbox, a Chat screen, Settings or Keyboard mode opens the Connection screen over it (REQ-7)
- [ ] After reconnection the covered screen is shown again, unchanged, as it was before the disconnection (REQ-6)
- [ ] Keys held in Keyboard mode are released at the disconnection; after reconnection held keys are not typed (REQ-7)
- [ ] A ringing Call screen closes at the disconnection and does not come back after reconnection (REQ-7)
- [ ] With the screen off, a disconnection or reconnection leaves it off; the right screen shows when turned on (REQ-7)

## Edge Cases

- iPhone disconnects in Keyboard mode → keys held are released; after reconnection Keyboard mode is back (REQ-7)
- iPhone disconnects with a Chat screen open → after reconnection the Chat screen is back if its Chat still exists, else the Inbox (REQ-6, REQ-7)
- iPhone disconnects on Settings with a confirmation pending → the confirmation is cancelled; Settings is back after reconnection (REQ-7)
- iPhone disconnects during an incoming call → the Call screen closes and the Connection screen shows (REQ-7)
- iPhone disconnects or reconnects with the screen off → the screen stays off; the new state shows when it is turned on (REQ-7)
- Short disconnection (a few seconds) → Connection screen shown for that time, then closed (REQ-6, REQ-7)
