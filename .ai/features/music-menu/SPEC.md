---
feature: music-menu
version: 1.0
status: approved
completed: false
base: 400cb1e089af2a71fe909db5e4829ced1c88588a
---

# Music and Menu

## Overview

Adds the **Menu**, opened with Fn+G0 from any screen in either mode, listing Music, WhatsApp and Settings. Music opens the **Music screen**: what the iPhone is playing in any app (Spotify, Apple Music, podcasts) — app, title, artist, progress, play state, volume — with keys for play/pause, previous, next and volume. The data and commands go through Apple Media Service (AMS) on the existing bonded link: no app on the iPhone, no new pairing. Controls work only on the Music screen. Browsing playlists or the library is out of scope.

## Project Context

- G0 alone switches Reading mode and Keyboard mode; Opt+s opens the **Settings screen**; `` ` `` goes back to the **Inbox**.
- In Keyboard mode every key goes to the iPhone; Fn+letter types accents, Fn+Enter starts Siri.
- AMS is an iOS GATT service read by the Cardputer as a client, like ANCS: no GATT change on the Cardputer side.
- AMS sends track, player and queue changes as they happen; the elapsed time is a snapshot with a playback rate.
- On-screen text is English; emoji and non-Latin-1 characters show as □.
- Redraw only on state change; the main loop is the only code that draws and issues GATT writes.

## Functional Requirements

### Menu

- REQ-1: Fn+G0 opens the Menu over any screen, in either mode, listing Music, WhatsApp and Settings.
- REQ-2: In the Menu, `;` and `.` move the selection; Enter opens the selected item in Reading mode.
- REQ-3: `` ` `` or Fn+G0 closes the Menu, back to the previous screen and mode.
- REQ-4: Music opens the Music screen, WhatsApp opens the Inbox, Settings opens the Settings screen.
- REQ-5: The Menu shows the current title next to Music and the unread count next to WhatsApp.

### Music screen

- REQ-6: The Music screen shows player app, title, artist, play state, elapsed and total time with a progress bar, and volume.
- REQ-7: A track, play-state or volume change on the iPhone appears on the Music screen within 1 s.
- REQ-8: Enter toggles play/pause, `,` previous track, `/` next track, `;` volume up, `.` volume down.
- REQ-9: Each command reaches the iPhone's player within 1 s of the key.
- REQ-10: `` ` `` on the Music screen goes back to the Inbox.
- REQ-11: Keys whose command the current player does not offer are shown dimmed and do nothing.

## External capabilities relied on

- Apple Media Service (AMS) on iOS — now-playing data and remote commands for the active player app; available on a bonded, encrypted link.

## Non-Functional Requirements

- **Performance** — Updates within 1 s (REQ-7), commands within 1 s (REQ-9); the progress bar advances once per second from the last snapshot, with no requests to the iPhone.
- **Battery** — No AMS traffic beyond iOS change notifications; the once-per-second redraw only while the Music screen is shown and the screen is on.
- **Privacy** — Titles and artists are never written to the serial log.

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| Fn+G0 with the screen off | Only wakes the screen, like G0 today | REQ-1 |
| Fn+G0 in Keyboard mode | Menu opens, no key is sent to the iPhone; opening an item switches to Reading mode | REQ-1, REQ-2 |
| Fn+G0 while a Settings action waits for confirmation | Prompt cancelled, Menu opens | REQ-1 |
| G0 alone while the Menu is open | Closes the Menu and switches mode as today | REQ-3 |
| Selection at the first or last item | Stays there; no wrap | REQ-2 |
| Item opened is the current screen (e.g. Settings from Settings) | That screen shown from its start | REQ-4 |
| Nothing playing, or no media app open | Menu: "Nothing playing"; Music screen: "Nothing playing", Enter sends Play and iOS decides | REQ-5, REQ-6, REQ-8 |
| iPhone not connected or AMS not available yet | Music screen: "iPhone not connected"; keys do nothing; `W` line on a key | REQ-6, REQ-8 |
| AMS appears or disappears at runtime (Service Changed) | Music screen follows: data shown when it appears, "iPhone not connected" when it goes | REQ-6, REQ-7 |
| Title or artist longer than one line | Title up to two lines then "…"; artist one line then "…" | REQ-6 |
| Title truncated by iOS (flag set) | Full value fetched once, up to 128 bytes | REQ-6 |
| Spotify puts the artist into the title | Shown as sent; no parsing | REQ-6 |
| Unknown or zero duration (live radio) | Elapsed time only, no bar | REQ-6 |
| Playback rate other than 1 (paused, fast-forward) | Bar advances at that rate; paused stops it | REQ-6, REQ-7 |
| Key held or repeated | One command per press; volume repeats every 200 ms while held | REQ-8 |
| Command refused by iOS (error response) | `W` line; screen unchanged | REQ-9 |
| Track changes while the Music screen is not shown | Menu subtitle up to date; Music screen shows it when opened | REQ-5, REQ-7 |
| Display timeout on the Music screen | Screen turns off as on any screen; AMS keeps updating | REQ-7 |
| Message arrives on the Music screen | Beep and wake as today; screen stays on Music | REQ-6 |
| Player offers no previous/next (e.g. some podcasts) | Those keys dimmed | REQ-11 |
| N/A — concurrent: one user; the main loop is the only writer to the screen and AMS | — | REQ-8, REQ-9 |
| N/A — permission: AMS needs the bonded iPhone, as ANCS | — | REQ-6, REQ-8 |
| N/A — integrity: nothing is stored; music data lives in RAM only | — | REQ-6 |

## Out of Scope

- **Browsing playlists, liked songs or the library** — AMS exposes only the current track and queue position; other routes need Shortcuts or the Spotify Web API over Wi-Fi.
- **Music controls outside the Music screen** — keys keep their meaning elsewhere, and in Keyboard mode they belong to the iPhone.
- **Like, shuffle, repeat, skip ±15 s** — not asked for; easy to add later with the same channel.
- **Album art** — AMS sends no images.
