# Task 02: bond-reset

## Objective

Holding Fn+Del for 3 seconds makes the Cardputer forget the bonded iPhone and become pairable again, with confirmation on screen and on serial.

## Context

**Satisfies**: REQ-5, REQ-13, REQ-14

During M1 and M2 pairing will be undone and redone often, and the settings menu with "Reset pairing" only arrives in M5. This stopgap needs only the Cardputer, no Mac.

## Depends on

- Task 01

## Expected Behavior

- Holding Fn and Del together for 3 s erases every stored bond, logs it tagged `BLE` and shows it on the status line.
- If connected, the Cardputer disconnects first; in every case it then advertises, ready for a new pairing.
- Releasing either key before 3 s erases nothing.
- With no bond stored, it logs "no bonds" and keeps advertising.

## Acceptance Criteria

- [ ] Fn+Del held 3 s erases the bond; the iPhone can then pair again as a new device after forgetting it (REQ-5)
- [ ] Fn+Del released before 3 s erases nothing (REQ-5)
- [ ] The reset is logged tagged `BLE` and shown on the status line (REQ-5, REQ-13, REQ-14)
- [ ] Host tests cover the 3 s hold: released early, held exactly 3 s, other keys pressed meanwhile (REQ-5)

## Edge Cases

- Fn+Del released before 3 s → nothing is erased (REQ-5 from the spec edge-case table)
- Fn+Del held while connected → disconnects, erases bonds, resumes advertising (REQ-5 from the spec edge-case table)
- Fn+Del with no stored bond → logs "no bonds", keeps advertising (REQ-5 from the spec edge-case table)

## Notes from implementation

- **REQ-5 — divergence**: after erasing the bond the Cardputer shows "pairing reset" for 1.5 s, then restarts itself instead of only resuming advertising. Verified on hardware: without a restart, iOS did not list "Tasca" under Other Devices after "Forget This Device", although the Mac's scan saw it advertising and connectable; after a restart it appears at once. With no stored bond nothing restarts: it only logs "no bonds".
