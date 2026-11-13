# Task 01: call-screen

## Objective
An incoming call opens the **Call screen** over any screen, in either mode, with the caller's name; it rings while open and closes back where the user was when the call stops ringing.

## Context
**Satisfies**: REQ-1, REQ-2, REQ-5, REQ-6, REQ-7, REQ-8
The Call screen and its lifecycle come first: they work with the iPhone's own answer and decline, so the user can already see who calls. Task 02 adds the keys that act on the call.

## Depends on
None

## Expected Behavior
- An incoming-call notification opens the Call screen within 1 s: "Incoming call" and the caller's name, over the Inbox, a Chat screen, the Settings screen or Keyboard mode.
- While the Call screen is open no key reaches the iPhone and G0 does nothing; keys do nothing else yet.
- The screen turns on, whatever the wake-on-message setting, and stays on while the Call screen is open.
- With sound on, a double beep repeats every 2 s while the Call screen is open; with sound off, silence.
- When the call is answered, declined or stops ringing (on the iPhone or the earphones), the Call screen closes and the previous screen and mode come back.
- Calls never become Messages; missed-call notifications are skipped. The caller's name never reaches the serial log.

## Acceptance Criteria
- [ ] An incoming call shows the Call screen with the caller's name, from Reading mode and from Keyboard mode (REQ-1)
- [ ] While the Call screen is open, no HID report reaches the iPhone and G0 changes nothing (REQ-2)
- [ ] The removed event of the call closes the Call screen and restores the previous screen and mode (REQ-5)
- [ ] The screen turns on with wake on message off, and does not turn off while the Call screen is open (REQ-6)
- [ ] With sound on a double beep every 2 s while open; none with sound off (REQ-7)
- [ ] No Message and no Inbox entry is created by an incoming or missed call (REQ-8)

## Edge Cases
- Caller has no name (unknown number) → the number is shown, as the notification title (REQ-1)
- Empty title → "Unknown caller" (REQ-1)
- Caller's name never received (attribute request refused or timed out) → "Unknown caller", keys still act (REQ-1 · from code)
- Caller name longer than the screen → cut with "…" on one line (REQ-1)
- Call arrives with the screen off → screen turns on with the Call screen; no key is needed first (REQ-1, REQ-6)
- Call arrives on the Settings screen with an action waiting for confirmation → prompt cancelled; after the call, Settings is shown without the prompt (REQ-1, REQ-5)
- Call arrives while Siri is held (Fn+Enter down) → Siri released first, then the Call screen opens (REQ-1, REQ-2)
- Second call while one rings (call waiting) → the Call screen shows the newest call (REQ-1)
- Call already ringing when the Cardputer reconnects (import) → shown like a live call (REQ-1)
- Other keys, Fn+Enter, Fn+Del held → ignored while the Call screen is open (REQ-2)
- Call answered or declined on the iPhone or earphones → removed event closes the Call screen (REQ-5)
- Link lost while ringing → Call screen closes; the status bar shows the link state as today (REQ-5)
- Previous screen was a Chat whose Messages were removed meanwhile → back to the Inbox, as when a Chat empties today (REQ-5)
- Message arrives while the Call screen is open → stored as usual; no Message beep and no Banner during the call (REQ-7, REQ-8)
- Sound turned off on the iPhone (silent switch) → not known to the Cardputer: only the Sound setting decides (REQ-7)
- Missed-call notification after an unanswered call → skipped and counted like other categories (REQ-8)
- N/A — concurrent: one user, one Call screen; the main loop is the only writer (REQ-2, REQ-5)
- N/A — integrity: nothing is stored by this feature (REQ-8)
