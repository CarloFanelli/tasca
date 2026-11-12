---
feature: ancs-capture
version: 1.0
status: approved
completed: true
base: c3a711fe9d8583d6ff4ce6da87b44c7f5ea4ed79
---

# ANCS Capture (M1)

## Overview

Makes the Cardputer pair with an iPhone over BLE and capture its notifications: it
advertises as a keyboard soliciting ANCS, bonds with Just Works, exposes the full GATT
table (HID included, still inert), subscribes to ANCS once the link is encrypted, asks
the content of every notification and dumps every raw ANCS packet on serial in a
fixture-friendly hex format. A status line on screen shows the BLE phase. It retires
the first technical risk of the project (ANCS client on the ESP32-S3 with NimBLE) and
produces the real packets M3's parser tests will be built on.

## Project Context

- Builds on firmware-skeleton: tagged serial log (256-char lines), "Hello" screen, key events, host tests on `src/core/`.
- The Cardputer has BLE only (no Classic) and no PSRAM: about 330 KB of free heap.
- BLE stack is NimBLE-Arduino 2.x (Tasca spec §5.1); fallback to ESP-IDF NimBLE if M1 fails with it.
- iOS caches the GATT table at pairing: it must be final from the first bonded firmware.
- ANCS is reachable only on an encrypted, bonded link, after the user allows notification sharing.
- Concurrency rules in CLAUDE.md apply: BLE callbacks only copy and enqueue; serial, display and GATT writes happen in the main loop.

## Functional Requirements

### Advertising & pairing

- REQ-1: When not connected, the Cardputer advertises as "Tasca" with keyboard appearance, the HID service UUID and an ANCS service solicitation.
- REQ-2: Selecting "Tasca" in iPhone Bluetooth settings bonds with LE Secure Connections, Just Works, with no PIN on either device.
- REQ-3: From the first pairing the GATT server exposes HID (keyboard and consumer control), Battery and Device Information; HID sends no reports.
- REQ-4: Bonding keys survive reboot and reflash; after a Cardputer restart the iPhone reconnects without a new pairing.
- REQ-5: Holding Fn+Del for 3 s erases stored bonds, logs and shows it on screen, and resumes advertising.
- REQ-6: After a disconnection the Cardputer resumes advertising within 1 s.

### ANCS capture

- REQ-7: ANCS discovery starts only after the link is encrypted; Data Source is subscribed before Notification Source.
- REQ-8: Every Notification Source packet is dumped on serial, PreExisting ones included.
- REQ-9: For every added or modified notification, PreExisting included, a Get Notification Attributes request asks AppIdentifier, Title (64), Subtitle (64), Message (512), Date.
- REQ-10: Attribute requests are sent one at a time; the next starts when the previous response is complete or after 2 s.
- REQ-11: Every Data Source packet is dumped on serial.
- REQ-12: Dumps are `ANCS`-tagged lines with channel (NS/DS), total length, byte offset and at most 32 bytes in hex.

### Status

- REQ-13: A status line under the version shows the BLE phase (advertising, connected, secured, ANCS active) and notifications received since boot.
- REQ-14: Every BLE phase change, pairing result and ANCS subscription result is logged on serial tagged `BLE` or `ANCS`.

## Data & Dependencies

### Information the feature must retain

- Bond with the iPhone — created at pairing, kept across reboots and reflashes, erased by the Fn+Del reset. At most one.
- Pending attribute requests — created per added/modified notification, consumed when the response completes or times out, cleared on disconnection. Never persisted.

### External capabilities relied on

- BLE peripheral with GATT server and client on the same connection — the BLE stack.
- Apple Notification Center Service on the iPhone — source of notifications and their content.

## Non-Functional Requirements

