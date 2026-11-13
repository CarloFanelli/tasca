---
feature: sd-sounds
version: 1.0
status: approved
completed: false
base: 400cb1e089af2a71fe909db5e4829ced1c88588a
---

# SD Sounds

## Overview

The user can pick their own sounds for live Messages and incoming calls from WAV files on the Cardputer's microSD card. The Cardputer creates `Tasca/Notifications` and `Tasca/Ringtones` on the card; the user copies WAV files there. The Settings screen gains a **Notification sound** row and a **Ringtone** row, each offering **Beep** (today's built-in sound) then the files, with a preview on every change, and a Volume row for every sound. Any error falls back to Beep.

## Project Context

- Today a live Message beeps once (2000 Hz, 80 ms) per Message; a ringing call double-beeps every 2 s.
- The Settings screen lists rows selected with `;` `.` and changed with `,` `/`; settings are kept across restarts.
- "Sound" On/Off silences every beep; "Wake on message" turns the screen on.
- The microSD card is not used today; the Cardputer has no card-detect switch.
- No PSRAM: about 330 KB free heap, so a WAV is read from the card in pieces while it plays.
- Concurrency rule: the SD card is touched only from the main loop.

## Functional Requirements

### Card and files

- REQ-1: At boot, with a microSD card present, the Cardputer creates `/Tasca/Notifications` and `/Tasca/Ringtones` when missing.
- REQ-2: Each folder's sound list is its valid WAV files: PCM 8 or 16-bit, mono or stereo, 8 to 48 kHz.
- REQ-3: The sound lists are read at boot and each time the Settings screen opens.

### Settings

- REQ-4: The Settings screen adds Notification sound and Ringtone rows; `,` and `/` step through Beep, then the files alphabetically.
- REQ-5: The Settings screen adds a Volume row, 20 % to 100 % in five steps, applied to every sound, Beep included.
- REQ-6: Changing Notification sound, Ringtone or Volume plays the selected sound for at most 5 s.
- REQ-7: Moving to another row or leaving the Settings screen stops the preview.
- REQ-8: The chosen Notification sound, Ringtone and Volume are kept across restarts.

### Playing

- REQ-9: Live Messages arriving together play the Notification sound once, for at most 5 s; Beep keeps one beep per Message.
- REQ-10: A ringing call plays the Ringtone in a loop; it stops within 200 ms of the call being answered, declined or ended.
- REQ-11: Sound Off silences the Notification sound and the Ringtone, as it silences the beeps today.

### Errors

- REQ-12: When a chosen file cannot be played (no card, missing, unreadable or invalid), Beep plays instead.
- REQ-13: When the sound lists are read and a chosen file is not among them, that choice returns to Beep and is saved.

### Documentation

- REQ-14: The README describes the two folders, the WAV formats accepted, and the three new Settings rows.

## Non-Functional Requirements

- **Performance** — Sound starts within 300 ms of the arrival, ring or change; playback never blocks the main loop for more than 20 ms, so keys stay responsive.
- **Memory** — Playback buffers use at most 16 KB of heap.
- **Flash** — The firmware grows by at most 48 KB (SD and file system support included).
- **Security** — File names may reach the serial log; Message content never does.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| No card at boot | No folders created; lists hold only Beep; nothing else changes | REQ-1, REQ-3, REQ-12 |
| Card read-only or full | Folders not created; `W` line; lists read from what exists | REQ-1 |
| Folder exists as a file | Treated as missing: list holds only Beep; `W` line | REQ-1, REQ-2 |
| Folder empty | Only Beep offered | REQ-2, REQ-4 |
| File with `.wav` name but other content (MP3, compressed WAV, 24-bit) | Not listed | REQ-2 |
| Files in subfolders, hidden files (`._ding.wav` from macOS) | Not listed | REQ-2 |
| More than 32 files in a folder | First 32 alphabetically listed | REQ-2, REQ-4 |
| Name longer than the row | Shown without `.wav`, cut with `…` to fit | REQ-4 |
| Name with accents or non-Latin characters | Shown as on other screens (□ for unsupported characters) | REQ-4 |
| Card removed and reinserted with Cardputer on | New files appear on the next Settings opening | REQ-3 |
| Card removed while a sound plays | Playback stops; Beep plays instead for that sound; `W` line | REQ-12 |
| Chosen file deleted on the computer | At boot or Settings opening the choice returns to Beep and is saved | REQ-13 |
| Key repeat on `,` `/` steps fast through files | Only the last selection previews; earlier previews stop | REQ-6 |
| Volume changed with Beep chosen | Beep plays at the new volume as preview | REQ-5, REQ-6 |
| Preview with Sound Off | Preview still plays: it is an explicit request | REQ-6, REQ-11 |
| Keyboard mode, a call or the Connection screen opens over Settings during a preview | Preview stops | REQ-7 |
| Sound turned Off while the Ringtone plays | Not possible: Settings cannot be opened during a call; Off applies to the next ring | REQ-10, REQ-11 |
| Message arrives during a preview | Preview stops; the Notification sound plays | REQ-7, REQ-9 |
| Message arrives while the Ringtone plays | No sound for it, as today during a call | REQ-9, REQ-10 |
| Message arrives while the Notification sound still plays | It does not restart | REQ-9 |
| WAV shorter than 5 s | Plays once, whole | REQ-9 |
| Ringtone file a few ms long | Loops; at most one restart per 200 ms | REQ-10 |
| Call answered from the iPhone | Ringtone stops within 200 ms | REQ-10 |
| Call ringing when the Connection screen opens (iPhone disconnects) | Ringtone stops with the Call screen | REQ-10 |
| Screen off when a Message or call arrives | Sound plays as today; screen follows Wake on message | REQ-9, REQ-10 |
| Settings saved with a file, then restart without card | Beep plays; at boot the choice returns to Beep and is saved | REQ-8, REQ-13 |
| Saved settings from before this feature | Notification sound and Ringtone default to Beep, Volume to 60 % | REQ-5, REQ-8 |
| N/A — concurrent: the SD card is read only from the main loop, one sound plays at a time | — | REQ-3, REQ-9, REQ-10 |
| N/A — permission: no user roles on the device | — | REQ-4, REQ-5 |
| N/A — network: the card is local; BLE is not involved | — | REQ-2, REQ-12 |
| N/A — integrity: the README is static text | — | REQ-14 |

## Out of Scope

- **Changing the built-in Beep** — the user chose to keep today's beep.
- **Formats other than PCM WAV (MP3, AAC)** — decoding costs flash and CPU; WAV converts easily on a computer.
- **A sound per contact or per chat** — not asked; one Notification sound for every Message.
- **Copying sounds without removing the card (USB mass storage, WiFi)** — the card is filled on a computer.
- **Sounds for other events (connection, pairing)** — not asked.
