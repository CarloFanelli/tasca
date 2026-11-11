---
feature: firmware-skeleton
version: 1.0
status: approved
completed: false
---

# Firmware Skeleton (M0)

## Overview

Sets up the Tasca firmware project for the M5Stack Cardputer v1.1: a reproducible
PlatformIO build, flashing over USB, a "Hello" screen, raw keyboard and G0 button
events on a tagged serial log, a boot report that settles whether the board has
PSRAM, and a host-side test environment that runs without hardware. It is the
foundation every later milestone (ANCS in M1, HID in M2) builds on, and proves
the toolchain, display, keyboard and test loop work before any Bluetooth code.

## Project Context

- Hardware target: Cardputer v1.1 (ESP32-S3, 240x135 display, 56-key US keyboard, G0 button).
- Build system is PlatformIO with the Arduino framework, board and flags per M5Stack docs (Tasca spec §5.1).
- Firmware and host tests are written in C++ (Arduino framework); no other language in the repo except tooling config.
- Hardware-free logic must compile and run on the Mac with no Arduino or Cardputer library dependency.
- The serial log format defined here is reused by every later milestone (Tasca spec §5.4).
- The repo is an empty git repository; no remote, no commits yet.
- Development is driven by Claude Code, which must be able to build, test and read serial output from the CLI.

## Functional Requirements

### Toolchain & repository

- REQ-1: PlatformIO CLI is installed on the development Mac and `pio --version` succeeds.
- REQ-2: The firmware environment builds with zero errors from a clean checkout using a single documented command.
- REQ-3: A single documented command flashes the firmware to a USB-connected Cardputer, which then runs it without manual steps.
- REQ-4: CLAUDE.md lists the build, flash, test and serial-monitor commands, the serial port, concurrency rules (Tasca spec §5.3) and the ANCS spec link.
- REQ-5: Build output and local tool caches are excluded from git.

### Display

- REQ-6: Within 3 s of power-on the display shows "Hello" and the firmware version.
- REQ-7: The display shows the last key pressed with its held modifiers, updated within 100 ms of the press.

### Serial log

- REQ-8: Every serial line has the form `[L][TAG] message`, where L is D, I, W or E and TAG names the module.
- REQ-9: Lines below the build-time minimum level are not printed; the default minimum is Debug.
- REQ-10: At boot one report is logged: firmware version, PSRAM present and size, free heap, flash size.
- REQ-11: Each key press logs one `down` line and each release one `up` line, naming the physical key and held modifiers.
- REQ-12: Modifier keys (Shift, Ctrl, Opt, Alt, Fn) pressed alone also log `down` and `up` lines.
- REQ-13: Each G0 button press and release logs exactly one line tagged `BTN`.

### Host tests

- REQ-14: `pio test -e native` runs on the Mac with no Cardputer connected and all tests pass.
- REQ-15: Host tests cover log-line formatting for all four levels and key-to-text conversion for plain keys, each modifier and combinations.

## Data & Dependencies

### External capabilities relied on

- Cardputer board support (display, keyboard matrix, G0 button) — official M5Stack drivers.
- Host unit-test runner — runs the hardware-free logic on the Mac.

## Non-Functional Requirements

- **Validation** — Platform and every third-party library are pinned to exact versions; two clean builds of the same commit use identical dependency versions.
- **Validation** — Code covered by host tests compiles in the native environment without any Arduino, ESP-IDF or M5Stack header.
- **Performance** — Key events reach serial within 50 ms of the physical press; a held key produces one `down` line, no auto-repeat.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| First build with empty dependency cache | Pinned dependencies are downloaded and the build succeeds | REQ-2 |
| No internet on first build | Build fails with PlatformIO's download error; CLAUDE.md notes the first build needs network | REQ-2 |
| No Cardputer connected at flash time | Flash command fails with a "no serial port" error, nothing else changes | REQ-3 |
| Board doesn't enumerate on USB (e.g. crashing firmware) | CLAUDE.md documents download mode: hold G0 while plugging USB, then flash | REQ-3 |
| Serial port name differs from the documented one | CLAUDE.md shows how to list ports and override the port in the command | REQ-4 |
| N/A — concurrent/permission/network: CLAUDE.md is static documentation | — | REQ-4 |
| Build creates new cache directories | None of them show up in `git status` | REQ-5 |
| N/A — concurrent/permission/network: ignore rules are static | — | REQ-5 |
| Display fails to initialize | Firmware keeps running, logs an `E` line tagged `SYS`; serial key logging still works | REQ-6 |
| Long version string | "Hello" and version fit 240x135 without clipping | REQ-6 |
| Key with no printable glyph (Enter, Del, Tab, Fn alone) | Shown by name, e.g. "Enter", "Del" | REQ-7 |
| Two keys pressed in quick succession | Display shows the most recent press | REQ-7 |
| No key pressed yet since boot | Key area shows a placeholder, e.g. "-" | REQ-7 |
| Message longer than 256 characters | Line truncated at 256 characters and ends with `...` | REQ-8 |
| Empty message | Line still printed with level and tag | REQ-8 |
| Tag longer than 6 characters | Tag truncated to 6 characters | REQ-8 |
| Minimum level set to Error at build time | Only `E` lines printed, including no boot report if it is logged at Info | REQ-9 |
| Board has no PSRAM | Boot report shows `psram=none`, boot continues normally | REQ-10 |
| PSRAM detected but fails to initialize | Boot report shows `psram=error`, boot continues | REQ-10 |
| Two non-modifier keys held together | Each gets its own `down` and `up` line, in physical order | REQ-11 |
| Key held for several seconds | One `down` line on press, one `up` on release, nothing in between | REQ-11 |
| Modifier released while another key is still held | `up` line for the modifier; later lines show updated modifiers | REQ-12 |
| G0 contact bounce | One physical press yields exactly one press and one release line | REQ-13 |
| G0 held during power-on | Board enters download mode, firmware doesn't run; documented in CLAUDE.md | REQ-13 |
| Test code accidentally includes a hardware header | Native build fails, `pio test -e native` reports failure | REQ-14 |
| Combination with multiple modifiers (e.g. Ctrl+Shift+a) | Converted text lists all modifiers in a fixed order | REQ-15 |

## Out of Scope

- **Bluetooth (advertising, bonding, ANCS)** — milestone M1, needs this skeleton first.
- **HID keyboard toward the iPhone** — milestone M2.
- **Fn translation into arrows/Esc and US keycode mapping** — belongs to the keymap in M2/M3; M0 logs only raw keys.
- **UI screens, status bar, battery reading** — milestone M4.
- **Settings, NVS, display timeout, sound** — milestone M5.
- **microSD logging** — milestone M6, optional.
- **First commit and remote setup** — repository init only for now, by user decision.
