---
feature: screen-stack
version: 1.0
status: approved
completed: true
base: 400cb1e089af2a71fe909db5e4829ced1c88588a
---

# Screen Stack

## Overview

Reorganizes the firmware's screens so that each one lives in its own source file with the same shape (draw, handle a key, update over time), and a **Screen stack** decides which screen is on top. A screen opened over another returns to it when closed. Nothing changes for the user: every key, screen and sound behaves as before. The work prepares the **Call screen** and the **Menu**, which both open over any screen and must return the user where they were.

## Project Context

- Today one file holds startup, the loop, drawing and key handling of the Inbox, Chat screen, Settings screen and Keyboard mode.
- Pure logic already lives in host-tested core code, which must stay free of hardware headers.
- G0 switches to Keyboard mode and back to the screen it was entered from (Inbox, Chat screen or Settings screen).
- The main loop is the only code that draws, and it redraws only on change.
- BLE callbacks never touch screens; they queue data for the main loop.
- Upcoming features (incoming calls, music menu) open screens over any screen.

## Functional Requirements

### No visible change

- REQ-1: Every key, G0 and key combination does what it did before, on the Inbox, Chat screen, Settings screen and in Keyboard mode.
- REQ-2: Every screen looks as before: same content, layout and colors, status bar included.
- REQ-3: Beeps, Banner, display timeout, wake on new message and Siri behave as before on every screen.

### Structure

- REQ-4: The Inbox, Chat screen, Settings screen and Keyboard mode each live in their own source file, with draw, key and update entry points.
- REQ-5: A screen opened over another (Settings screen, Keyboard mode) returns to that screen, unchanged, when closed.
- REQ-6: Only the screen on top of the Screen stack receives keys and draws its area.
- REQ-7: The Screen stack rules (open over, close back, top receives keys) are covered by host tests.
- REQ-8: The main firmware file keeps only startup, the loop and dispatch, in at most 300 lines.

### Checks

- REQ-9: All existing host tests pass unchanged, the firmware builds, and flash use grows by at most 4 KB.
- REQ-10: A regression checklist covering REQ-1 to REQ-3 is run on the Cardputer and its outcome recorded.

## Non-Functional Requirements

- **Performance** — No screen redraws on a loop iteration where nothing changed; key-to-screen latency unchanged (one loop, about 10 ms).
- **Security** — No typed text reaches the serial log in Keyboard mode, as today.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| Key or G0 with the screen off | Only turns the screen on in Reading mode; in Keyboard mode the key also reaches the iPhone, as today | REQ-1, REQ-3 |
| G0 on the Settings screen | Keyboard mode opens over it; G0 again returns to the Settings screen | REQ-1, REQ-5 |
| G0 on a Chat screen | Keyboard mode shows that Chat's last Message as context, as today; G0 returns to the Chat screen | REQ-1, REQ-5 |
| Settings action waiting for confirmation when another screen opens over it | Prompt cancelled, as any other key does today | REQ-1, REQ-5 |
| Messages arrive while the Settings screen or Keyboard mode covers a Chat screen | Inbox updated underneath; that Chat's Messages marked read only when it is back on top, as today | REQ-3, REQ-5 |
| Every Message of the covered Chat removed meanwhile | Closing the screen above shows the Inbox, as today | REQ-5 |
| Live Message in Keyboard mode | Banner and beep as today; in Reading mode the screens show it | REQ-3 |
| Fn+Del held 3 s | Pairing reset from Reading mode screens only, as today | REQ-1 |
| Fn+Enter (Siri) and ` right after it | Same on every screen; ` closes Siri instead of going back | REQ-1, REQ-3 |
| Closing the only screen left (the Inbox) | Not possible: the Inbox is always at the bottom | REQ-5, REQ-6 |
| Same screen opened twice over itself | Not possible: opening a screen already on top does nothing | REQ-5, REQ-7 |
| Status bar changes (link, battery, unread) under any screen | Redrawn as today, whatever screen is on top | REQ-2, REQ-6 |
| Flash budget exceeded | The build fails the 4 KB check: report it before merging | REQ-9 |
| N/A — concurrent: the main loop alone opens, closes and draws screens | — | REQ-5, REQ-6 |
| N/A — permission: no new action reaches the iPhone | — | REQ-1, REQ-4 |
| N/A — network: BLE handling and its queues are untouched | — | REQ-3, REQ-4 |
| N/A — integrity: no stored data changes shape (settings and Messages unchanged) | — | REQ-8, REQ-9 |

## Out of Scope

- **Call screen and Menu** — they are the next features, built on this structure.
- **Any visible change or new setting** — the reorganization must be invisible to compare behavior before and after.
- **BLE, ANCS and HID code** — already in separate modules; untouched.
- **A generic UI library (widgets, layouts)** — not needed for four screens; drawing code moves as it is.
