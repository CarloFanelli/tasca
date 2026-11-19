# Task 04: mesh-settings

## Objective

In Settings the user writes the Mesh name the Cardputer announces to other nodes and turns the Mesh sound on or off; a received Mesh message beeps and wakes the screen.

## Context

**Satisfies**: REQ-14, REQ-15, REQ-16, REQ-17, REQ-18

The name makes the user recognisable on the channel and needs the sending of Task 03. The Settings screen has no text row today: the Mesh name is the first.

## Depends on

- Task 03

## Expected Behavior

- Settings lists "Mesh sound" (On by default) and "Mesh name" before the actions.
- Enter on "Mesh name" edits it in place with the keyboard, up to 24 characters; Enter saves, `` ` `` cancels. Both are kept across restarts.
- With no name set the Mesh name is "Tasca" plus 4 digits of the node ID; the short name is the first 4 characters.
- The Cardputer announces its names when the unit is detected, when the name changes, and every 3 hours.
- Each received Mesh message beeps when Mesh sound is on, whatever the sound row says, and wakes the screen when wake on new message is on.

## Acceptance Criteria

- [ ] Mesh name edited, saved and kept after a restart; `` ` `` leaves it unchanged (REQ-14)
- [ ] Short name is the first 4 characters; with no name, "Tasca" plus 4 node-ID digits (REQ-15)
- [ ] The names are announced at detection, on change and every 3 hours (REQ-16)
- [ ] Mesh sound On beeps per Mesh message even with sound off; Off never beeps for Mesh (REQ-17)
- [ ] A Mesh message wakes the screen when wake on new message is on (REQ-18)

## Edge Cases

- Name cleared and saved → default name used and announced (REQ-14, REQ-15)
- Name at 24 characters → further keys type nothing (REQ-14)
- Name shorter than 4 characters → short name is the whole name (REQ-15)
- Name changed without the unit → saved; announced when the unit is detected (REQ-16)
- Mesh sound off, sound on → Mesh messages silent, WhatsApp messages beep (REQ-17)
- Mesh sound on, sound off → Mesh messages beep, WhatsApp messages silent (REQ-17)
- Mesh message during a call → no beep, as WhatsApp messages (REQ-17 · from code)

## Notes from implementation

- **Known limitation**: Meshtastic nodes drop an announcement whose short name passes 4 bytes or long name 39 bytes, so the short name is the first 4 characters that fit in 4 bytes (an accented letter takes 2: "Nèèè" gives "Nè") and the Mesh name entry also stops at 39 bytes, which only bites before 24 characters with many accented letters. /dev, 2026-11-19.
- **Known limitation**: the announcement was not sent from the device yet: the build that announces was not flashed, so the first transmission happens with the user in Task 08. /dev, 2026-11-19.
