# Task 03: slim-main-checks

## Objective
The main firmware file keeps only startup, the loop and dispatch, and the reorganization is checked on the Cardputer against a regression checklist.

## Context
**Satisfies**: REQ-2, REQ-3, REQ-8, REQ-9, REQ-10
Last, once every screen has moved: what remains in the main file is moved out, then the whole behavior is compared with before on the device.

## Depends on
02-keyboard-mode-screen

## Expected Behavior
- The main firmware file holds startup, the loop and the dispatch to the Screen stack, in at most 300 lines.
- Host tests pass unchanged; the firmware builds; flash use grows by at most 4 KB over the build before this feature.
- A regression checklist (every key per screen, G0, Siri, Banner, beeps, display timeout, wake on message, status bar) is run on the Cardputer and its outcome recorded.

## Acceptance Criteria
- [ ] The main firmware file is at most 300 lines (REQ-8)
- [ ] All existing host tests pass unchanged and the firmware builds (REQ-9)
- [ ] Flash use is at most 4 KB above 2,013,903 bytes, the build before this feature (REQ-9)
- [ ] The regression checklist is run on the Cardputer, each item recorded pass or fail (REQ-10, REQ-2, REQ-3)

## Edge Cases
- Flash budget exceeded → the 4 KB check fails: reported before merging (REQ-9)
- N/A — integrity: no stored data changes shape (settings and Messages unchanged) (REQ-8, REQ-9)

## Notes from implementation

- **Known limitation**: the regression checklist needs the user on the Cardputer; the outcome of each item is recorded below once run. Sizes measured by /dev: main firmware file 187 lines; flash 2,016,519 bytes, 2,616 above the 2,013,903 baseline; 203 host tests pass. /dev, 2026-11-13.
- **Regression checklist** (each: pass / fail):
  1. Inbox: `;` `.` move the selection; Enter opens the Chat screen.
  2. Chat screen: `;` `.` scroll; ` back to the Inbox, same selection.
  3. Opt+s from the Inbox and from a Chat screen opens Settings; ` returns to that screen.
  4. Settings: `,` `/` change values, kept after restart; "Clear messages" needs a second Enter; prompt expires after 3 s.
  5. G0 from the Inbox, a Chat screen and Settings opens Keyboard mode; G0 returns to the same screen.
  6. Keyboard mode from a Chat screen shows that Chat's last Message as context; typing reaches the iPhone; echo line clears after 10 s.
  7. Settings with "Press Enter to confirm" showing, then G0: back from Keyboard mode the prompt is gone.
  8. Live Message: beep with sound on; Banner in Keyboard mode; Inbox updates in Reading mode; a Chat screen covered by Settings shows it read only once back on top.
  9. Display timeout: screen turns off; in Reading mode a key only turns it on; in Keyboard mode the key also reaches the iPhone; G0 only turns it on.
  10. Fn+Enter starts Siri from Reading mode and Keyboard mode; ` right after closes it.
  11. Fn+Space toggles the iPhone on-screen keyboard in Keyboard mode.
  12. Status bar: mode, link state, unread count and battery as before, on every screen.
  13. Fn+Del held 3 s in Reading mode resets pairing and restarts (optional: needs pairing again).
- **Regression checklist outcome** (2026-11-13, user on the Cardputer with a serial capture): items 1–6, 8, 9, 10 and 12 pass, confirmed by the user and seen in the log (Reading mode keys, Settings opened and changed, "Clear messages" run, Keyboard mode opened five times and keys sent, three live Messages, display off and on, Siri requested and closed twice). Items 7 and 11 confirmed by the user only: they leave no log line. Item 13 (pairing reset) not run, as it was optional. /dev, 2026-11-13.
