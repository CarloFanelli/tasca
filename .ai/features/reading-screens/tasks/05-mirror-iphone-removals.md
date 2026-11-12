# Task 05: mirror-iphone-removals

## Objective
The Cardputer shows only what is still in the iPhone's notifications: a Message read or cleared on the iPhone disappears, also when that happened while disconnected.

## Context
**Satisfies**: REQ-13, REQ-14
Found on hardware: read Messages stayed in the Inbox, but the Cardputer never shows the user's replies, so keeping them is no real chat history. The user chose a mirror of the iPhone.

## Depends on
04-keyboard-context-banner

## Expected Behavior
- A notification removed on the iPhone deletes its Message; the counts and the Inbox follow; an empty Chat leaves the Inbox.
- Messages read on the Cardputer stay, as read, until removed on the iPhone.
- After a reconnection, once every resent notification is answered, Messages the iPhone did not resend are deleted.

## Acceptance Criteria
- [ ] A removal on the iPhone deletes the Message and drops its Chat when empty (REQ-13)
- [ ] A Message read on the Cardputer stays until the iPhone removes it (REQ-13)
- [ ] After a complete reconnection import, Messages not resent are deleted (REQ-14)
- [ ] A lost answer during the import deletes nothing for that connection (REQ-14)

## Edge Cases
- Removed event for a Message already read on the Cardputer → deleted like any other (REQ-7, REQ-13)
- Removed event for a notification never stored → ignored (REQ-13)
- Open Chat screen loses its last Message → back to the Inbox (REQ-9, REQ-13)
- Message read on the iPhone while disconnected → deleted once the import ends (REQ-14)
- Answer lost during the import → nothing deleted; `W` line (REQ-14)
- Disconnection before the import ends → nothing deleted (REQ-14)
- New Message during the import → kept (REQ-14)
