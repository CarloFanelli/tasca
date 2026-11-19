---
feature: mesh-chat
version: 1.0
status: approved
completed: true
base: 34eab6e9005a8092099e5d979dbc3c44c9137e29
---

# Mesh chat

## Overview

With the LoRa unit (EU868) on the Grove port, the Cardputer becomes a Meshtastic node on the public channel used in Italy (EU_868, MediumFast). The **Mesh screen**, opened from the Menu, shows the channel's messages and lets the user write and send one from the keyboard, with a mark telling whether a nearby node heard it. The **Node list** shows the nodes heard. Messages and nodes are kept on the microSD card. The user picks the **Mesh name** in Settings, and a separate Mesh sound switch controls the beep. The Mesh screen works without the iPhone; when the iPhone connects, it also gives the Cardputer the time of day.

## Project Context

- One Grove port; the LoRa unit is driven with AT commands over a serial line, and on 2026-11-19 it received and decrypted a MediumFast public-channel packet in Leno.
- The public channel's key is published by Meshtastic; Italian nodes use EU_868 MediumFast (869.525 MHz).
- The Connection screen covers every screen while the iPhone is not connected; Fn+G0 does nothing there today.
- The Menu lists Music, Find iPhone, WhatsApp and Settings; Settings rows change with `,` and `/`, actions run on Enter.
- The Cardputer has no clock of its own; the firmware does not use the microSD card yet.
- The Bluetooth services Tasca offers must not change once bonded; reading a service the iPhone offers is allowed.
- Radio rules for 869.4–869.65 MHz: at most 10% of the time on air.
- On-screen text stays in English; the main loop is the only code that draws and plays sounds.

## Functional Requirements

### LoRa unit

- REQ-1: Tasca detects the LoRa unit on the Grove port within 5 s of startup or of plugging it in, without a restart.
- REQ-2: Without the LoRa unit, the Mesh screen reads "Connect the LoRa unit" and nothing can be sent.

### Reaching the Mesh screen

- REQ-3: The Menu lists "Mesh" after Find iPhone, with the count of unread Mesh messages; Enter opens the Mesh screen.
- REQ-4: Fn+G0 opens the Menu from the Connection screen; there, only Mesh and Settings open.
- REQ-5: While the Mesh screen or the Node list is open, a lost iPhone link does not bring up the Connection screen.

### Mesh screen

- REQ-6: The Mesh screen lists public-channel text messages, newest at the bottom, each with time, sender's short name and text.
- REQ-7: `;` and `.` scroll the Mesh screen; `` ` `` goes back to the screen the Menu was opened from.
- REQ-8: Opening the Mesh screen marks every Mesh message read.

### Writing

- REQ-9: Enter on the Mesh screen opens a compose line; keys type their legends, Fn+a/e/i/o/u give à è ì ò ù, Ctrl+e gives é.
- REQ-10: Enter in the compose line sends its text on the public channel and adds it to the list as the user's message.
- REQ-11: The compose line shows the bytes left and accepts at most 200 bytes; `` ` `` closes it without sending.
- REQ-12: The user's message reads "sent", then "heard" when a node's rebroadcast is received, or "not heard" after 30 s.

### Node list

- REQ-13: Tab on the Mesh screen opens the Node list: long and short name, signal strength and time since last heard, newest first.

### Mesh name

