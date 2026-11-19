# Task 03: mesh-send

## Objective

On the Mesh screen, Enter opens a compose line; Enter sends the text on the public channel, and the message shows whether a nearby node heard it.

## Context

**Satisfies**: REQ-9, REQ-10, REQ-11, REQ-12

Writing from the Cardputer's keyboard is why Meshtastic is in Tasca. Sending was never tried with the unit: the device check in Task 08 confirms a node hears it.

## Depends on

- Task 01

## Expected Behavior

- Enter on the Mesh screen opens a compose line at the bottom: keys type their legends, Fn+a/e/i/o/u give à è ì ò ù, Ctrl+e gives é, Del deletes.
- The line shows the bytes left out of 200; `` ` `` closes it without sending.
- Enter sends the text on the public channel and adds it to the list under the user's short name, marked "sent".
- The mark becomes "heard" when a node's rebroadcast of it is received, or "not heard" after 30 s.

## Acceptance Criteria

- [ ] Enter opens the compose line; letters, Fn+vowel and Ctrl+e type as on the iPhone (REQ-9)
- [ ] Enter sends the text and adds it to the list as the user's message (REQ-10)
- [ ] The line shows bytes left and accepts at most 200 bytes; `` ` `` closes without sending (REQ-11)
- [ ] The mark reads "sent", then "heard" on a received rebroadcast, or "not heard" after 30 s (REQ-12)

## Edge Cases

- Enter without the unit → compose line does not open (REQ-2, REQ-9)
- Mesh message arriving while composing → list updates, the line and its text stay (REQ-9)
- Text at exactly 200 bytes → further keys type nothing; accented letters count 2 bytes (REQ-11)
- Enter on an empty line → nothing sent, line stays open (REQ-10)
- Unit unplugged while sending → "not sent" (REQ-10, REQ-12)
- Radio time in the last hour at the 10% limit → send refused, line reads "Radio limit: wait" (REQ-10)
- Own message heard back from a relay → not listed twice; marks it "heard" (REQ-12)
- No node hears it → "not heard" after 30 s; it stays listed (REQ-12)

## Notes from implementation

- **Known limitation**: the send sequence was checked on the unit by hand (sending while listening returns `AT_BUSY_ERROR`, so listening stops first; the unit reports `+EVT:TXP2P DONE` when the packet is out), but no message was sent from Tasca and no "heard" mark was observed yet: that is the device check of Task 08. /dev, 2026-11-19.
