---
feature: find-iphone
version: 1.0
status: approved
completed: false
base: e1ed3af4f71db5e2a6b4de1087309026491c9cf6
---

# Find iPhone

## Overview

Two ways not to lose the iPhone, switched by one Settings row, **Find iPhone**, on by default. The **Out-of-range alert**: when the link to the bonded iPhone is lost because the signal is gone, the Cardputer beeps and the Connection screen says "iPhone out of range", repeating until a key, the iPhone's return or 2 minutes. The **Find iPhone screen**, opened from the Menu while the iPhone is connected, guides the user to it by signal strength: a bar, one word (Far, Near, Very close) and a beep that gets faster closer to it. With Find iPhone on, both sound even when sound is off. The iPhone itself never rings.

## Project Context

- The Cardputer holds one encrypted BLE link to one bonded iPhone; the Connection screen covers every screen while it is down.
- The link reaches about 10 m; its signal strength varies with bodies, walls and pockets, so it gives "warmer or colder", not distance.
- The Menu (Fn+G0) lists Music, WhatsApp and Settings; items open in Reading mode.
- Settings rows are changed with `,` and `/` and kept across restarts; the sound row silences every Beep today.
- The main loop is the only code that draws and plays sounds; redraw only on state change.
- On-screen text stays in English.

## Functional Requirements

### Find iPhone setting

- REQ-1: The Settings screen has a "Find iPhone" row, On or Off, On by default and kept across restarts.
- REQ-2: With Find iPhone on, the Out-of-range alert and the Find iPhone screen's Beeps play even when sound is off.

### Out-of-range alert

- REQ-3: With Find iPhone on, when the link to the bonded iPhone is lost by signal, the Cardputer plays a triple Beep within 2 s.
- REQ-4: The triple Beep repeats every 5 s until a key or G0 is pressed, the iPhone reconnects, or 2 minutes pass.
- REQ-5: While an Out-of-range alert is on, the Connection screen line reads "iPhone out of range" instead of "Connecting…".
- REQ-6: With Find iPhone off, or after a disconnection closed by the iPhone, a pairing reset or a restart, no alert starts.

### Find iPhone screen

- REQ-7: The Menu lists "Find iPhone" after Music; Enter opens the Find iPhone screen in Reading mode.
- REQ-8: Opening the Find iPhone screen turns Find iPhone on, and it stays on until turned off in Settings.
- REQ-9: The Find iPhone screen shows a signal bar and one word, Far, Near or Very close, from the link signal averaged over 2 s.
- REQ-10: The bar and the word update at least twice per second.
- REQ-11: A short Beep repeats once every 2 s at Far, once per second at Near, and four times per second at Very close.
- REQ-12: Next to the iPhone the screen reads Very close; across a room, about 5 m away, it reads Far, on the user's iPhone.
- REQ-13: ` on the Find iPhone screen goes back to the Inbox.

## Data & Dependencies

### Information the feature must retain

- Find iPhone setting — On or Off; On until the user changes it; kept across restarts like the other settings.
- Out-of-range alert — starts when the link is lost by signal; ends on a key, G0, the iPhone's return or after 2 minutes; RAM only.
- Recent signal readings — the last 2 s of link signal while the Find iPhone screen is open; discarded when it closes.

### External capabilities relied on

- Signal strength of the established BLE link, read by the Cardputer — the only measure of how near the iPhone is.
- The disconnection reason reported by the BLE stack — tells a lost signal from a link the iPhone closed.

## Non-Functional Requirements

- **Performance** — The alert's first triple Beep within 2 s of the link loss being reported (REQ-3); the bar and word refresh at least every 500 ms (REQ-10).
- **Battery** — Signal readings only while the Find iPhone screen is open; no extra BLE traffic otherwise.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| Find iPhone row changed with `,` or `/` | Toggles On/Off, saved at once like the other rows | REQ-1 |
| First start after the update, no saved value | Find iPhone is On | REQ-1 |
| Sound off and Find iPhone off | Alert does not start; Find iPhone screen opened turns Find iPhone on, so it beeps | REQ-2, REQ-8 |
| Sound on and Find iPhone on | Same Beeps as with sound off: one rule | REQ-2 |
| Screen off when the link is lost | The alert turns the screen on to show "iPhone out of range" | REQ-3, REQ-5 |
| Link lost during a call | Call screen closes as today; the alert starts | REQ-3 |
| Link lost while the Find iPhone screen is open | Connection screen covers it and the alert starts; on reconnection the Find iPhone screen is back | REQ-3, REQ-9 |
| No iPhone bonded | No link to lose: no alert | REQ-3, REQ-6 |
| Key pressed to silence the alert | Only silences: no other effect, except Fn+Del held 3 s still resets pairing | REQ-4 |
| iPhone reconnects during the alert | Alert stops; the Connection screen closes as today | REQ-4, REQ-5 |
| 2 minutes pass without a key | Beeps stop; "iPhone out of range" stays until the iPhone returns | REQ-4, REQ-5 |
| Alert silenced by a key | Line keeps reading "iPhone out of range" until the iPhone returns | REQ-5 |
| iPhone turns Bluetooth off, powers off or forgets Tasca | No alert; Connection screen as today | REQ-6 |
| Pairing reset (Fn+Del) or restart | No alert | REQ-6 |
| Find iPhone turned off while an alert beeps | Alert stops at once | REQ-6 |
| Fn+G0 while the iPhone is not connected | Does nothing, as today: the Find iPhone screen opens only with the link up | REQ-7 |
| Signal reading fails or none yet | Empty bar, word "Searching…", no Beep | REQ-9, REQ-11 |
| Display timeout on the Find iPhone screen | Screen turns off as usual; the Beeps go on, and stop after 5 minutes on the screen without a key | REQ-11 |
| Message arrives on the Find iPhone screen | Beep and wake as today; screen stays on Find iPhone | REQ-11 |
| N/A — concurrent: one user, one iPhone; the main loop is the only writer of sounds and screen | — | REQ-3, REQ-11 |
| N/A — permission: the signal and the reason come from the bonded link only | — | REQ-3, REQ-9 |

## Out of Scope

- **Ringing the iPhone** — BLE gives an accessory no way to ring a phone; music or a Shortcut were offered and declined.
- **Direction to the iPhone** — one antenna's signal strength gives warmer or colder, never a direction.
- **Finding it beyond Bluetooth range** — needs Apple's Find My network, closed to accessories.
- **Distance in metres** — the signal varies too much with bodies and walls to show metres honestly.
