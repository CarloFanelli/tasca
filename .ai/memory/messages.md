# Messages

WhatsApp notifications decoded from ANCS and kept on the Cardputer, grouped by chat.

## Terms

### Message
One WhatsApp notification kept by the Cardputer: **Chat**, sender, text, date and read state, in RAM only. Created from the notification's attributes, updated by a modified event, deleted by a removed event.
_Avoid_: "notification" for the stored item (a notification is what the iPhone sends; most are not WhatsApp and are not kept).

### Chat
The conversation a **Message** belongs to: the notification subtitle (group name) when present, otherwise the title (contact name). The sender is always the title.
_Avoid_: "conversation", "thread" (one name for the grouping key).

## Decisions

- **Store limit is 200 Messages or 56 KB of memory**: the 56 KB covers each Message's own struct as well as its strings; the oldest Messages are dropped first. Why: there is no PSRAM, 200 messages of up to 512 bytes could take about 130 KB, and the user chose a predictable budget over always keeping 200.
- **The Cardputer mirrors the iPhone's notifications**: a Message removed on the iPhone (read or cleared) is deleted, and after the reconnection import Messages not resent are deleted too; a Message read on the Cardputer stays, as read, until then. Why: the user's replies never reach the Cardputer, so keeping read Messages is no real chat history.
- **Attributes requested only for the Social category**: on the user's iPhone every WhatsApp notification arrives as Social (CategoryID 4); other categories are counted and skipped. Why: it halved the import at reconnection (about 11 s to 5.6 s); the user accepted 5.6 s, since other messaging apps are Social too and the filter cannot skip them.
- **Message content on serial only at Debug**: Chat, sender and text are logged at `D`; above Debug only counts. Why: the log is needed to verify decoding, and a higher-level build keeps content off serial.

## Gotchas

- **Invisible direction marks in names**: some WhatsApp titles (e.g. "Meta AI") start with U+200E. They are trimmed at both ends before a name is used as Chat or sender; marks inside a name are kept.
- **Test data never comes from captures**: serial captures hold the user's real messages. Fixtures are built by hand with invented names, as in `test/test_ancs_request`.
- **A wrong category filter empties the store**: if WhatsApp notifications arrived in a category outside the filter, none would be requested and the reconnection cleanup would delete every stored Message as gone from the iPhone. Confirm the category on hardware before changing the filter.
- **A message read while disconnected sends no removal**: iOS does not resend a notification that is already gone. It is detected only by absence once the reconnection import ends, and only when every answer of that import arrived.
