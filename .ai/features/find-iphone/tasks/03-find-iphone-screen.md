# Task 03: find-iphone-screen

## Objective

Menu > Find iPhone opens the Find iPhone screen: a signal bar, Far, Near or Very close, and a short Beep that gets faster closer to the iPhone.

## Context

**Satisfies**: REQ-2, REQ-7, REQ-8, REQ-9, REQ-10, REQ-11, REQ-13

The link's signal strength is the only measure of distance, and it varies a lot: readings are averaged over 2 s and shown as three words. Opening the screen turns Find iPhone on. Thresholds are tuned on the device in Task 04.

## Depends on

Task 01

## Expected Behavior

- The Menu lists Music, Find iPhone, WhatsApp, Settings; Enter on Find iPhone opens the Find iPhone screen in Reading mode.
- Opening it turns Find iPhone on, saved like a change in Settings.
- The screen shows a bar and Far, Near or Very close from the signal averaged over 2 s, refreshed at least twice per second.
- A short Beep repeats once every 2 s at Far, once per second at Near, four times per second at Very close, even with sound off.
- ` goes back to the Inbox.

## Acceptance Criteria

- [ ] The Menu lists "Find iPhone" after Music and Enter opens the Find iPhone screen (REQ-7)
- [ ] Opening the screen turns Find iPhone on, still on after a restart (REQ-8)
- [ ] Bar and word follow the signal averaged over 2 s, refreshed at least every 500 ms (REQ-9, REQ-10)
- [ ] The Beep cadence is 2 s, 1 s and 250 ms for Far, Near and Very close, with sound on or off (REQ-11, REQ-2)
- [ ] ` on the Find iPhone screen goes back to the Inbox (REQ-13)

## Edge Cases

- Sound off and Find iPhone off → opening the Find iPhone screen turns Find iPhone on, so it beeps (REQ-2, REQ-8)
- Fn+G0 while the iPhone is not connected → does nothing, as today: the Find iPhone screen opens only with the link up (REQ-7)
- Signal reading fails or none yet → empty bar, word "Searching…", no Beep (REQ-9, REQ-11)
- Display timeout on the Find iPhone screen → screen turns off as usual; the Beeps go on, and stop after 5 minutes on the screen without a key (REQ-11)
- Message arrives on the Find iPhone screen → Beep and wake as today; screen stays on Find iPhone (REQ-11)
