# Task 01: menu

## Objective

Fn+G0 opens the Menu from any screen and mode: Music, WhatsApp and Settings, with the unread count; Enter opens the item in Reading mode, ` or Fn+G0 closes it.

## Context

**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-4, REQ-5, REQ-10

The entry point for everything else. Music opens the Music screen, which shows "Nothing playing" until Task 02 brings the iPhone's data; the Menu's Music line shows the title from Task 02 on.

## Depends on

None

## Expected Behavior

- Fn+G0 opens the Menu over the screen on top, in Reading mode or Keyboard mode; no key reaches the iPhone while it is open.
- `;` and `.` move the selection without wrapping; Enter opens the selected item in Reading mode.
- Music opens the Music screen; WhatsApp opens the Inbox; Settings opens the Settings screen.
- WhatsApp shows the unread count; Music shows "Nothing playing" until Task 02.
- ` or Fn+G0 closes the Menu back to the previous screen and mode; ` on the Music screen goes back to the Inbox.

## Acceptance Criteria

- [ ] Fn+G0 opens the Menu over any screen, in Reading mode and Keyboard mode (REQ-1)
- [ ] `;` `.` move the selection, stopping at the ends; Enter opens the item in Reading mode (REQ-2, REQ-4)
- [ ] ` or Fn+G0 closes the Menu back to the same screen and mode (REQ-3)
- [ ] WhatsApp shows the unread count next to it (REQ-5)
- [ ] ` on the Music screen shows the Inbox (REQ-10)

## Edge Cases

- Fn+G0 with the screen off → only wakes the screen, like G0 today (REQ-1)
- Fn+G0 in Keyboard mode → Menu opens, no key is sent to the iPhone; opening an item switches to Reading mode (REQ-1, REQ-2)
- Fn+G0 while a Settings action waits for confirmation → prompt cancelled, Menu opens (REQ-1)
- G0 alone while the Menu is open → closes the Menu and switches mode as today (REQ-3)
- Selection at the first or last item → stays there; no wrap (REQ-2)
- Item opened is the current screen (e.g. Settings from Settings) → that screen shown from its start (REQ-4)
- Message arrives on the Music screen → beep and wake as today; screen stays on Music (REQ-6)
- Fn+G0 on the Connection screen or the Call screen → does nothing, like G0 there (REQ-1 · from code)
