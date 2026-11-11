# Task 01: build-and-flash

## Objective

With PlatformIO installed, one command builds the firmware and one command flashes it; the Cardputer then shows "Hello" and the firmware version.

## Context

**Satisfies**: REQ-1, REQ-2, REQ-3, REQ-4, REQ-5, REQ-6

Everything else in the project depends on a working build–flash–run loop on real hardware. This task also writes the first CLAUDE.md so Claude Code can drive build, flash and serial monitor from the CLI; Task 02 adds the test command to it.

## Depends on

None

## Expected Behavior

- `pio --version` succeeds on the development Mac.
- A single documented command builds the Cardputer firmware with zero errors from a clean checkout.
- A single documented command flashes a USB-connected Cardputer; the board then runs the new firmware without pressing reset.
- Within 3 s of power-on the display shows "Hello" and the firmware version, both fully visible on the 240x135 screen.
- The platform and every third-party library are pinned to exact versions.
- CLAUDE.md lists build, flash and serial-monitor commands, the serial port and how to override it, download-mode recovery, the concurrency rules of the Tasca spec §5.3 written out in full, and the ANCS specification link.
- After a build, `git status` shows no build output or tool cache.

## Acceptance Criteria

- [ ] `pio --version` prints a version on the development Mac (REQ-1)
- [ ] The documented build command finishes with zero errors from a clean checkout (REQ-2)
- [ ] The documented flash command uploads the firmware and the board runs it without manual reset (REQ-3)
- [ ] CLAUDE.md contains build, flash, monitor commands, serial port, concurrency rules and ANCS link (REQ-4)
- [ ] After a full build, `git status` lists no build or cache files (REQ-5)
- [ ] Within 3 s of power-on the screen shows "Hello" and the firmware version (REQ-6)
- [ ] Platform and all libraries are pinned to exact versions in the project configuration (REQ-2)

## Edge Cases

- First build with empty dependency cache → pinned dependencies are downloaded and the build succeeds (REQ-2 from the spec edge-case table)
- No internet on first build → build fails with PlatformIO's download error; CLAUDE.md notes the first build needs network (REQ-2 from the spec edge-case table)
- No Cardputer connected at flash time → flash fails with a "no serial port" error, nothing else changes (REQ-3 from the spec edge-case table)
- Board doesn't enumerate on USB → CLAUDE.md documents download mode: hold G0 while plugging USB, then flash (REQ-3 from the spec edge-case table)
- Serial port name differs from the documented one → CLAUDE.md shows how to list ports and override the port (REQ-4 from the spec edge-case table)
- N/A — concurrent/permission/network: CLAUDE.md is static documentation (REQ-4 from the spec edge-case table)
- Build creates new cache directories → none of them show up in `git status` (REQ-5 from the spec edge-case table)
- N/A — concurrent/permission/network: ignore rules are static (REQ-5 from the spec edge-case table)
- Long version string → "Hello" and version fit 240x135 without clipping (REQ-6 from the spec edge-case table)

## Notes from implementation

- **Known limitation**: AC 3 (flash and run without manual reset) and AC 6 ("Hello" and version within 3 s) are not yet verified on hardware: no Cardputer was connected during this run, by user decision. Build, pinning, git ignores and the no-device flash failure were verified. /dev, 2026-11-11.
- **REQ-6 / AC 6 — divergence**: the M5Stack examples force a 90° display rotation that is wrong with the pinned M5GFX 0.3.0, which already configures the Cardputer panel as 240x135 landscape; the firmware keeps the native rotation. Verified on hardware: AC 3 (flash with automatic reset) and AC 6 now pass, closing the known limitation above. /dev, 2026-11-11.
