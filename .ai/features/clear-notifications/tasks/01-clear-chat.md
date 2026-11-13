# Task 01: clear-chat

## Objective

In Reading mode the user can Clear a Chat with Del twice, from the Inbox or the Chat screen: its notifications are removed from the iPhone and the Chat leaves the Cardputer.

## Context

**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-4, REQ-5, REQ-6, REQ-8

The whole flow when the iPhone accepts every action. Refusals and unlinked Messages, and their notice, come in Task 02.

## Depends on

None

## Expected Behavior

- Del on the Inbox shows "Press Del again to clear" on the selected Chat; Del on a Chat screen shows it for that Chat.
- Any other key, or 3 s without Del, removes the prompt; the key does what it does today.
- A second Del within 3 s sends the negative action for every linked Message of the Chat, without freezing the keys.
- Each Message leaves when the iPhone reports its notification removed; when the Chat is empty the Inbox shows with the next Chat selected.
- Del in Keyboard mode sends Backspace; Fn+Del never shows the prompt.

## Acceptance Criteria

- [ ] Del on the Inbox and on a Chat screen shows "Press Del again to clear" (REQ-1, REQ-2)
- [ ] Another key or 3 s without Del removes the prompt and clears nothing, checked by host tests (REQ-3)
- [ ] A second Del within 3 s sends one negative action per linked Message of that Chat (REQ-4)
- [ ] Messages leave only on the iPhone's removal; the emptied Chat leaves and the next Chat is selected (REQ-5, REQ-6)
- [ ] Del in Keyboard mode sends Backspace; Fn+Del, held or not, never prompts nor clears (REQ-8)

## Edge Cases

- Inbox empty ("No messages yet") → Del does nothing (REQ-1)
- Chat with one Message → same flow; the Chat leaves the Inbox (REQ-4, REQ-6)
- Chat with 200 Messages (store limit) → every linked Message gets its action; keys stay usable (REQ-4)
- Del pressed a third time while clearing → ignored until the clearing ends (REQ-4)
- `;` or `.` while the prompt shows → prompt removed; selection moves as usual (REQ-3)
- Opt+s, Enter, G0, Fn+Enter while the prompt shows → prompt removed; the key does what it does today (REQ-3)
- A new Message arrives in that Chat while the prompt shows → prompt stays; the second Del clears it too if linked (REQ-1, REQ-4)
- A new Message arrives in that Chat during clearing → not cleared; the Chat stays with it (REQ-4, REQ-5)
- Del with the screen off → only turns the screen on, as any key in Reading mode (REQ-1, REQ-2)
- Fn+Del held 3 s → pairing reset as today; no prompt, nothing cleared (REQ-8)
- Fn+Del released before 3 s → nothing happens (REQ-8)
- Del in Keyboard mode, also over a Chat screen → Backspace sent to the iPhone (REQ-8)
- iPhone disconnects during clearing → remaining actions not sent; the Connection screen covers; Messages already removed stay removed (REQ-4, REQ-5)
- iPhone accepts but never sends the removal → Message stays on the Cardputer (REQ-5)
- Removal also arrives for the Message open on the Chat screen → Chat screen closes once the Chat is gone; the Inbox shows (REQ-6)
- Cleared Chat was the last in the Inbox → "No messages yet" (REQ-6)
- Settings "Clear messages" → unchanged: clears the Cardputer only (REQ-8)
