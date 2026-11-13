---
feature: connection-screen
version: 1.0
status: approved
completed: true
base: 400cb1e089af2a71fe909db5e4829ced1c88588a
---

# Connection Screen

## Overview

Until the iPhone is connected, the Cardputer shows the **Connection screen**: "Tasca" in large type, a smaller line under it that says what to do or what is happening, and the battery level in a corner. With a bonded iPhone the line reads "Connecting…"; with none it reads "Connect your iPhone"; after a failed pairing it says to forget Tasca on the iPhone. The screen closes when the link is secured and opens again whenever the iPhone disconnects. While it is shown, the Cardputer does nothing else: keys and G0 are ignored, except the pairing reset.

## Project Context

- BLE phases today: advertising, connected, secured, pairing failed, pairing reset; "pairing failed" stays until restart.
- The Cardputer knows at boot whether an iPhone is bonded (one bond at most).
- Every screen lives on the **Screen stack**; the **Inbox** is at the bottom.
- The status bar shows mode, link state, unread count and battery on every screen today.
- Messages are kept in RAM only; after a reconnection iOS sends the notifications still on the iPhone again.
- Fn+Del held 3 s erases the bond and restarts the Cardputer.

## Functional Requirements

### Content

- REQ-1: The Connection screen shows "Tasca" in large type, one smaller line under it, and the battery level; no other status bar content.
- REQ-2: The line reads "Connecting…" when an iPhone is bonded, or one is connected but the link is not yet secured.
- REQ-3: The line reads "Connect your iPhone" when no iPhone is bonded and none is connected.
- REQ-4: After a failed pairing the line reads "Forget Tasca on the iPhone" until the Cardputer restarts.

### When it shows

- REQ-5: The Connection screen is shown from boot until the link is secured.
- REQ-6: When the link is secured, the Connection screen closes and the screen under it is shown again.
- REQ-7: When the iPhone disconnects after that, the Connection screen opens again over whatever was showing.

### Keys and display

- REQ-8: On the Connection screen keys and G0 do nothing, except Fn+Del held 3 s, which resets pairing as today.
- REQ-9: The display timeout applies on the Connection screen; a key or G0 with the screen off only turns it on.

### Documentation

- REQ-10: The README describes the Connection screen and what each of its three lines means.

## Non-Functional Requirements

- **Performance** — The Connection screen opens or closes within 200 ms of the link change.
- **Flash** — The firmware grows by at most 4 KB.
- **Security** — Nothing new reaches the serial log; the iPhone's name is never shown or logged.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| Boot with a bonded iPhone out of range | "Connecting…" stays until it comes back | REQ-2, REQ-5 |
| Boot with no bond, iPhone starts pairing | Line changes to "Connecting…" while pairing, then the screen closes when secured | REQ-2, REQ-3, REQ-6 |
| Bond kept on the Cardputer, forgotten on the iPhone | "Connecting…" stays; the README says to use Fn+Del to pair again | REQ-2, REQ-10 |
| Pairing fails (iPhone remembers an old Tasca) | "Forget Tasca on the iPhone" until restart, even if the iPhone disconnects | REQ-4 |
| Fn+Del held 3 s, after a failed pairing or not | Bond erased and restart as today; after boot the line reads "Connect your iPhone" | REQ-3, REQ-4, REQ-8 |
| A second device tries to pair and is refused | Line unchanged for the bonded iPhone; the screen stays | REQ-2, REQ-8 |
| Battery level unknown | Corner shows the battery as the status bar does today when unknown | REQ-1 |
| Link secured but notification sharing off | Screen closes; the status bar says notifications are denied, as today | REQ-6 |
| iPhone disconnects in Keyboard mode | Keys held are released; after reconnection Keyboard mode is back | REQ-7 |
| iPhone disconnects with a Chat screen open | After reconnection the Chat screen is back if its Chat still exists, else the Inbox | REQ-6, REQ-7 |
| iPhone disconnects on Settings with a confirmation pending | The confirmation is cancelled; Settings is back after reconnection | REQ-7 |
| iPhone disconnects during an incoming call | The Call screen closes and the Connection screen shows | REQ-7 |
| iPhone disconnects or reconnects with the screen off | The screen stays off; the new state shows when it is turned on | REQ-7, REQ-9 |
| Fn+Enter (Siri), Opt+s, G0, Enter on the Connection screen | Nothing happens; nothing is logged as sent | REQ-8 |
| Fn+Del held less than 3 s | Nothing happens | REQ-8 |
| Key pressed with the screen off | Screen turns on; the key does nothing else | REQ-9 |
| Display timeout set to Never | Screen stays on | REQ-9 |
| Short disconnection (a few seconds) | Connection screen shown for that time, then closed | REQ-6, REQ-7 |
| N/A — concurrent: one link and one screen at a time; link events reach the screen through the main loop | — | REQ-5, REQ-6, REQ-7 |
| N/A — permission: no user roles on the device | — | REQ-1, REQ-8 |
| N/A — integrity: the feature stores nothing | — | REQ-1, REQ-10 |

## Out of Scope

- **Reading messages while disconnected** — chosen: the Connection screen covers everything until the iPhone is back.
- **Opening Settings without an iPhone** — chosen: no keys on the Connection screen; the pairing reset stays on Fn+Del.
- **Showing the iPhone's name** — the Cardputer does not know it reliably, and it is personal data.
- **Logo or animation** — "Tasca" in large type is enough for now.
