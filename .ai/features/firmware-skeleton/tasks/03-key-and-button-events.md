# Task 03: key-and-button-events

## Objective

Every key press and release, modifiers included, and every G0 press and release appears on serial, and the display shows the last key pressed.

## Context

**Satisfies**: REQ-7, REQ-11, REQ-12, REQ-13, REQ-15

M2 sends exactly these press and release events to the iPhone, and M4 uses G0 to switch modes: this task proves both are readable on the hardware. Key-to-text conversion is pure logic and joins the host tests set up in Task 02.

## Depends on

- Task 02

## Expected Behavior

- Each key press logs one `down` line and each release one `up` line, tagged `KEY`, naming the physical key and held modifiers.
- Shift, Ctrl, Opt, Alt and Fn pressed alone also log `down` and `up` lines.
- A held key logs one `down` line and nothing until release: no auto-repeat.
- Each G0 press and release logs exactly one line tagged `BTN`.
- Under "Hello" the display shows the last key pressed with its modifiers, updated within 100 ms of the press.
- Key events reach serial within 50 ms of the physical press.
- Host tests cover key-to-text conversion for plain keys, each modifier and combinations.

## Acceptance Criteria

- [ ] Pressing and releasing a key prints one `down` and one `up` line naming key and modifiers (REQ-11)
- [ ] Each modifier pressed alone prints its own `down` and `up` lines (REQ-12)
- [ ] One G0 press prints exactly one press and one release line tagged `BTN` (REQ-13)
- [ ] The display shows the last key with modifiers within 100 ms of the press (REQ-7)
- [ ] Host tests for key-to-text cover plain keys, each modifier and multi-modifier combinations, and pass (REQ-15)

## Edge Cases

- Key with no printable glyph (Enter, Del, Tab, Fn alone) → shown by name, e.g. "Enter", "Del" (REQ-7 from the spec edge-case table)
- Two keys pressed in quick succession → display shows the most recent press (REQ-7 from the spec edge-case table)
- No key pressed yet since boot → key area shows a placeholder, e.g. "-" (REQ-7 from the spec edge-case table)
- Two non-modifier keys held together → each gets its own `down` and `up` line, in physical order (REQ-11 from the spec edge-case table)
- Key held for several seconds → one `down` line on press, one `up` on release, nothing in between (REQ-11 from the spec edge-case table)
- Modifier released while another key is still held → `up` line for the modifier; later lines show updated modifiers (REQ-12 from the spec edge-case table)
- G0 contact bounce → one physical press yields exactly one press and one release line (REQ-13 from the spec edge-case table)
- G0 held during power-on → board enters download mode, firmware doesn't run; documented in CLAUDE.md (REQ-13 from the spec edge-case table)
- Combination with multiple modifiers (e.g. Ctrl+Shift+a) → converted text lists all modifiers in a fixed order (REQ-15 from the spec edge-case table)
