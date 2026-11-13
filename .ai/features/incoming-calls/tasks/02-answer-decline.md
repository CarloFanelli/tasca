# Task 02: answer-decline

## Objective
On the **Call screen**, Enter answers the call and ` declines it on the iPhone.

## Context
**Satisfies**: REQ-2, REQ-3, REQ-4
Builds on the Call screen of Task 01: its keys now perform the notification's positive or negative action. REQ-2 is shared: Task 01 blocks every key, this task gives Enter and ` their meaning.

## Depends on
01-call-screen

## Expected Behavior
- The Call screen shows the keys it accepts: "Enter answer" and "` decline".
- Enter asks the iPhone to answer the shown call, ` to decline it, within 1 s of the key.
- After one action, further presses do nothing until the call closes.
- If the iPhone refuses the action, the Call screen shows "Failed — use the iPhone" until the call closes.
- The serial log records the action taken, never the caller's name.

## Acceptance Criteria
- [ ] Enter on the Call screen performs the positive action of the shown call (REQ-3)
- [ ] ` on the Call screen performs the negative action of the shown call (REQ-4)
- [ ] Any other key, and G0, still does nothing on the Call screen (REQ-2)
- [ ] One action per press; nothing more is sent until the Call screen closes (REQ-3, REQ-4)
- [ ] A refused action logs a `W` line and shows "Failed — use the iPhone" (REQ-3, REQ-4)

## Edge Cases
- Second call while one rings (call waiting) → Enter and ` act on the newest call, the one shown (REQ-3, REQ-4)
- Enter or ` held or repeated → one action per press; nothing more while waiting for the call to close (REQ-3, REQ-4)
- Notification without a positive action → Enter does nothing; hint shows only "` decline" (REQ-3)
- Notification without a negative action → ` does nothing; hint shows only "Enter answer" (REQ-4)
- The iPhone refuses the action (error from Control Point) → `W` line; Call screen stays with "Failed — use the iPhone" until the call closes (REQ-3, REQ-4)
- N/A — permission: every action needs the bonded iPhone, as for every ANCS write (REQ-3, REQ-4)
