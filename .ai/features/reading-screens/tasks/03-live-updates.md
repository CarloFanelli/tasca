# Task 03: live-updates

## Objective
The Inbox and the open Chat screen follow new, updated and dropped Messages within 1 s, without losing the user's place.

## Context
**Satisfies**: REQ-7, REQ-9
Messages arrive while the user is reading; screens must stay correct and keep the selection on the same Chat even when the order changes.

## Depends on
02-chat-screen

## Expected Behavior
- A new or updated Message redraws the Inbox or the open Chat screen within 1 s.
- The selected Chat stays selected when it moves in the list.
- A Message arriving in the open Chat is shown and marked read at once.
- Messages dropped by the store limit disappear from the screens.

## Acceptance Criteria
- [ ] A new Message appears on the Inbox within 1 s, and the previously selected Chat is still selected (REQ-9)
- [ ] A new Message in the open Chat appears within 1 s and is read; total unread unchanged (REQ-7, REQ-9)
- [ ] An updated Message shows its new text in place (REQ-9)
- [ ] Dropped Messages disappear; the scroll position is clamped to the remaining lines (REQ-9)

## Edge Cases
- Chat screen open while its oldest Messages are dropped by the store limit → dropped Messages disappear; scroll position clamped (REQ-5, REQ-9)
- Every Message of the open Chat dropped → back to the Inbox (REQ-9)
- Message of the open Chat updated by the iPhone → its text replaced in place (REQ-9)
- Message arrives in another Chat while a Chat screen is open → open Chat unchanged; status bar unread count goes up (REQ-7, REQ-9)
- New Message moves the selected Chat to the top → selection follows that Chat (REQ-9)
- Selected Chat dropped entirely from the Inbox → selection moves to the nearest remaining Chat (REQ-9 · from code)
