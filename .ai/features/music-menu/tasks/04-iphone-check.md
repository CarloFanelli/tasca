# Task 04: iphone-check

## Objective

The Menu and the Music screen are checked on the user's iPhone with Spotify and Apple Music: data, timings and commands.

## Context

**Satisfies**: REQ-7, REQ-9

The two timing requirements can only be measured on the device; the check also covers the AMS caveats (Spotify's title field, podcasts without next).

## Depends on

Task 03

## Expected Behavior

- With Spotify and Apple Music playing, the Music screen shows the right data and follows changes made on the iPhone within 1 s.
- Every key's command reaches the player within 1 s.
- The checklist and its outcome are recorded in this task's notes.
- The README describes the Menu (Fn+G0) and the Music screen keys.

## Acceptance Criteria

- [x] Track, play-state and volume changes made on the iPhone appear within 1 s, with Spotify and Apple Music (REQ-7) — Spotify only, see notes
- [x] Play/pause, previous, next and volume reach the player within 1 s (REQ-9)
- [x] The checklist and its outcome are recorded in this task's notes (REQ-7, REQ-9)
- [x] The README lists Fn+G0 and the Music screen keys (REQ-1, REQ-8)

## Edge Cases

- Spotify puts the artist into the title → recorded as seen; no parsing (REQ-6)
- Player offers no previous/next (e.g. some podcasts) → checked that the keys are dimmed (REQ-11)

## Notes from implementation

- **Spotify**: Menu (Fn+G0), Music screen data and every key work on the user's iPhone; changes on the iPhone and commands from the Cardputer within 1 s (tested 2026-11-14).
- **Podcasts**: previous and next are offered by the player, so their keys are not dimmed; on a podcast they skip back and forward 15 s (iOS maps the track commands to skips). Accepted as seen.
- **REQ-7 — divergence**: Apple Music not tested, the user does not have it.
