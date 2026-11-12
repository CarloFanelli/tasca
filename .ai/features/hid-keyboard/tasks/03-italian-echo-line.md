# Task 03: italian-echo-line

## Objective

In Keyboard mode the screen shows the line being typed as it appears on the iPhone, accented letters included.

## Context

**Satisfies**: REQ-11, REQ-12, REQ-17

The echo line lets the user check what they typed without looking at the phone, accented letters from the Fn and Ctrl combinations included. CLAUDE.md gets the iPhone setting the keycaps rely on.

## Depends on

- Task 01

## Expected Behavior

- In Keyboard mode the screen shows the characters typed since the last Enter: keycap legends, Shift characters and the accented letters.
- Backspace removes the last echoed character; Enter starts an empty line; leaving Keyboard mode clears the line.
- Accented letters (à è é ì ò ù, uppercase too) are drawn correctly.
- CLAUDE.md states the iPhone setting: hardware keyboard layout "Automatic" (or U.S.).

## Acceptance Criteria

- [x] Typing "perché però così" in Notes shows the same text on the Cardputer screen (REQ-11)
- [x] Shift characters and uppercase accented letters on the echo match the iPhone (REQ-11)
- [x] Backspace and Enter update the echo as on the iPhone (REQ-12)
- [x] CLAUDE.md documents the iPhone hardware keyboard layout (REQ-17)
- [x] Host tests cover the layout table, Backspace, Enter, ignored keys and the dead-key placeholder (REQ-11, REQ-12)

## Edge Cases

- Modifier alone pressed and released → no echo (REQ-6 from the spec edge-case table)
- Arrow, Esc, Tab, or a key with Ctrl/Command → sent; echo line unchanged (REQ-7, REQ-11 from the spec edge-case table)
- Opt combination typed by hand (e.g. Opt+e, a dead key) → sent; echo shows a placeholder character (REQ-11 from the spec edge-case table)
- Echo line longer than the screen width → screen shows the end of the line (REQ-11 from the spec edge-case table)
- Backspace on an empty echo line → sent to the iPhone; echo stays empty (REQ-12 from the spec edge-case table)
- No key pressed for 10 s → echo line cleared (REQ-12 from the spec edge-case table)
- Display failed to initialize → echo not shown; keys still sent (REQ-11 from the spec edge-case table)
- N/A — concurrent/permission: one user, one bonded iPhone (REQ-17 from the spec edge-case table)

## Notes from implementation

- **REQ-11 — divergence**: the echo is rebuilt from the keyboard reports sent, the way a U.S. layout types them. An Option dead key typed by hand (Opt+` or Opt+e, then a vowel) therefore shows the accented letter, as the iPhone does. The placeholder `?` appears only for other Option characters and for a dead key followed by a non-vowel.
- **REQ-12 — divergence**: during the soak test the user found that the echo kept the old text after switching from Notes to another app, because iOS tells the keyboard nothing about focus. Decided with the user: the echo line is cleared after 10 s without a key. SPEC raised to 1.2 with a new edge-case row.
