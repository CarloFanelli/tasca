# Task 07: mesh-history

## Objective

The last 500 Mesh messages and the known nodes are kept on the microSD card and come back after a restart; Del twice clears every Mesh message.

## Context

**Satisfies**: REQ-21, REQ-22

First use of the microSD card in the firmware. The history survives restarts; without a card the Mesh screen works from memory only.

## Depends on

- Task 05
- Task 06

## Expected Behavior

- Every Mesh message and node change is written to the microSD card; at startup the last 500 messages and the nodes are loaded.
- Messages from before a restart show their stored time, or "--:--" when they had none and no clock is known.
- Without a usable card the Mesh screen title reads "not saved".
- Del on the Mesh screen shows "Press Del again to clear"; a second Del clears every Mesh message on screen and on the card.

## Acceptance Criteria

- [ ] After a restart the Mesh screen shows the messages from before it (REQ-21)
- [ ] The Node list after a restart shows the nodes known before it (REQ-21)
- [ ] Beyond 500 messages the oldest is removed, on screen and card (REQ-21)
- [ ] Del twice clears every Mesh message on screen and on the card (REQ-22)

## Edge Cases

- No microSD card → Mesh works from memory; title reads "not saved" (REQ-21)
- Card full or a write fails → same as no card (REQ-21)
- Stored message without a time, clock not known → "--:--" (REQ-20, REQ-21)
- Del pressed once → "Press Del again to clear", as on the Chat screen (REQ-22)

## Notes from implementation

- **Known limitation**: the microSD card was not tried on the device yet: mounting (pins from the M5Cardputer example, on the shared SPI bus), writing and loading after a restart are checked with the user in Task 08. /dev, 2026-11-19.
- **Known limitation**: the Mesh screen title has room for one note: while the LoRa unit is missing it reads "Connect the LoRa unit", and "not saved" shows only once the unit is there. /dev, 2026-11-19.
- **Known limitation**: a node loaded from the card shows "--" for its age in the Node list until it is heard again, since the time it was last heard is not known after a restart. /dev, 2026-11-19.
