# Task 01: mesh-receive

## Objective

With the LoRa unit on the Grove port, the Menu lists Mesh with its unread count, and the Mesh screen shows the public channel's text messages as they arrive.

## Context

**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-6, REQ-7, REQ-8, REQ-20

The base every other task builds on: finding the unit, setting its radio to the Meshtastic public channel, decoding packets and showing them. Until the clock arrives in Task 06, a message shows the time elapsed since it arrived.

## Depends on

None

## Expected Behavior

- At startup and while running, Tasca checks the Grove port for the LoRa unit; once found, it sets the MediumFast radio parameters and listens continuously.
- The Menu lists "Mesh" after Find iPhone, with the count of unread Mesh messages; Enter opens the Mesh screen in Reading mode.
- The Mesh screen lists text messages of the public channel, newest at the bottom, each with elapsed time ("5m"), sender's short name and text, wrapped over lines.
- `;` and `.` scroll; `` ` `` goes back to the Inbox. Opening the Mesh screen marks every Mesh message read.
- Without the unit the Mesh screen reads "Connect the LoRa unit".

## Acceptance Criteria

- [ ] The unit is detected within 5 s of startup or of being plugged in, without a restart (REQ-1)
- [ ] Without the unit the Mesh screen reads "Connect the LoRa unit" (REQ-2)
- [ ] The Menu shows "Mesh" after Find iPhone with the unread count; Enter opens the Mesh screen (REQ-3)
- [ ] A public-channel text appears with elapsed time, short name and text, newest at the bottom (REQ-6, REQ-20)
- [ ] `;`/`.` scroll; `` ` `` goes back; opening marks all read (REQ-7, REQ-8)

## Edge Cases

- Unit unplugged while running → within 5 s "Connect the LoRa unit"; messages stay (REQ-1, REQ-2)
- Another device on the Grove port → not taken for the unit (REQ-1)
- No message yet → "No messages yet" (REQ-6)
- Same message relayed by several nodes → shown once (REQ-6)
- Position, telemetry and node announcements → not listed; they update the known nodes (REQ-6, REQ-13)
- Packets of another channel, or direct messages → ignored (REQ-6)
- Sender not yet announced → short name "!" plus the last 4 digits of its node ID (REQ-6, REQ-13)
- Emoji and other characters the screen cannot draw → shown as on the Chat screen (REQ-6)
- Mesh message arriving on another screen → unread count in the Menu rises (REQ-3)
