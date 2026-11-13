# Task 03: iphone-verification

## Objective
Verify on the user's iPhone that phone, FaceTime and WhatsApp calls show on the Call screen and can be answered and declined, and record the outcome in the README.

## Context
**Satisfies**: REQ-9
Whether FaceTime and WhatsApp calls arrive as incoming-call notifications with actions is known only on a real iPhone; the README states what works.

## Depends on
02-answer-decline

## Expected Behavior
- A phone call, a FaceTime call and a WhatsApp call are each placed to the user's iPhone with Tasca connected.
- For each: whether the Call screen opens, whether Enter answers and ` declines.
- The README gains a Calls section: keys, what works per kind of call, and that audio stays on the iPhone or earphones.

## Acceptance Criteria
- [ ] Phone call: Call screen, answer and decline recorded as working or not (REQ-9)
- [ ] FaceTime call: same three outcomes recorded (REQ-9)
- [ ] WhatsApp call: same three outcomes recorded (REQ-9)
- [ ] README lists the Call screen keys and which calls work (REQ-9)

## Edge Cases
- FaceTime or WhatsApp calls not in the incoming-call category → not shown; the README says which calls work (REQ-9)
