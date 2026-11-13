# Task 02: not-cleared-notice

## Objective

When the iPhone refuses to clear some Messages, or some are no longer linked to a notification, they stay and the Cardputer shows "Not cleared on the iPhone" for 3 s.

## Context

**Satisfies**: REQ-7

The user chose that Cardputer and iPhone stay aligned: nothing leaves the Cardputer unless it left the iPhone.

## Depends on

Task 01

## Expected Behavior

- A negative action the iPhone refuses leaves its Message on the Cardputer and logs a `W` line with the error.
- Unlinked Messages of the Chat (kept from before the last connection) get no action and stay.
- When at least one Message of the clearing stays for either reason, "Not cleared on the iPhone" shows for 3 s where the prompt was.

## Acceptance Criteria

- [ ] A refused action keeps its Message and logs a `W` line with the error code (REQ-7)
- [ ] Unlinked Messages get no action and stay on the Cardputer (REQ-7)
- [ ] "Not cleared on the iPhone" shows for 3 s when at least one Message stays (REQ-7)

## Edge Cases

- iPhone refuses the action (error, or no negative action on that notification) → Message stays; "Not cleared on the iPhone"; `W` line with the error (REQ-7)
- Message kept from before the last connection and not resent (unlinked) → stays; "Not cleared on the iPhone" (REQ-7)
- Every Message of the Chat refused → Chat unchanged; message shown (REQ-7)
- Some cleared, some refused → cleared ones leave; the Chat stays with the rest; message shown (REQ-5, REQ-7)
