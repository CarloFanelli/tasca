---
feature: incoming-calls
version: 1.0
status: approved
completed: false
base: feeb19bd448105ee62bdc7e4d703452801867f93
---

# Incoming Calls

## Overview

Shows an incoming call on the Cardputer and lets the user answer or decline it with a key, without taking the iPhone out. The iPhone's incoming-call notification opens the **Call screen** over any screen, in Reading mode and Keyboard mode, with the caller's name. Enter answers, `` ` `` declines, and while the call rings the Cardputer beeps when sound is on. The Call screen closes when the call is answered, declined or stops ringing, and the user is back where they were. Audio stays on the iPhone or the earphones. Calls are never stored: missed calls stay on the iPhone.

## Project Context

- Today every notification outside the Social category is counted and skipped; incoming calls arrive in the IncomingCall category.
- ANCS lets the Cardputer perform the positive (answer) or negative (decline) action of a notification; nothing is sent about a call once answered.
- GATT writes (the ANCS Control Point included) are issued only from the main loop.
- Sound and the screen sleep come from the **Settings screen**: sound on/off, display timeout, wake on new message.
- `` ` `` already means "back" in Reading mode; Enter opens or confirms.
- The Cardputer chip has no Bluetooth Classic: it never carries call audio.

## Functional Requirements

### Call screen

- REQ-1: An incoming-call notification opens the Call screen within 1 s, over any screen and in either mode, showing "Incoming call" and the caller's name.
- REQ-2: While the Call screen is open, no key reaches the iPhone and only Enter and `` ` `` act; G0 does nothing.
- REQ-3: Enter on the Call screen answers the call on the iPhone.
- REQ-4: `` ` `` on the Call screen declines the call on the iPhone.
- REQ-5: The Call screen closes when the call is answered, declined or stops ringing, returning to the previous screen and mode.

### Alerting

- REQ-6: An incoming call always turns the screen on, whatever the wake-on-message setting, and the screen stays on while the Call screen is open.
- REQ-7: With sound on, a double beep repeats every 2 s while the Call screen is open; with sound off there is no beep.

### Storage and verification

- REQ-8: Calls never become Messages or appear in the Inbox; missed-call notifications are skipped.
- REQ-9: On the user's iPhone, answer and decline are verified for phone, FaceTime and WhatsApp calls; the README records what works.

## External capabilities relied on

- ANCS IncomingCall category with positive and negative actions (iOS 8+) — answer and decline; whether FaceTime and WhatsApp calls use it is to be verified (REQ-9).

## Non-Functional Requirements

- **Performance** — The Call screen opens within 1 s of the notification; answer or decline reaches the iPhone within 1 s of the key.
- **Privacy** — The caller's name is never written to the serial log; only the call event and the action taken.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| Caller has no name (unknown number) | The number is shown, as the iPhone's notification title | REQ-1 |
| Empty title | "Unknown caller" | REQ-1 |
| Caller name longer than the screen | Cut with "…" on one line | REQ-1 |
| Call arrives with the screen off | Screen turns on with the Call screen; no key is needed first | REQ-1, REQ-6 |
| Call arrives on the Settings screen with an action waiting for confirmation | Prompt cancelled; after the call, Settings is shown without the prompt | REQ-1, REQ-5 |
| Call arrives while Siri is held (Fn+Enter down) | Siri released first, then the Call screen opens | REQ-1, REQ-2 |
| Second call while one rings (call waiting) | The Call screen shows the newest call; Enter and `` ` `` act on it | REQ-1, REQ-3, REQ-4 |
| Call already ringing when the Cardputer reconnects (import) | Shown like a live call | REQ-1 |
| Other keys, Fn+Enter, Fn+Del held | Ignored while the Call screen is open | REQ-2 |
| Enter or `` ` `` held or repeated | One action per press; nothing more while waiting for the call to close | REQ-3, REQ-4 |
| Notification without a positive action | Enter does nothing; hint shows only "` decline" | REQ-3 |
| Notification without a negative action | `` ` `` does nothing; hint shows only "Enter answer" | REQ-4 |
| The iPhone refuses the action (error from Control Point) | `W` line; Call screen stays with "Failed — use the iPhone" until the call closes | REQ-3, REQ-4 |
| Call answered or declined on the iPhone or earphones | Removed event closes the Call screen | REQ-5 |
| Link lost while ringing | Call screen closes; the status bar shows the link state as today | REQ-5 |
| Previous screen was a Chat whose Messages were removed meanwhile | Back to the Inbox, as when a Chat empties today | REQ-5 |
| Message arrives while the Call screen is open | Stored as usual; no Message beep and no Banner during the call | REQ-7, REQ-8 |
| Sound turned off on the iPhone (silent switch) | Not known to the Cardputer: only the Sound setting decides | REQ-7 |
| Missed-call notification after an unanswered call | Skipped and counted like other categories | REQ-8 |
| FaceTime or WhatsApp calls not in the IncomingCall category | Not shown; the README says which calls work | REQ-9 |
| N/A — concurrent: one user, one call screen; the main loop is the only writer | — | REQ-2, REQ-5 |
| N/A — permission: every action needs the bonded iPhone, as for every ANCS write | — | REQ-3, REQ-4 |
| N/A — integrity: nothing is stored by this feature | — | REQ-8 |

## Out of Scope

- **Hanging up or showing an ongoing call** — ANCS sends nothing about a call after it is answered; whether another route exists is a backlog item.
- **Missed calls in the Inbox** — kept small; missed calls stay on the iPhone (backlog item).
- **Call audio on the Cardputer** — no Bluetooth Classic on the chip.
- **Ringing patterns or volume settings** — one fixed double beep, governed by the existing Sound setting.
