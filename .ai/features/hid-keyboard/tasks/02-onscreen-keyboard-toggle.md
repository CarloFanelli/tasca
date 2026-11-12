# Task 02: onscreen-keyboard-toggle

## Objective

In Keyboard mode, Fn+Space shows or hides the iPhone's on-screen keyboard.

## Context

**Satisfies**: REQ-8

iOS hides its on-screen keyboard while a hardware keyboard is connected; the consumer Eject usage is the documented way to bring it back. Kept apart because iOS may ignore it: the outcome is a finding for the project risk list either way.

## Depends on

- Task 01

## Expected Behavior

- In Keyboard mode, each Fn+Space press sends the consumer Eject usage once, then its release.
- In Reading mode Fn+Space sends nothing.

## Acceptance Criteria

- [x] Fn+Space in Keyboard mode shows the iPhone on-screen keyboard; pressing it again hides it (REQ-8)
- [x] Holding Fn+Space toggles once per press (REQ-8)
- [x] Host tests cover Fn+Space producing the Eject usage and nothing else (REQ-8)

## Edge Cases

- iOS ignores the Eject usage → nothing happens on the iPhone; noted for the on-screen keyboard fallback (REQ-8 from the spec edge-case table)
- Fn+Space held → one Eject toggle per press (REQ-8 from the spec edge-case table)