- REQ-14: Settings has a "Mesh name" row: Enter edits it with the keyboard, up to 24 characters; Enter saves, `` ` `` cancels.
- REQ-15: The short name is the Mesh name's first 4 characters; with no name set, the Mesh name is "Tasca" plus 4 node-ID digits.
- REQ-16: The Cardputer announces its Mesh name on the channel when the LoRa unit is detected, when the name changes, and every 3 hours.

### Alerts

- REQ-17: Settings has a "Mesh sound" row, On by default; each received Mesh message beeps when it is on, whatever the sound row says.
- REQ-18: A received Mesh message turns the screen on when wake on new message is on.

### Time

- REQ-19: At each iPhone connection Tasca reads the iPhone's clock and keeps it running until the Cardputer restarts.
- REQ-20: Mesh messages show their arrival time from that clock, or the time elapsed since arrival ("5m") when no clock is known.

### Storage

- REQ-21: The last 500 Mesh messages and the Node list are kept on the microSD card and shown again after a restart.
- REQ-22: Del twice on the Mesh screen clears every Mesh message, on the screen and on the microSD card.

## Data & Dependencies

### Information the feature must retain

- Mesh message — sender's node, short name, text, arrival time when known, read state; for the user's own messages also the delivery mark. Created on receipt or send; the oldest goes past 500; all go on Del twice. Kept on the microSD card.
- Node — node ID, long and short name, last signal strength, last heard time. Created or updated by any packet from it; kept on the microSD card.
- Mesh name — text up to 24 characters, empty meaning the default; kept across restarts like the other settings.
- Mesh sound — On or Off, On by default; kept across restarts.
- Clock — time of day read from the iPhone at connection; RAM only, lost on restart.

### External capabilities relied on

- LoRa unit (EU868) — sends and receives LoRa packets with the Meshtastic radio parameters.
- Meshtastic public channel — the network of nodes that relay messages; its key and packet format are published.
- The iPhone's time service — gives the time of day to accessories over Bluetooth.
- microSD card — keeps the Mesh history across restarts.

## Non-Functional Requirements

- **Performance** — a received text appears on the Mesh screen within 2 s of the packet arriving; a message leaves the radio within 1 s of Enter.
- **Security** — only public-channel messages are read or sent; the README states that the Mesh history on the microSD card is readable by anyone holding the card.
- **Validation** — at most 200 bytes per message and 24 characters per Mesh name; an empty compose line sends nothing.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| LoRa unit plugged in while Tasca runs | Detected within 5 s; announcement sent; Mesh screen starts listing | REQ-1, REQ-16 |
| LoRa unit unplugged while Tasca runs | Within 5 s the Mesh screen reads "Connect the LoRa unit"; history stays | REQ-1, REQ-2 |
| Another device on the Grove port | Not taken for the LoRa unit: no answer to the unit's check, "Connect the LoRa unit" | REQ-1 |
| Enter on the Mesh screen without the LoRa unit | Compose line does not open | REQ-2, REQ-9 |
| No Mesh message yet | Mesh screen reads "No messages yet" | REQ-6 |
| The same message relayed by several nodes | Shown once | REQ-6 |
| Packets that are not text (position, telemetry, node announcements) | Not listed; they update the Node list | REQ-6, REQ-13 |
| Packets of another channel, or direct messages | Ignored | REQ-6 |
| Message from a node not yet announced | Short name is "!" plus the last 4 digits of its node ID until its name arrives | REQ-6, REQ-13 |
| Characters the screen cannot draw (emoji) | Shown as on the Chat screen | REQ-6 |
| Message longer than one line | Wraps over the next lines | REQ-6 |
| Mesh message arrives while composing | List updates, compose line and its text stay | REQ-9 |
| Mesh message arrives on another screen | Unread count in the Menu rises; beep and wake per REQ-17, REQ-18 | REQ-3, REQ-17 |
| Mesh screen opened from the Connection screen, then the iPhone connects | Mesh screen stays; ` goes back to the Inbox | REQ-4, REQ-7 |
| iPhone link lost while on the Mesh screen | Mesh screen stays; Out-of-range alert beeps as usual; leaving the Mesh screen shows the Connection screen | REQ-5 |
| Music, Find iPhone or WhatsApp chosen from the Menu without the iPhone | Nothing happens | REQ-4 |
| Text at exactly 200 bytes | Accepted; further keys type nothing; accented letters count 2 bytes | REQ-11 |
| Enter on an empty compose line | Nothing sent, line stays open | REQ-10 |
| LoRa unit unplugged while sending | Message reads "not sent" | REQ-10, REQ-12 |
| No node hears the message | "not heard" after 30 s; the message stays in the list | REQ-12 |
| Radio time in the last hour reaches the 10% limit | Send refused; compose line reads "Radio limit: wait" | REQ-10 |
| Our own message heard back from a relay | Not listed twice; marks it "heard" | REQ-12 |
| No node heard yet | Node list reads "No nodes heard yet" | REQ-13 |
| Mesh name cleared and saved | Default name "Tasca" plus 4 digits is used and announced | REQ-14, REQ-15 |
| Mesh name at 24 characters | Further keys type nothing | REQ-14 |
| Mesh name shorter than 4 characters | Short name is the whole name | REQ-15 |
| Mesh name changed without the LoRa unit | Saved; announced when the unit is detected | REQ-16 |
| Mesh sound off, sound on | Mesh messages do not beep, WhatsApp messages do | REQ-17 |
| Mesh sound on, sound off | Mesh messages beep, WhatsApp messages do not | REQ-17 |
| iPhone does not give its time | Messages show elapsed time | REQ-19, REQ-20 |
| iPhone disconnects after giving the time | Clock keeps running until restart | REQ-19 |
| Messages from before a restart, clock not yet known | Shown with their stored time, or "--:--" when they had none | REQ-20, REQ-21 |
| No microSD card | Mesh works; history kept only until restart; the Mesh screen title reads "not saved" | REQ-21 |
| microSD card full or write fails | Same as no card | REQ-21 |
| More than 500 messages | The oldest is removed | REQ-21 |
| Del pressed once | "Press Del again to clear", as on the Chat screen | REQ-22 |
| N/A — concurrent: one user, one radio; the main loop is the only writer of the history | — | REQ-6, REQ-21 |
| N/A — permission: the public channel has no access control | — | REQ-6, REQ-10 |

## Out of Scope

- **Direct messages** — Meshtastic encrypts them with per-node keys; a feature of its own.
- **Private channels and other presets** — the Italian nodes found use the public MediumFast channel; picking channels comes later.
- **Relaying other nodes' packets** — the Cardputer listens and sends but does not rebroadcast, to save battery and radio time.
- **Sharing position** — the Cardputer has no GPS.
- **LoRaWAN / The Things Network** — no active gateway near Leno.
- **RFID unit** — a different unit on the same port; a feature of its own.
