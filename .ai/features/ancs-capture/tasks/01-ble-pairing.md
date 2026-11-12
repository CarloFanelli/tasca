# Task 01: ble-pairing

## Objective

"Tasca" appears in the iPhone's Bluetooth settings, pairs without a PIN and reconnects by itself after a restart or a disconnection; a status line and the serial log show the BLE phase.

## Context

**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-4, REQ-6, REQ-13, REQ-14

Everything in M1 rides on a bonded, encrypted link, and iOS caches the GATT table at the first pairing: the full table (HID included, silent) must ship in this very first BLE firmware. The status line starts here with the phases this task owns; Tasks 02 and 03 add their own states.

## Depends on

None

## Expected Behavior

- While not connected the Cardputer advertises as "Tasca" with keyboard appearance, the HID service UUID and an ANCS solicitation; the name is in the scan response.
- Selecting "Tasca" on the iPhone bonds with LE Secure Connections, Just Works: no PIN on either device.
- The GATT server exposes HID (keyboard and consumer control), Battery and Device Information; HID never sends a report.
- Bonds survive reboot and reflash; after a Cardputer restart the iPhone reconnects without pairing again.
- After a disconnection advertising resumes within 1 s.
- Only one iPhone can be bonded at a time.
- A status line under the version shows advertising, connected or secured; every phase change and pairing result is logged tagged `BLE`.

## Acceptance Criteria

- [ ] "Tasca" is listed in iPhone Bluetooth settings with a keyboard icon while the Cardputer is not connected (REQ-1)
- [ ] Pairing completes with no PIN or confirmation on either device (REQ-2)
- [ ] The iPhone sees HID, Battery and Device Information services; no key ever reaches the iPhone (REQ-3)
- [ ] After a Cardputer reboot or reflash the iPhone reconnects and the link is encrypted without a new pairing (REQ-4)
- [ ] After a disconnection advertising restarts within 1 s (REQ-6)
- [ ] The status line shows advertising, connected and secured in turn; each change is logged tagged `BLE` (REQ-13, REQ-14)

## Edge Cases

- Advertising payload with solicitation and HID UUID → fits in 31 bytes; the name goes in the scan response (REQ-1 from the spec edge-case table)
- Already connected to the iPhone → no advertising while the connection lasts (REQ-1 from the spec edge-case table)
- User cancels the pairing on the iPhone → keeps advertising, logs a `W` pairing-failed line (REQ-2 from the spec edge-case table)
- A second iPhone tries to pair while one is bonded → pairing refused, `W` line; Fn+Del reset needed to pair another device (REQ-2 from the spec edge-case table)
- Cardputer bonds were reset but the iPhone still remembers it → pairing fails; CLAUDE.md documents "Forget This Device" then pair again (REQ-2 from the spec edge-case table)
- iOS has cached the GATT table → no service is added, removed or changed after this firmware (REQ-3 from the spec edge-case table)
- N/A — concurrent/permission/network: the table is static (REQ-3 from the spec edge-case table)
- User did "Forget This Device" on the iPhone → reconnection fails encryption: `W` line, status shows "pairing invalid — Fn+Del" (REQ-4 from the spec edge-case table)
- Same iPhone pairs again after "Forget This Device" → old bond replaced automatically, `I` line logged, no Fn+Del needed (REQ-4 · from code)
- iPhone Bluetooth off at Cardputer boot → advertises until the iPhone comes back, then reconnects without pairing (REQ-4 from the spec edge-case table)
- iPhone walks out of range → disconnection logged, advertising resumes within 1 s, reconnects when back (REQ-6 from the spec edge-case table)
- Display failed to initialize → status line skipped; serial logging unaffected (REQ-13 from the spec edge-case table)

## Notes from implementation

- **REQ-4 / AC 4 — divergence**: when encryption fails the status line shows "pairing failed" (kept while advertising, until the next connection) instead of "pairing invalid — Fn+Del": after "Forget This Device" the iPhone re-pairs on its own (see the edge case from code), so Fn+Del is not the fix the message would suggest. /dev, 2026-11-12.
- **Known limitation**: verified on hardware — pairing without PIN, encrypted bond with identity address, reconnection after power cycle, after reflash and after iPhone Bluetooth off/on, status line, `BLE` logs. Not exercised: pairing cancelled on the iPhone and the second-device refusal (no second iPhone available). /dev, 2026-11-12.