- **Security** — Notification content is never stored on the device: it exists only in RAM while being dumped to serial.
- **Security** — ANCS characteristics are accessed only on an encrypted, bonded link.
- **Validation** — The BLE library is pinned to an exact version, like every other dependency.
- **Performance** — With an empty request queue, the dump of a new notification's content completes within 3 s of its arrival.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| Advertising payload with solicitation and HID UUID | Fits in 31 bytes; the name goes in the scan response | REQ-1 |
| Already connected to the iPhone | No advertising while the connection lasts (one connection at a time) | REQ-1 |
| User cancels the pairing on the iPhone | Cardputer keeps advertising, logs a `W` pairing-failed line | REQ-2 |
| A second iPhone tries to pair while one is bonded | Pairing refused, `W` line logged; Fn+Del reset needed to pair another device | REQ-2 |
| Cardputer bonds were reset but the iPhone still remembers it | Pairing fails; CLAUDE.md documents "Forget This Device" on the iPhone, then pair again | REQ-2 |
| iOS has cached the GATT table | No service is added, removed or changed after the first bonded firmware | REQ-3 |
| N/A — concurrent/permission/network: the table is static | — | REQ-3 |
| User did "Forget This Device" on the iPhone | Reconnection fails encryption: `W` line, status shows "pairing invalid — Fn+Del" | REQ-4 |
| iPhone Bluetooth off at Cardputer boot | Cardputer advertises until the iPhone comes back, then reconnects without pairing | REQ-4 |
| Fn+Del released before 3 s | Nothing is erased | REQ-5 |
| Fn+Del held while connected | Disconnects, erases bonds, resumes advertising | REQ-5 |
| Fn+Del with no stored bond | Logs "no bonds", keeps advertising | REQ-5 |
| iPhone walks out of range | Disconnection logged, advertising resumes within 1 s, reconnects when back | REQ-6 |
| Disconnection during the ANCS setup | Setup restarts from encryption on the next connection | REQ-6 |
| User did not allow notification sharing | Subscription fails: `W` line, status shows "ANCS denied", connection kept | REQ-7 |
| Central without ANCS (not an iPhone) | `W` "ANCS not found" line, status stays "secured" | REQ-7 |
| Dozens of PreExisting notifications at connection | Every one is dumped; none is lost | REQ-8 |
| Removed event | Dumped like any other event; no attribute request | REQ-8 |
| Notification removed before its attributes are read | iOS answers with an error: `W` line, next request starts | REQ-9 |
| Notification without subtitle or message | Request is identical; empty attributes appear in the dump | REQ-9 |
| No response within 2 s | `W` timeout line, next request starts | REQ-10 |
| New notifications while a request is pending | Queued in arrival order | REQ-10 |
| More than 64 pending requests | Requests beyond 64 are dropped with a `W` line; NS dumps continue | REQ-10 |
| Disconnection with requests pending | Queue cleared, `I` line with how many were dropped | REQ-10 |
| Response split across several Data Source packets | Each packet dumped separately, in arrival order | REQ-11 |
| Disconnection mid-response | Packets already received stay dumped; nothing else is printed for that request | REQ-11 |
| Packet of exactly 32 bytes / 33 bytes | One line / two lines, the second with offset +32 | REQ-12 |
| Display failed to initialize | Status line skipped; serial logging unaffected | REQ-13 |
| Notification counter after a reboot | Restarts from 0 | REQ-13 |
| N/A — REQ-14 only adds log lines to transitions covered above | — | REQ-14 |

## Out of Scope

- **Parsing ANCS responses** — milestone M3, built on the dumps captured here.
- **WhatsApp-only filtering** — needs the parser (M3); M1 dumps every app's notifications.
- **Message store, chat grouping, deduplication** — milestone M3.
- **Typing on the iPhone via HID** — milestone M2; the HID service is present but silent.
- **Notification actions (positive/negative)** — optional in the Tasca spec (RX-8), not needed to capture.
- **Settings screen and reset-pairing menu** — milestone M5; Fn+Del is the stopgap.
- **More than one bonded iPhone** — single-user device; a second phone needs a reset.
