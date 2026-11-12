# Task 05: whatsapp-category-filter

## Objective
The Cardputer requests attributes only for notifications in WhatsApp's category, so a reconnection imports in a few seconds.

## Context
**Satisfies**: REQ-13
At every reconnection ~120 notifications of other apps are fetched one at a time (~11 s). Each notification event carries its category; WhatsApp's must be observed on hardware before filtering.

## Depends on
None

## Expected Behavior
- The category of every notification event is read; other categories are counted and skipped without a request.
- Before the filter is enabled, the category WhatsApp uses is confirmed on serial.

## Acceptance Criteria
- [ ] WhatsApp's notification category is confirmed on hardware and noted in the task (REQ-13)
- [ ] Notifications of other categories get no attribute request and are counted (REQ-13)
- [ ] A reconnection with ~120 other-app notifications finishes its import in under 5 s (REQ-13)

## Edge Cases
- Notification event in another category → counted; no attribute request (REQ-13)
- WhatsApp notification in an unexpected category → not stored: confirm the category on hardware before enabling the filter (REQ-13)
- Removed event for a skipped notification → ignored (REQ-13)

## Notes from implementation

- **WhatsApp category**: on the user's iPhone every WhatsApp notification arrived with CategoryID 4 (Social); the filter accepts Social only.
- **REQ-13 / AC 3 — divergence**: the import at reconnection measured 5.6 s, down from about 11 s, against the 5 s target. The rest is spent on notifications of other messaging apps, which iOS also files as Social (53 of 137 in the measurement), so the time depends on how many of them are on the iPhone. Resolution: the user accepted 5.6 s on 2026-11-12.
