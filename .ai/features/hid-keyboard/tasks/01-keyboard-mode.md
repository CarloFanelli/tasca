# Task 01: keyboard-mode

## Objective

With G0 the user switches the Cardputer into Keyboard mode and types on the iPhone, modifiers, arrows, Esc, Backspace and Italian accented letters included; G0 again returns to Reading mode, where nothing reaches the iPhone.

## Context

**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-4, REQ-5, REQ-6, REQ-7, REQ-9, REQ-10, REQ-13, REQ-16, REQ-18, REQ-19

The first time the HID service sends anything. Everything else in this feature (echo, Eject, soak test) builds on keys reaching the iPhone and on the mode switch.

## Depends on

- None

## Expected Behavior

- At boot the Cardputer is in Reading mode and the screen shows it; keys stay on the Cardputer.
- G0 toggles Reading mode and Keyboard mode; the screen shows the current mode and whether the iPhone is ready for keys.
- In Keyboard mode every press and release goes to the iPhone by key position, with Shift, Control, Option and Command (Alt).
- Fn + `;` `,` `.` `/` send the arrows, Fn+`` ` `` sends Esc, Del sends Backspace; other Fn combinations send nothing.
- Fn+a, e, i, o, u type à, è, ì, ò, ù and Ctrl+e types é (uppercase with Shift), as Option dead-key sequences.
- Leaving Keyboard mode releases every key on the iPhone first.
- The Fn+Del pairing reset works only in Reading mode.
- In Keyboard mode the serial log shows mode changes, errors and a count of keys sent, never the keys.

## Acceptance Criteria

- [x] After boot, letters typed reach the iPhone only after G0 switches to Keyboard mode (REQ-1, REQ-2)
- [x] With the iPhone layout on "Automatic", every key types its keycap legend in Notes, Shift included (REQ-5, REQ-6)
- [x] Fn+vowel types à è ì ò ù, Ctrl+e types é, Shift gives the uppercase; one letter per press (REQ-18, REQ-19)
- [x] Fn arrows, Esc, Backspace and Alt as Command work in Notes; Fn+a sends nothing (REQ-7, REQ-9)
- [x] A held key repeats on the iPhone; leaving Keyboard mode with keys held leaves none stuck (REQ-3, REQ-10)
- [x] Without a ready iPhone the screen says so and keys are discarded (REQ-13)
- [x] In Keyboard mode the serial log contains no typed character; Fn+Del does not reset pairing (REQ-4, REQ-16)
- [x] Host tests cover key-to-report mapping: modifiers, Fn layer, accent sequences, unmapped Fn combos, more than 6 keys (REQ-5, REQ-6, REQ-7, REQ-9, REQ-18, REQ-19)

## Edge Cases

- Key pressed in Reading mode → nothing sent; Reading behavior only (key log as today) (REQ-1 from the spec edge-case table)
- Key already held when Keyboard mode starts → ignored until released; next press is sent (REQ-2 from the spec edge-case table)
- G0 pressed while keys are held in Keyboard mode → all keys released on the iPhone, then Reading mode (REQ-3 from the spec edge-case table)
- G0 pressed rapidly several times → each press toggles once; no key left pressed on the iPhone (REQ-2, REQ-3 from the spec edge-case table)
- Fn+Del held 3 s in Keyboard mode → nothing sent, no reset (REQ-4, REQ-9 from the spec edge-case table)
- More than 6 non-modifier keys down at once → the extra keys are ignored until a slot frees (REQ-5 from the spec edge-case table)
- Modifier alone pressed and released → sent as modifier press and release (REQ-6 from the spec edge-case table)
- Display failed to initialize → mode not shown; keys still sent (REQ-2 from the spec edge-case table)
- iPhone disconnects while keys are held → keys discarded; status shows not connected; nothing re-sent on reconnect (REQ-13 from the spec edge-case table)
- iPhone reconnects while in Keyboard mode → keys are sent again once the link is secured (REQ-13 from the spec edge-case table)
- iPhone connected but keyboard reports not enabled yet → treated as not ready: status shown, keys discarded (REQ-13 from the spec edge-case table)
- Sending a report fails → `W` line without the key, key dropped, typing continues (REQ-5, REQ-16 from the spec edge-case table)
- Log level set to Debug → still no typed characters in Keyboard mode (REQ-16 from the spec edge-case table)
- Fn+vowel or Ctrl+e held → one accented letter per press, no repeat (REQ-18, REQ-19 from the spec edge-case table)
- Fn+vowel pressed while another key is held → the accented letter is typed; the held key is released on the iPhone and counts again only once pressed anew (REQ-18 from the spec edge-case table)
- Fn+Ctrl+e → nothing sent (REQ-9, REQ-19 from the spec edge-case table)
- Accent combination while the iPhone is not ready → discarded like any key (REQ-13, REQ-18, REQ-19 from the spec edge-case table)
- Ctrl+e as the iOS "end of line" shortcut → no longer available from the Cardputer (REQ-19 from the spec edge-case table)

## Notes from implementation

- **REQ-5 / REQ-11 — divergence**: after testing on the iPhone the user chose keycap legends over Italian-layout positions: the iPhone stays on "Automatic - Italian", which behaves as U.S., and accented letters come from Fn+vowel (grave) and Ctrl+e (é). SPEC raised to 1.1 with REQ-18 and REQ-19; the user confirmed è, é and the arrows on the iPhone.
- **REQ-18 — divergence**: an accent typed while another key is held releases that key on the iPhone, and it counts again only once pressed anew; without this the held key would repeat after the accent sequence. The edge-case row in SPEC.md was updated.
- **REQ-10 — Known limitation**: holding a letter opens the iOS accent popup instead of repeating it. The user accepted this, since the popup is handy for picking special characters.
- **REQ-9 / REQ-19 — divergence**: Fn+Ctrl+e sends only the Ctrl modifier press and release, with no character, which matches "nothing typed".
