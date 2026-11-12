# Task 01: attribute-decoding

## Objective

The Cardputer decodes every attribute response from the iPhone and keeps only WhatsApp notifications; decoded notifications replace the raw hex dumps on serial.

## Context

**Satisfies**: REQ-1, REQ-2, REQ-12

Everything else in the milestone consumes decoded attributes, so decoding comes first. Raw dumps go away here: from now on serial shows readable WhatsApp notifications (Debug level) and a count for other apps.

## Depends on

None

## Expected Behavior

- Each attribute response yields app, title, subtitle, message and date, whether it arrived in one Data Source packet or several.
- Only notifications from `net.whatsapp.WhatsApp` go further; other apps are counted, not logged with content.
- At Debug level each decoded WhatsApp notification is logged with title, subtitle and text; no hex dump is logged any more.
- Text cut by the iPhone in the middle of a character keeps only whole UTF-8 characters.

## Acceptance Criteria

- [x] A WhatsApp message received on the iPhone appears on serial with readable title and text, accents and emoji intact (REQ-1)
- [x] Notifications of other apps produce no content on serial, only a count (REQ-2)
- [x] No Notification Source or Data Source hex dump appears on serial (REQ-12)
- [x] Host tests cover responses split over several packets, empty attributes, malformed lengths and UTF-8 cut mid-character (REQ-1)

## Edge Cases

- Response split over 2 or more Data Source packets → decoded once complete, same result as a single packet (REQ-1 from the spec edge-case table)
- Attribute with length 0 (e.g. empty subtitle or message) → decoded as empty (REQ-1 from the spec edge-case table)
- Response longer than declared or malformed (length past the end) → discarded with a `W` line; next request proceeds (REQ-1 from the spec edge-case table)
- No response within 2 s → request abandoned with a `W` line, next one sent; nothing kept (REQ-1 from the spec edge-case table)
- Disconnection mid-response → partial response discarded; nothing kept (REQ-1 from the spec edge-case table)
- Notification from another app → not kept; counted only (REQ-2 from the spec edge-case table)
- WhatsApp Business or other app ids → not kept (not in the list) (REQ-2 from the spec edge-case table)
- Text truncated by the iPhone mid-character → partial character dropped; rest kept (REQ-4 from the spec edge-case table)
- Emoji in title, subtitle or text → kept and logged as received (REQ-4 from the spec edge-case table)

## Notes from implementation

- **Verified on hardware**: at connection 66 notifications were decoded with no malformed response and no hex dump; all were from other apps and only counted. Two live WhatsApp notifications were decoded: a group message (title, subtitle, emoji) and a Meta AI reply whose title carries the leading invisible mark that task 02 trims. Accented letters are covered by host tests; the live texts happened to contain none.
- **Known limitation**: the WhatsApp notifications already on the iPhone were among the requests skipped by the 64-request cap. Task 04 removes the cap.
