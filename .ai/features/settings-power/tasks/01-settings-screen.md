# Task 01: settings-screen

## Objective
Opt+s opens a Settings screen listing brightness, sound, display timeout and wake on new Message; values change with `,` and `/` and survive restarts. Brightness takes effect at once.

## Context
**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-4, REQ-5
Every other task reads a setting from here: sound (beep), display timeout and wake (screen sleep). Brightness is the only setting whose effect lands in this task.

## Depends on
None

## Expected Behavior
- Opt+s in Reading mode opens Settings from the Inbox or a Chat screen; `` ` `` returns to that screen.
- Settings lists Brightness, Sound, Display timeout, Wake on message, then the two actions (added by Task 02).
- `;` and `.` move the selection; `,` and `/`, with or without Fn, step the selected value back and forward.
- A changed value is applied and saved at once; it is the value after a restart.
- The status bar stays visible on Settings.

## Acceptance Criteria
- [ ] Opt+s opens Settings from the Inbox and from a Chat screen; `` ` `` returns to the screen it came from (REQ-1)
- [ ] `;`/`.` select, `,`/`/` step values, with or without Fn (REQ-2)
- [ ] Brightness steps 20–100 % by 20 and changes the screen at once (REQ-3)
- [ ] Display timeout offers 15 s, 30 s, 1 min, 2 min, 5 min, Never (REQ-4)
- [ ] After a restart every setting shows its last value; first start shows 60 %, on, 30 s, on (REQ-5)

## Edge Cases
- Opt+s in Keyboard mode → sent to the iPhone like any key; Settings not opened (REQ-1)
- Opt+s while Settings is open → ignored (REQ-1)
- `;`/`.` past the first or last setting → selection stays; no wrap-around (REQ-2)
- `,`/`/` past the first or last value → value stays; no wrap-around (REQ-2)
- Brightness changed → screen brightness changes at once (REQ-3)
- Stored settings missing or unreadable (first start, corrupted) → defaults used; `W` line when unreadable (REQ-5)
- Saving a setting fails → value applies until restart; `W` line (REQ-5)
- Display failed to initialize → settings still loaded from storage (REQ-3)
