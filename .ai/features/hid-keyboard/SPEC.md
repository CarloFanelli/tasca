---
feature: hid-keyboard
version: 1.2
status: approved
completed: true
base: c3a711fe9d8583d6ff4ce6da87b44c7f5ea4ed79
---

# HID Keyboard (M2)

## Overview

Turns the Cardputer into a Bluetooth keyboard for the paired iPhone, on the same connection that
delivers notifications. G0 switches between **Reading mode** (keys stay on the Cardputer) and
**Keyboard mode** (keys go to the iPhone). Keys are sent by position and the iPhone's hardware layout
("Automatic", which behaves as U.S.) makes each key type its keycap legend; Italian accented
letters come from Fn and Ctrl combinations that the Cardputer turns into Option dead-key
sequences. The screen echoes the line being typed. It retires the second technical risk of the project: HID and ANCS together on
one link, stable for 30 minutes.

## Project Context

- Builds on ancs-capture: bonded link, HID service already in the GATT table (inert), ANCS dumps on serial.
- The GATT table must not change: iOS cached it at pairing.
- The Cardputer keycaps carry US legends; the iPhone hardware layout "Automatic" types them as printed.
- On that layout iOS writes accents with Option dead keys: Option+`` ` `` then e gives è, Option+e then e gives é (verified).
- Built-in display fonts are ASCII-only; Italian accented characters need a font that covers them.
- Concurrency rules in CLAUDE.md apply: HID reports are sent from the main loop only.
- Fn+Del (3 s) already erases the bond and restarts the board.

## Functional Requirements

### Modes

- REQ-1: At boot the Cardputer is in Reading mode; in Reading mode no key reaches the iPhone.
- REQ-2: Pressing G0 toggles Reading and Keyboard mode; the screen always shows the current mode.
- REQ-3: Leaving Keyboard mode first releases every key on the iPhone.
- REQ-4: The Fn+Del pairing reset works only in Reading mode.

### Sending keys

- REQ-5: In Keyboard mode every key press and release is sent as an HID keyboard report identifying the key position.
- REQ-6: Shift, Ctrl and Opt are sent as Shift, Control and Option; Alt is sent as Command.
- REQ-7: Fn with `;` `,` `.` `/` sends Up, Left, Down, Right; Fn+`` ` `` sends Esc; Del sends Backspace.
- REQ-8: Fn+Space sends the consumer Eject usage, which shows or hides the iPhone on-screen keyboard.
- REQ-9: Fn combined with any key not listed in REQ-7, REQ-8 or REQ-18 sends nothing.
- REQ-10: A held key stays pressed on the iPhone until released; auto-repeat is left to the iPhone.

### Echo and status

- REQ-11: In Keyboard mode the screen shows the current line as typed: keycap characters, Shift included, and accented letters.
- REQ-12: Backspace removes the last echoed character; Enter starts an empty line.
- REQ-13: In Keyboard mode without a secured link ready for keyboard reports, the screen says so and key presses are discarded.

### Coexistence and logging

- REQ-14: While typing, ANCS keeps working: every notification is still dumped on serial.
- REQ-15: 30 minutes of typing with notifications arriving cause no disconnection.
- REQ-16: In Keyboard mode the serial log never contains typed characters, only mode changes, errors and a count of keys sent.
- REQ-17: CLAUDE.md states the required iPhone setting: hardware keyboard layout "Automatic" (or U.S.).

### Accented letters

- REQ-18: Fn+a, e, i, o, u type à, è, ì, ò, ù; with Shift, À, È, Ì, Ò, Ù.
- REQ-19: Ctrl+e types é; with Shift, É.

## Data & Dependencies

### Information the feature must retain

- Current mode — Reading at boot, changed by G0, never persisted.
- Echo line — the characters typed since the last Enter, cleared when leaving Keyboard mode or after 10 s without a key; RAM only.

### External capabilities relied on

- HID over GATT on the iPhone — receives keyboard and consumer-control reports.
- iPhone hardware keyboard layout "Automatic" (U.S.) — turns key positions into the keycap characters and supports Option dead keys.

## Non-Functional Requirements

