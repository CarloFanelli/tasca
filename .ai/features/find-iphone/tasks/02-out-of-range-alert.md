# Task 02: out-of-range-alert

## Objective

When the iPhone is lost by distance and Find iPhone is on, the Cardputer plays a triple Beep every 5 s and the Connection screen reads "iPhone out of range", until a key, G0, the iPhone's return or 2 minutes.

## Context

**Satisfies**: REQ-2, REQ-3, REQ-4, REQ-5, REQ-6

The Out-of-range alert rides on the Connection screen, which already covers every screen while the link is down. Only a link lost by signal starts it; a link the iPhone closes does not.

## Depends on

Task 01

## Expected Behavior

- A link lost by signal, with Find iPhone on, plays a triple Beep within 2 s and turns the screen on.
- The triple Beep repeats every 5 s; a key or G0 silences it, the iPhone's return or 2 minutes end it.
- The Connection screen line reads "iPhone out of range" while the alert is on, silenced or not, until the iPhone returns.
- The alert's Beeps play even when sound is off in Settings.
- Find iPhone off, a link closed by the iPhone, a pairing reset or a restart start no alert.

## Acceptance Criteria

- [ ] A link lost by signal plays a triple Beep within 2 s, with Find iPhone on (REQ-3)
- [ ] The triple Beep repeats every 5 s until a key, G0, reconnection or 2 minutes (REQ-4)
- [ ] The Connection screen reads "iPhone out of range" while the alert is on (REQ-5)
- [ ] The alert beeps with sound off in Settings (REQ-2)
- [ ] Find iPhone off, a link closed by the iPhone, a pairing reset or a restart start no alert (REQ-6)

## Edge Cases

- Screen off when the link is lost → the alert turns the screen on to show "iPhone out of range" (REQ-3, REQ-5)
- Link lost during a call → Call screen closes as today; the alert starts (REQ-3)
- Link lost while the Find iPhone screen is open → Connection screen covers it and the alert starts; on reconnection the Find iPhone screen is back (REQ-3)
- No iPhone bonded → no link to lose: no alert (REQ-3, REQ-6)
- Key pressed to silence the alert → only silences: no other effect, except Fn+Del held 3 s still resets pairing (REQ-4)
- iPhone reconnects during the alert → alert stops; the Connection screen closes as today (REQ-4, REQ-5)
- 2 minutes pass without a key → Beeps stop; "iPhone out of range" stays until the iPhone returns (REQ-4, REQ-5)
- Alert silenced by a key → line keeps reading "iPhone out of range" until the iPhone returns (REQ-5)
- iPhone turns Bluetooth off, powers off or forgets Tasca → no alert; Connection screen as today (REQ-6)
- Pairing reset (Fn+Del) or restart → no alert (REQ-6)
- Find iPhone turned off while an alert beeps → alert stops at once (REQ-6)
- Sound on and Find iPhone on → same Beeps as with sound off: one rule (REQ-2)
