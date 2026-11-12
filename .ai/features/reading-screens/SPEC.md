---
feature: reading-screens
version: 1.2
status: approved
completed: true
base: d99f677ec0743e4ca679933a42254c2245a28b44
---

# Reading Screens (M4)

## Overview

Gives the stored **Messages** a real interface. In **Reading mode** the Cardputer shows the
**Inbox**, a list of **Chats** newest first with unread counts and a preview. Enter opens the
**Chat screen**, which shows the Chat's Messages and marks them read on the Cardputer. A status
bar is always visible. G0 still switches to **Keyboard mode**. Entered from a Chat screen, Keyboard
mode shows that Chat's last Message above the echo line. A Message arriving while typing shows a
3-second **Banner**. The Cardputer mirrors the iPhone: a Message read or cleared there disappears. This closes the milestone loop: receive, read on the Cardputer, open the chat
on the iPhone, reply from the Cardputer keyboard. All on-screen text is in English.

## Project Context

- Builds on message-store: Messages in RAM (max 200), grouped by Chat, unread state, counts.
- Keyboard mode, its readiness notice and the echo line exist (hid-keyboard) and keep their behavior.
- Display is 240 × 135; the efont family covers Latin-1 accents but not emoji.
- Keycaps of `;` `,` `.` `/` carry arrow legends; `` ` `` carries Esc.
- Only notifications reach the Cardputer: no chat history, no messages sent by the user.
- Concurrency rules in CLAUDE.md apply: draw from the main loop only, redraw on change only.
- Fn+Del (3 s) pairing reset stays available in Reading mode.

## Functional Requirements

### Status bar

- REQ-1: A status bar is always visible: link state (advertising, connected, ANCS active), mode, total unread, battery percent.

### Inbox

- REQ-2: In Reading mode the Inbox lists every Chat, the one with the newest Message first.
- REQ-3: Each Chat takes two lines: name and unread count, then the last Message in grey, prefixed by its sender in groups.
- REQ-4: `;` and `.` move the selection up and down, scrolling the list; Enter opens the selected Chat screen.

### Chat screen

- REQ-5: The Chat screen shows the Chat's Messages oldest to newest, word-wrapped, each with its time; in groups each starts with its sender.
- REQ-6: The Chat screen opens showing the start of its newest Message; `;` and `.` scroll by one line; `` ` `` returns to the Inbox.
- REQ-7: Opening a Chat screen marks its Messages read on the Cardputer; Messages arriving while it is open are read too.

### Text and live updates

- REQ-8: Accented letters display correctly; each emoji displays as a small square (□).
- REQ-9: A new or updated Message refreshes the Inbox or open Chat screen within 1 s, keeping the selected Chat selected.

### Keyboard mode

- REQ-10: G0 from the Inbox or a Chat screen enters Keyboard mode; G0 again returns to that same screen.
- REQ-11: Keyboard mode entered from a Chat screen shows that Chat's last Message above the echo line.
- REQ-12: A Message arriving in Keyboard mode shows a 3-second Banner with Chat and text; typing is unaffected.

### Mirror of the iPhone

- REQ-13: A Message removed on the iPhone (read or cleared there) is deleted from the Cardputer; a Chat left empty leaves the Inbox.
- REQ-14: After the notifications resent at connection are all answered, Messages the iPhone no longer has are deleted.

## Data & Dependencies

### Information the feature must retain

- Current screen and selection — which screen is shown, the selected Chat and the scroll position; RAM only, reset at restart to the Inbox, top.

## Non-Functional Requirements

- **Performance** — A navigation key redraws the screen within 100 ms.
- **Performance** — The screen is redrawn only when its content changes, never on every loop.
- **Validation** — Text never overflows the screen: lines too long for one line are cut with "…" (names) or wrapped (Message text).

## Edge Cases & Error Handling