- **Performance** — A key press reaches the iPhone within 50 ms, outside the notification flood at connection.
- **Security** — Typed text is never written to serial, SD or flash.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| Key pressed in Reading mode | Nothing sent; Reading behavior only (key log as today) | REQ-1 |
| Key already held when Keyboard mode starts | Ignored until released; next press is sent | REQ-2 |
| G0 pressed while keys are held in Keyboard mode | All keys released on the iPhone, then Reading mode | REQ-3 |
| G0 pressed rapidly several times | Each press toggles once; no key is left pressed on the iPhone | REQ-2, REQ-3 |
| Fn+Del held 3 s in Keyboard mode | Nothing sent, no reset | REQ-4, REQ-9 |
| More than 6 non-modifier keys down at once | The extra keys are ignored until a slot frees | REQ-5 |
| Modifier alone (e.g. Shift) pressed and released | Sent as modifier press and release; no character, no echo | REQ-6 |
| Arrow, Esc, Tab, or a key with Ctrl/Command | Sent; echo line unchanged | REQ-7, REQ-11 |
| iOS ignores the Eject usage | Nothing happens on the iPhone; noted for the on-screen keyboard fallback | REQ-8 |
| Fn+Space held | One Eject toggle per press | REQ-8 |
| Opt combination typed by hand (e.g. Opt+e, a dead key) | Sent; echo shows a placeholder character | REQ-11 |
| Echo line longer than the screen width | Screen shows the end of the line | REQ-11 |
| Backspace on an empty echo line | Sent to the iPhone; echo stays empty | REQ-12 |
| No key pressed for 10 s (e.g. the user switched app or text field on the iPhone, which the keyboard cannot detect) | Echo line cleared | REQ-12 |
| Display failed to initialize | Mode and echo not shown; keys still sent | REQ-2, REQ-11 |
| iPhone disconnects while keys are held | Keys discarded; status shows not connected; nothing re-sent on reconnect | REQ-13 |
| iPhone reconnects while in Keyboard mode | Keys are sent again once the link is secured; echo line kept | REQ-13 |
| iPhone connected but keyboard reports not enabled yet | Treated as not ready: status shown, keys discarded | REQ-13 |
| Sending a report fails | `W` line without the key, key dropped, typing continues | REQ-5, REQ-16 |
| Typing during the PreExisting notification flood at connection | Keys may be delayed up to 1 s; none lost or reordered | REQ-14 |
| Notification arrives mid-word | Dumped on serial; typing and echo unaffected | REQ-14 |
| iOS renegotiates the connection during the 30-minute run | Allowed if the link stays up; a disconnection fails the test | REQ-15 |
| Log level set to Debug | Still no typed characters in Keyboard mode | REQ-16 |
| N/A — concurrent/permission: one user, one bonded iPhone | — | REQ-17 |
| Fn+vowel or Ctrl+e held | One accented letter per press, no repeat | REQ-18, REQ-19 |
| Fn+vowel pressed while another key is held | The accented letter is typed; the held key is released on the iPhone and counts again only once pressed anew | REQ-18 |
| Fn+Ctrl+e | Nothing sent (Fn combination not listed) | REQ-9, REQ-19 |
| Accent combination while the iPhone is not ready | Discarded like any key | REQ-13, REQ-18, REQ-19 |
| iPhone layout set to a non-U.S. layout (e.g. Italian) | Keys type that layout's characters and accent sequences may differ; CLAUDE.md says how to fix the setting | REQ-17, REQ-18 |
| Ctrl+e as the iOS "end of line" shortcut | No longer available from the Cardputer: Ctrl+e types é | REQ-19 |

## Out of Scope

- **3-second banner for messages arriving in Keyboard mode** — needs the message store and parser (M3/M4).
- **Reading-mode navigation (inbox, chat)** — the interface milestone; Reading mode only blocks keys here.
- **Italian hardware layout on the iPhone** — decided: U.S./Automatic layout so keycaps match, accents through combinations.
- **Other accented letters (ó, ú, ç, …)** — Italian needs only the grave vowels and é.
- **Cardputer-side key repeat, macros, text buffers** — the iPhone repeats held keys; nothing else is needed to type.
- **Settings screen** — milestone M5.
