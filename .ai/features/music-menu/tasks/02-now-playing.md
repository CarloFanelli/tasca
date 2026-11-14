# Task 02: now-playing

## Objective

The Music screen shows what the iPhone is playing in any app — app, title, artist, play state, elapsed and total time with a progress bar, volume — and follows changes within 1 s; the Menu shows the title next to Music.

## Context

**Satisfies**: REQ-5, REQ-6, REQ-7

Read-only first: the AMS data and its display, before any command is sent in Task 03.

## Depends on

Task 01

## Expected Behavior

- Once the link is secured, the Cardputer subscribes to the iPhone's now-playing data.
- The Music screen shows player app, title, artist, play state, elapsed/total time with a bar, and volume.
- Track, play-state and volume changes on the iPhone appear within 1 s; the bar advances each second from the last snapshot without asking the iPhone.
- The Menu's Music line shows the current title, or "Nothing playing".

## Acceptance Criteria

- [ ] The Music screen shows app, title, artist, play state, elapsed and total time with a bar, and volume (REQ-6)
- [ ] A track, play-state or volume change on the iPhone appears within 1 s (REQ-7)
- [ ] The bar advances once per second at the playback rate, with no request to the iPhone (REQ-6, REQ-7)
- [ ] The Menu shows the current title next to Music (REQ-5)
- [ ] Titles and artists never reach the serial log (REQ-6)

## Edge Cases

- Nothing playing, or no media app open → Menu: "Nothing playing"; Music screen: "Nothing playing" (REQ-5, REQ-6)
- iPhone not connected or AMS not available yet → Music screen: "iPhone not connected" (REQ-6)
- AMS appears or disappears at runtime (Service Changed) → Music screen follows: data shown when it appears, "iPhone not connected" when it goes (REQ-6, REQ-7)
- Title or artist longer than one line → title up to two lines then "…"; artist one line then "…" (REQ-6)
- Title truncated by iOS (flag set) → full value fetched once, up to 128 bytes (REQ-6)
- Spotify puts the artist into the title → shown as sent; no parsing (REQ-6)
- Unknown or zero duration (live radio) → elapsed time only, no bar (REQ-6)
- Playback rate other than 1 (paused, fast-forward) → bar advances at that rate; paused stops it (REQ-6, REQ-7)
- Track changes while the Music screen is not shown → Menu subtitle up to date; Music screen shows it when opened (REQ-5, REQ-7)
- Display timeout on the Music screen → screen turns off as on any screen; AMS keeps updating (REQ-7)
