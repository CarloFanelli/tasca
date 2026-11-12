# Task 03: ancs-subscription

## Objective

Once the link is encrypted the Cardputer subscribes to the iPhone's notifications and dumps the 8-byte event of every notification on serial, PreExisting ones included, in a hex format that can be pasted into a test.

## Context

**Satisfies**: REQ-7, REQ-8, REQ-12, REQ-13, REQ-14

This is the first ANCS contact and the main technical risk of M1 (GATT client on the connection where the Cardputer is peripheral). The hex dump format defined here is reused for Data Source in Task 04.

## Depends on

- Task 01

## Expected Behavior

- ANCS discovery starts only after the link is encrypted; Data Source is subscribed before Notification Source.
- Every Notification Source packet is dumped, PreExisting ones included, as `ANCS`-tagged lines: channel, total length, byte offset, at most 32 bytes in hex.
- The status line shows "ANCS active" and the number of notifications received since boot.
- Subscription results and failures are logged tagged `ANCS`.

## Acceptance Criteria

- [ ] Subscriptions happen only after encryption, Data Source before Notification Source (REQ-7)
- [ ] A WhatsApp message received on the iPhone produces a Notification Source dump on serial (REQ-8)
- [ ] At connection every PreExisting notification is dumped (REQ-8)
- [ ] Dump lines follow `[D][ANCS] NS len=N +offset: hex…`, at most 32 bytes per line (REQ-12)
- [ ] Host tests cover the dump format: 8 bytes, exactly 32, 33, empty (REQ-12)
- [ ] Status line shows "ANCS active" and the notification count; subscription outcome logged (REQ-13, REQ-14)

## Edge Cases

- Disconnection during the ANCS setup → setup restarts from encryption on the next connection (REQ-6 from the spec edge-case table)
- User did not allow notification sharing → subscription fails: `W` line, status shows "ANCS denied", connection kept (REQ-7 from the spec edge-case table)
- Central without ANCS (not an iPhone) → `W` "ANCS not found" line, status stays "secured" (REQ-7 from the spec edge-case table)
- Dozens of PreExisting notifications at connection → every one is dumped; none is lost (REQ-8 from the spec edge-case table)
- Removed event → dumped like any other event (REQ-8 from the spec edge-case table)
- Packet of exactly 32 bytes / 33 bytes → one line / two lines, the second with offset +32 (REQ-12 from the spec edge-case table)
- Notification counter after a reboot → restarts from 0 (REQ-13 from the spec edge-case table)

## Notes from implementation

- **Known limitation**: with "Share System Notifications" turned off for Tasca on the iPhone, iOS accepted both subscriptions and simply sent nothing, so the status showed "ANCS active - 0 notifications" instead of "ANCS denied". "ANCS denied" only appears if iOS refuses the subscription write. Turning the switch on takes effect from the next connection. Documented in CLAUDE.md.
- **Impacts Task 04**: Data Source is already subscribed and its packets are dumped as `DS` lines (same queue and dump path); Task 04 only adds the attribute requests and their completion tracking.
- Verified on hardware: 109 PreExisting notifications dumped at connection with none dropped, a new WhatsApp message (EventID Added) and a Removed event dumped live, counter 109 → 110. Not exercised: a disconnection during setup and a central without ANCS.
