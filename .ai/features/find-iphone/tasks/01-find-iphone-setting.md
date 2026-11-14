# Task 01: find-iphone-setting

## Objective

The Settings screen has a "Find iPhone" row, On or Off, On by default and kept across restarts; the list scrolls so every row stays reachable.

## Context

**Satisfies**: REQ-1

The switch the Out-of-range alert and the Find iPhone screen both read. A seventh row no longer fits the screen, so the Settings list scrolls with the selection.

## Depends on

None

## Expected Behavior

- The Settings screen lists "Find iPhone" after the other settings, showing On or Off.
- `,` and `/` toggle it; the value is saved at once and survives a restart.
- On a first start after the update, with no saved value, it reads On.
- The Settings list shows the rows that fit and scrolls to keep the selected row visible, actions included.

## Acceptance Criteria

- [ ] "Find iPhone" is listed in Settings with On or Off (REQ-1)
- [ ] `,` and `/` toggle it and the value survives a restart (REQ-1)
- [ ] With no saved value it reads On (REQ-1)
- [ ] Every Settings row, "Clear messages" included, can be selected and is fully visible when selected (REQ-1)

## Edge Cases

- Find iPhone row changed with `,` or `/` → toggles On/Off, saved at once like the other rows (REQ-1)
- First start after the update, no saved value → Find iPhone is On (REQ-1)
- Seven rows no longer fit the Settings screen → the list scrolls with the selection; no row is cut off (REQ-1 · from code)