| Edge Case | Expected Behavior | Relates to |
|-----------|-------------------|------------|
| Battery level unknown | Status bar shows "--%" | REQ-1 |
| Link not ready (advertising, connecting, pairing failed) | Status bar shows the state; Inbox still shows stored Messages | REQ-1, REQ-2 |
| ANCS denied | Status bar shows "ANCS denied" | REQ-1 |
| No Messages yet | Inbox shows "No messages yet" | REQ-2 |
| Total unread above 99 | Shown as "99+" | REQ-1, REQ-3 |
| Chat name too long for the line | Cut with "…" before the unread count | REQ-3 |
| Chat with no unread Messages | No count shown | REQ-3 |
| Selection on the first or last Chat and key pressed further | Selection stays; no wrap-around | REQ-4 |
| Fn held with `;` or `.` in Reading mode | Same as without Fn | REQ-4, REQ-6 |
| Other keys in Reading mode (letters, Enter in a Chat screen) | Ignored; nothing sent to the iPhone | REQ-4, REQ-6 |
| Chat screen already at its top or bottom | Further scroll ignored | REQ-6 |
| Message longer than the screen | Wrapped; reachable line by line with scrolling | REQ-5, REQ-6 |
| Chat screen open while its oldest Messages are dropped by the store limit | Dropped Messages disappear; scroll position clamped | REQ-5, REQ-9 |
| Every Message of the open Chat dropped | Back to the Inbox | REQ-9 |
| Message of the open Chat updated by the iPhone | Its text is replaced in place | REQ-9 |
| Message arrives in another Chat while a Chat screen is open | Open Chat unchanged; status bar unread count goes up | REQ-7, REQ-9 |
| New Message moves the selected Chat to the top | Selection follows that Chat | REQ-9 |
| Removed event for a Message already read on the Cardputer | Deleted like any other | REQ-7, REQ-13 |
| Message read on the Cardputer, still on the iPhone | Stays, shown as read, until removed on the iPhone | REQ-7, REQ-13 |
| Removed event for a notification never stored (another app, failed request) | Ignored | REQ-13 |
| Open Chat screen loses its last Message to a removal | Back to the Inbox | REQ-9, REQ-13 |
| Message read on the iPhone while the Cardputer was disconnected | Deleted once the reconnection import ends | REQ-14 |
| An answer lost during the reconnection import (timeout, malformed, dropped packet, queue full) | Nothing deleted for that connection; `W` line | REQ-14 |
| Disconnection before the import ends | Nothing deleted | REQ-14 |
| New Message arriving during the import | Kept: it belongs to this connection | REQ-14 |
| Emoji in a Chat name | Shown as □ in Inbox, Chat screen and Banner | REQ-8 |
| Character outside Latin-1 that is not an emoji (e.g. Cyrillic) | Shown as □ | REQ-8 |
| Keyboard mode entered from the Inbox | Area above the echo line left empty | REQ-11 |
| Message arrives in the Chat shown above the echo line | Banner shown and the line updates to the new Message | REQ-11, REQ-12 |
| Several Messages arrive within 3 s in Keyboard mode | Newest replaces the Banner; 3 s restart | REQ-12 |
| Banner shown while the iPhone is not ready | Banner shown; keys still discarded as today | REQ-12 |
| G0 pressed while a Banner is shown | Banner removed; Reading screen shown | REQ-10, REQ-12 |
| Message arrives in Reading mode | No Banner; Inbox or Chat screen updates | REQ-9, REQ-12 |
| Display failed to initialize | Nothing drawn; keys still sent in Keyboard mode | REQ-1 |
| N/A — concurrent: one user, one main loop draws | — | REQ-4, REQ-10 |
| N/A — permission and network: no access control; link loss covered by status bar rows | — | REQ-5, REQ-12 |

## Out of Scope

- **Settings screen, beep, display timeout, power management** — milestone M5.
- **Marking a message read on the iPhone from the Cardputer** — iOS offers no way to do it over ANCS.
- **Chat history, messages sent by the user, muted chats** — iOS sends only notifications.
- **Showing emoji as images** — needs an emoji font too large for the flash budget of this milestone.
- **Deleting messages from the Cardputer** — part of "clear data" in M5/M6.
- **Italian on-screen text** — decided: the interface stays in English.
- **History of read Messages on the Cardputer** — dropped: the user wants the Cardputer to mirror the iPhone's notifications, since replies are never visible anyway.
