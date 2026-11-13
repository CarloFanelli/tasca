---
feature: clear-notifications
version: 1.0
status: approved
completed: true
base: 400cb1e089af2a71fe909db5e4829ced1c88588a
---

# Clear Notifications

## Overview

In Reading mode, Del clears a Chat: its notifications are removed from the iPhone, and its Messages leave the Cardputer once the iPhone confirms. The first Del asks "Press Del again to clear"; a second Del within 3 s clears. It works on the selected Chat in the Inbox and on the open Chat screen. Messages the iPhone refuses to clear stay, and the Cardputer says so. In Keyboard mode Del is still Backspace on the iPhone.

## Project Context

- Each stored **Message** is linked to its iPhone notification only while connected; after a reconnection, PreExisting notifications are linked again by content.
- The Cardputer already sends ANCS notification actions (answer and decline on the **Call screen**); the negative action means "Clear" for most apps.
- A notification removed on the iPhone already deletes its Message on the Cardputer; a Chat with no Messages leaves the **Inbox**, and its **Chat screen** closes.
- Settings "Clear messages" empties the Cardputer only, after Enter twice; the iPhone is untouched.
- Fn+Del held 3 s resets pairing; Del alone is free in Reading mode.

## Functional Requirements

### Asking

- REQ-1: In Reading mode, Del on the Inbox shows "Press Del again to clear" for the selected Chat.
- REQ-2: In Reading mode, Del on a Chat screen shows the same prompt for that Chat.
- REQ-3: Any other key, or 3 s without a second Del, removes the prompt and clears nothing.

### Clearing

- REQ-4: A second Del within 3 s sends the negative action for every linked Message of that Chat.
- REQ-5: Messages leave the Cardputer only when the iPhone reports their notification removed.
- REQ-6: When a cleared Chat has no Messages left, the Inbox shows, its selection on the next Chat.
- REQ-7: Messages the iPhone refuses to clear, or not linked, stay; "Not cleared on the iPhone" shows for 3 s.

### Keys and documentation

- REQ-8: Del in Keyboard mode still sends Backspace; Fn+Del never asks nor clears.
- REQ-9: The README lists Del in the Reading mode keys and explains what clearing does on the iPhone.

## External capabilities relied on

- ANCS Perform Notification Action, negative action — iOS removes the notification for apps that offer it; WhatsApp's behavior to be verified on the user's iPhone.

## Non-Functional Requirements

- **Performance** — Clearing a Chat of 20 Messages sends every action within 2 s; keys stay usable meanwhile.
- **Security** — The serial log records counts and UIDs only, never a Chat name, sender or text.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| Inbox empty ("No messages yet") | Del does nothing | REQ-1 |
| Chat with one Message | Same flow; the Chat leaves the Inbox | REQ-4, REQ-6 |
| Chat with 200 Messages (store limit) | Every linked Message gets its action; keys stay usable | REQ-4 |
| Del pressed a third time while clearing | Ignored until the clearing ends | REQ-4 |
| `;` or `.` while the prompt shows | Prompt removed; selection moves as usual | REQ-3 |
| Opt+s, Enter, G0, Fn+Enter while the prompt shows | Prompt removed; the key does what it does today | REQ-3 |
| A new Message arrives in that Chat while the prompt shows | Prompt stays; the second Del clears it too if linked | REQ-1, REQ-4 |
| A new Message arrives in that Chat during clearing | Not cleared; the Chat stays with it | REQ-4, REQ-5 |
| Del with the screen off | Only turns the screen on, as any key in Reading mode | REQ-1, REQ-2 |
| Fn+Del held 3 s | Pairing reset as today; no prompt, nothing cleared | REQ-8 |
| Fn+Del released before 3 s | Nothing happens | REQ-8 |
| Del in Keyboard mode, also over a Chat screen | Backspace sent to the iPhone | REQ-8 |
| iPhone refuses the action (error, or no negative action on that notification) | Message stays; "Not cleared on the iPhone"; `W` line with the error | REQ-7 |
| Message kept from before the last connection and not resent (unlinked) | Stays; "Not cleared on the iPhone" | REQ-7 |
| Every Message of the Chat refused | Chat unchanged; message shown | REQ-7 |
| Some cleared, some refused | Cleared ones leave; the Chat stays with the rest; message shown | REQ-5, REQ-7 |
| iPhone disconnects during clearing | Remaining actions not sent; the Connection screen covers; Messages already removed stay removed | REQ-4, REQ-5 |
| iPhone accepts but never sends the removal | Message stays on the Cardputer | REQ-5 |
| Removal also arrives for the Message open on the Chat screen | Chat screen closes once the Chat is gone; the Inbox shows | REQ-6 |
| Cleared Chat was the last in the Inbox | "No messages yet" | REQ-6 |
| Settings "Clear messages" | Unchanged: clears the Cardputer only | REQ-8 |
| N/A — permission: one bonded iPhone, no user roles | — | REQ-1, REQ-2, REQ-8 |
| N/A — concurrent: one clearing at a time, from the main loop | — | REQ-6 |
| N/A — integrity: the README is static text | — | REQ-9 |
| N/A — network: the README is static text | — | REQ-9 |

## Out of Scope

- **Clearing one Message only** — chosen: WhatsApp is read by Chat.
- **Clearing every Chat at once on the iPhone** — not asked; Settings "Clear messages" stays local.
- **Marking as read on the iPhone** — iOS has no such action for accessories.
- **Clearing from Keyboard mode** — Del is Backspace there.
