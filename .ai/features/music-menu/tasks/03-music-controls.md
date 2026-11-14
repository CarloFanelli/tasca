# Task 03: music-controls

## Objective

On the Music screen, Enter plays or pauses, `,` and `/` go to the previous and next track, `;` and `.` change the volume; keys the player does not offer are dimmed.

## Context

**Satisfies**: REQ-8, REQ-9, REQ-11

The commands, once the screen shows what they act on.

## Depends on

Task 02

## Expected Behavior

- Enter toggles play/pause, `,` previous, `/` next, `;` volume up, `.` volume down, each reaching the player within 1 s.
- One command per press; volume repeats every 200 ms while held.
- The commands the player offers now come from the iPhone; the others' key hints are dimmed and do nothing.

## Acceptance Criteria

- [ ] Enter, `,`, `/`, `;`, `.` send play/pause, previous, next, volume up, volume down (REQ-8)
- [ ] Each command reaches the iPhone's player within 1 s of the key (REQ-9)
- [ ] Volume repeats every 200 ms while `;` or `.` is held; other keys send once per press (REQ-8)
- [ ] Keys whose command the player does not offer are dimmed and send nothing (REQ-11)

## Edge Cases

- Nothing playing → Enter sends Play and iOS decides (REQ-8)
- iPhone not connected or AMS not available yet → keys do nothing; `W` line on a key (REQ-8)
- Key held or repeated → one command per press; volume repeats every 200 ms while held (REQ-8)
- Command refused by iOS (error response) → `W` line; screen unchanged (REQ-9)
- Player offers no previous/next (e.g. some podcasts) → those keys dimmed (REQ-11)
