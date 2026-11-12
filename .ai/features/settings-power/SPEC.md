---
feature: settings-power
version: 1.1
status: approved
completed: false
base: 891ab744da34a768179d6a417aeaf23186330185
---

# Settings and Power (M5)

## Overview

Adds a **Settings** screen and makes the Cardputer last on battery. From Reading mode, Opt+s opens Settings:
- brightness, sound, display timeout and wake on new Message, all kept across restarts;
- two actions: reset pairing and clear Messages.

A Message arriving live beeps, if sound is on, and wakes the screen, if that setting is on. The screen turns off after the display timeout. While it is off, the Bluetooth link uses slower, power-saving timings. Attributes are requested only for the notification category WhatsApp uses, so reconnection is faster and lighter. A README explains how to set up the iPhone.

## Project Context

- Builds on reading-screens: Inbox, Chat screen, status bar, Banner, Messages mirrored from the iPhone.
- Keyboard mode sends every key to the iPhone; Settings are reachable only from Reading mode.
- Fn+Del (3 s) pairing reset exists and stays; the Settings action does the same thing.
- Settings and bonding keys persist in the ESP32's non-volatile storage; Messages stay in RAM only.
- iOS accepts only certain connection timings from accessories; slower timings must stay within them.
- Notification categories come with every notification event; WhatsApp's category must be observed on hardware.

## Functional Requirements

### Settings screen

- REQ-1: Opt+s in Reading mode opens Settings from the Inbox or a Chat screen; `` ` `` returns to that screen.
- REQ-2: `;` and `.` select a setting; `,` and `/` (Fn optional) step its value back and forward, applied at once.
- REQ-3: Settings offer brightness (20–100 % in steps of 20), sound (on/off), display timeout and wake on new Message (on/off).
- REQ-4: Display timeout offers 15 s, 30 s, 1 min, 2 min, 5 min and Never; default 30 s.
- REQ-5: Every setting keeps its value across restarts; defaults are brightness 60 %, sound on, wake on new Message on.

### Actions

- REQ-6: "Reset pairing" and "Clear Messages" run only after Enter, then Enter again on the "Press Enter to confirm" prompt.
- REQ-7: Reset pairing erases the bond and restarts, as Fn+Del does; Clear Messages empties the Messages, keeping the pairing.

### Beep and screen

- REQ-8: A Message arriving live beeps once briefly when sound is on; notifications resent at reconnection never beep.
- REQ-9: The screen turns off after the display timeout without a key, G0 press or live Message; Bluetooth stays connected.
- REQ-10: A key or G0 turns the screen back on; in Reading mode that key does nothing else, in Keyboard mode it reaches the iPhone.
- REQ-11: A Message arriving live turns the screen on when wake on new Message is on.

### Power

- REQ-12: With the screen off the link requests slower timings accepted by iOS; with the screen on it requests the usual ones.
- REQ-13: Attributes are requested only for notifications in WhatsApp's category; others are counted and skipped.
- REQ-14: The README states the iPhone setup steps and that Messages on the Cardputer are unprotected.

## Data & Dependencies

### Information the feature must retain

- Settings — brightness, sound, display timeout, wake on new Message. Created with defaults on first start; changed from the Settings screen; kept across restarts and pairing resets.

### External capabilities relied on

- Apple Notification Center Service — the category of each notification, used to skip other apps.
- iOS connection timing rules for accessories — bound the slower timings.

## Non-Functional Requirements

- **Performance** — A key on the off screen turns it on within 200 ms.
- **Performance** — Typing in Keyboard mode with the screen off loses no key and lags under 300 ms.
- **Performance** — Reconnection import finishes in under 5 s with ~120 other-app notifications present.
- **Security** — Messages stay readable by anyone holding the Cardputer: the README says so (no screen lock in v1).

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| Opt+s in Keyboard mode | Sent to the iPhone like any key; Settings not opened | REQ-1 |
| Opt+s while Settings is open | Ignored | REQ-1 |
| `;`/`.` past the first or last setting | Selection stays; no wrap-around | REQ-2 |
| `,`/`/` past the first or last value | Value stays; no wrap-around | REQ-2 |
| `,`/`/` on an action | Ignored | REQ-2, REQ-6 |
| Brightness changed | Screen brightness changes at once | REQ-3 |
| Stored settings missing or unreadable (first start, corrupted) | Defaults used; `W` line when unreadable | REQ-5 |
| Saving a setting fails | Value applies until restart; `W` line | REQ-5 |
| Reset pairing with no bond | Restart anyway; nothing to erase | REQ-7 |
| Confirmation prompt then another key, or 3 s without Enter | Action cancelled | REQ-6 |
| Live Message during the confirmation prompt | Prompt stays; Message handled as usual | REQ-6, REQ-8 |
| Clear Messages with a Chat screen in the history | Back to the empty Inbox on leaving Settings | REQ-7 |
| Cleared Messages still open on the iPhone | Return at the next reconnection | REQ-7 |
| Several Messages arrive within a second | One beep each, never overlapping | REQ-8 |
| Live Message while the screen is off and wake is off | Beep if sound on; screen stays off; unread counts update | REQ-8, REQ-11 |
| Updated Message (edited on the iPhone) | No beep, no wake | REQ-8, REQ-11 |
| Display timeout Never | Screen stays on; timings stay usual | REQ-4, REQ-9 |
| Screen turns off in Keyboard mode | Keys still go to the iPhone; echo continues | REQ-9, REQ-10 |
| Fn+Del held with the screen off | Pairing reset still works after 3 s | REQ-10 |
| G0 with the screen off | Screen on; mode switches only on the next press | REQ-10 |
| iPhone refuses the slower timings | Link keeps the current ones; `W` line; no retry until the next screen change | REQ-12 |
| Screen on while slow timings pending | Usual timings requested at once | REQ-12 |
| Notification event without a category WhatsApp uses | Counted; no attribute request | REQ-13 |
| WhatsApp notification in an unexpected category | Not stored — check the category on hardware before enabling the filter | REQ-13 |
| Removed event for a skipped notification | Ignored | REQ-13 |
| README read before pairing | Setup steps in the order they are done: pair, share notifications, keyboard layout | REQ-14 |
| Display failed to initialize | Settings still applied from storage; beep and timings work | REQ-3, REQ-9 |
| N/A — concurrent: one user, one main loop | — | REQ-2, REQ-6 |
| N/A — permission: no access control on the device (stated in README) | — | REQ-1, REQ-14 |

## Out of Scope

- **Measured standby autonomy in the README** — postponed by the user; tracked as backlog B-6.
- **Log on microSD and reload at start** — milestone M6.
- **Screen lock or PIN** — not in v1; the README states the device is unprotected.
- **Volume or beep tone settings** — one short beep is enough; on/off covers the need.
- **Deep sleep with the link off** — the Cardputer must stay connected to receive Messages.
- **Siri reply spike** — planned separately after M5 in the project spec.
