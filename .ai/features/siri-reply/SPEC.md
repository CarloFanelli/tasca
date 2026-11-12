---
feature: siri-reply
version: 1.1
status: approved
completed: false
base: e7d3cfba187ed44dd7d2e7dc526c2875408fd574
---

# Siri Reply (spike)

## Overview

Finds out whether the Cardputer can reply to WhatsApp through Siri, without taking the iPhone out. Fn+Enter asks the iPhone to start Siri, from Reading mode or Keyboard mode, leaving the screen as it is. The user then speaks to Siri through the iPhone's or earphones' microphone. The spike verifies on the user's iPhone what works, unlocked and locked, and keeps only the commands that do. The README records the outcome and the iPhone settings it needs.

## Project Context

- The Cardputer is a BLE keyboard; its consumer control report already carries any 16-bit usage (Eject works), so no GATT change is needed.
- The Cardputer's chip has no Bluetooth Classic and no LE Audio: it can never be a microphone or headset for the iPhone.
- Keyboard mode sends keys only while the iPhone is ready (secured link, keyboard report enabled).
- Fn+letter types accents, Fn+Space toggles the on-screen keyboard, Opt+s opens the **Settings screen**; Fn+Enter is free.
- Siri's answers never reach the Cardputer: the outcome shows on the iPhone or is spoken in the earphones.

## Functional Requirements

### Siri key

- REQ-1: Fn+Enter in Reading mode or Keyboard mode asks the iPhone to start Siri.
- REQ-2: Fn+Enter leaves the mode and screen unchanged: Reading mode stays on its screen, Keyboard mode keeps typing.
- REQ-3: Fn+Enter types nothing on the iPhone and never opens the Chat screen.

### Verification on the iPhone

- REQ-4: On the user's iPhone, unlocked and locked, the spike records whether Fn+Enter starts Siri, and with which command when several are tried.
- REQ-5: The spike records whether a WhatsApp message dictated to Siri is sent, with and without earphones.
- REQ-6: The README states what works, the iPhone settings it needs, and that the Cardputer cannot act as microphone.
- REQ-7: If no command starts Siri, Fn+Enter is removed and the README says Siri cannot be started from the Cardputer.

## External capabilities relied on

- iOS response to a Bluetooth keyboard's assistant command — starts Siri; to be verified.
- Siri with "Allow Siri When Locked" — to make a spoken request without the iPhone in hand.

## Non-Functional Requirements

- **Performance** — Siri appears on the iPhone within 1 s of Fn+Enter.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| iPhone not ready (not connected or keyboard not enabled) | Nothing sent; `W` line; Keyboard mode shows "iPhone not ready" as today | REQ-1, REQ-2 |
| Fn+Enter held | One request only; no repeat while held | REQ-1 |
| Fn+Enter pressed again while Siri is listening | Sent again; iOS decides (usually stops Siri) | REQ-1 |
| Fn+Enter with the screen off | In Reading mode only wakes the screen, as any key; in Keyboard mode wakes it and starts Siri | REQ-1, REQ-2 |
| Fn+Enter on the Settings screen or in a Chat screen | Starts Siri; the screen stays as it is | REQ-2, REQ-3 |
| Fn+Enter while an action waits for confirmation | Prompt cancelled, as any other key; Siri started | REQ-2, REQ-3 |
| Enter without Fn | Unchanged: opens a Chat, confirms an action, or sends Enter in Keyboard mode | REQ-3 |
| Siri disabled on the iPhone, or not allowed when locked | Nothing visible on the Cardputer; the README lists the setting | REQ-4, REQ-6 |
| Siri asks for confirmation ("Send it?") | Answered by voice | REQ-5 |
| WhatsApp contact name ambiguous for Siri | Siri asks which contact; outcome recorded, not handled by the Cardputer | REQ-5 |
| No earphones and iPhone in a pocket | Siri uses the iPhone's microphone; recorded whether it understands | REQ-4, REQ-5 |
| N/A — concurrent: one user, one key at a time | — | REQ-1, REQ-4 |
| N/A — integrity: no data stored by this feature | — | REQ-5, REQ-6, REQ-7 |

## Out of Scope

- **Type to Siri** — turning it on replaces the spoken Siri on the iPhone; the user keeps the native voice Siri.
- **Cardputer as microphone or headset** — impossible: the chip has no Bluetooth Classic nor LE Audio.
- **Showing Siri's answer on the Cardputer** — iOS sends nothing back to a keyboard.
- **Incoming calls (show the caller, answer or decline)** — a separate feature after this spike.
- **Voice commands other than WhatsApp** — the spike checks the WhatsApp reply; other requests work the same way if Siri starts.
