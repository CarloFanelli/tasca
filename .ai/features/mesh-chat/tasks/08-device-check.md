# Task 08: device-check

## Objective

The Mesh chat is checked on the Cardputer with the LoRa unit in Leno, and the README explains it, the privacy of the history included.

## Context

**Satisfies**: REQ-1, REQ-10, REQ-12, REQ-16

Receiving was proven with a test firmware; sending, the announcement and the full flow were not. The user takes part: outdoors if indoors hears nothing.

## Depends on

- Task 02
- Task 03
- Task 04
- Task 07

## Expected Behavior

- On the device: the unit is detected, public-channel messages appear, a sent message gets "heard", the name is announced.
- The README gains a Mesh section: what it needs, how to reach it, keys, Settings rows, limits and that the history on the card is readable by anyone holding it.

## Acceptance Criteria

- [ ] Unit detected on the device, plugged at startup and while running (REQ-1)
- [ ] A message sent from the Cardputer is marked "heard" by a nearby node (REQ-10, REQ-12)
- [ ] The announced name is seen on a received node announcement or an online map (REQ-16)
- [ ] README describes the Mesh chat and the microSD history's privacy (REQ-21)

## Edge Cases

- Nothing heard indoors → repeat outdoors; record where it worked in the task notes (REQ-12)
- Sent message never "heard" → record it, the feature still reads (REQ-12)

## Notes from implementation

- **Known limitation**: checked indoors in Leno on 2026-11-19: the LoRa unit was found at startup, the name was announced, packets from nearby nodes arrived at about -105 to -110 dBm (SNR -11 to -15), the iPhone's time was read (messages showed "14:25"), and messages and nodes came back from the microSD card after a restart. The message sent from the Cardputer left the radio but read "not heard": no node repeated it, likely because the nearest nodes are at the edge of reach; to repeat outdoors. /dev, 2026-11-19.
- **Known limitation**: the announced name was not yet seen on another node or an online map. /dev, 2026-11-19.
