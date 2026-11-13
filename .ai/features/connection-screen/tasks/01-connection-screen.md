# Task 01: connection-screen

## Objective

From boot until the iPhone's link is secured, the Cardputer shows the Connection screen: "Tasca" in large type, the line that says what to do or what is happening, and the battery level.

## Context

**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-4, REQ-5, REQ-6, REQ-8, REQ-9

The core of the feature: the screen, its three lines, and its closing on the secured link. Reopening on a later disconnection, and the screens it covers, come in Task 02.

## Depends on

None

## Expected Behavior

- At boot the Connection screen is shown instead of the Inbox; the status bar shows only the battery level.
- The line reads "Connecting…" with a bonded iPhone, or while an iPhone is connected but not yet secured.
- The line reads "Connect your iPhone" with no bonded iPhone and none connected.
- After a failed pairing the line reads "Forget Tasca on the iPhone".
- When the link is secured the Connection screen closes and the Inbox is shown, with the full status bar.
- Keys and G0 do nothing on the Connection screen; Fn+Del held 3 s resets pairing and restarts, as today.
- The display timeout applies; a key or G0 with the screen off only turns it on.

## Acceptance Criteria

- [ ] At boot "Tasca" in large type, one line under it and the battery are shown; mode, link state and unread are not (REQ-1, REQ-5)
- [ ] The line text follows bond, connection and pairing state as REQ-2, REQ-3, REQ-4 state, checked by host tests (REQ-2, REQ-3, REQ-4)
- [ ] On the secured link the Connection screen closes and the Inbox with the full status bar is shown (REQ-6)
- [ ] Opt+s, Fn+Enter, Enter, G0 and every other key do nothing on the Connection screen (REQ-8)
- [ ] Fn+Del held 3 s on the Connection screen erases the bond and restarts (REQ-8)
- [ ] The screen turns off after the display timeout; a key or G0 then only turns it on (REQ-9)

## Edge Cases

- Boot with a bonded iPhone out of range → "Connecting…" stays until it comes back (REQ-2, REQ-5)
- Boot with no bond, iPhone starts pairing → line changes to "Connecting…" while pairing, then the screen closes when secured (REQ-2, REQ-3, REQ-6)
- Pairing fails (iPhone remembers an old Tasca) → "Forget Tasca on the iPhone", even if the iPhone disconnects (REQ-4)
- Pairing failed, then the iPhone tries to connect again → the line returns to "Connecting…" and the screen closes if pairing succeeds (REQ-2, REQ-4 · from code)
- Fn+Del held 3 s, after a failed pairing or not → bond erased and restart; after boot the line reads "Connect your iPhone" (REQ-3, REQ-4, REQ-8)
- A second device tries to pair and is refused → line unchanged for the bonded iPhone; the screen stays (REQ-2, REQ-8)
- Battery level unknown → the corner shows the battery as the status bar does today when unknown (REQ-1)
- Link secured but notification sharing off → screen closes; the status bar says notifications are denied, as today (REQ-6)
- Fn+Enter (Siri), Opt+s, G0, Enter on the Connection screen → nothing happens; nothing is logged as sent (REQ-8)
- Fn+Del held less than 3 s → nothing happens (REQ-8)
- Key pressed with the screen off → screen turns on; the key does nothing else (REQ-9)
- Display timeout set to Never → screen stays on (REQ-9)
