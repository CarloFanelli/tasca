# Screens

What the Cardputer shows: status bar, Inbox, Chat screen, and the Keyboard mode context and Banner.

## Terms

### Inbox
The Reading mode list of **Chats**, newest first, two lines each (name with unread count, then a grey preview). Enter opens the **Chat screen** of the selected Chat.
_Avoid_: "chat list", "home" (one name for the list screen).

### Chat screen
The Reading mode screen of one Chat: its Messages oldest to newest, each with time (and sender in groups). Opening it marks its Messages read on the Cardputer only.
_Avoid_: "conversation view" (it is not a conversation: only received notifications appear).

### Settings screen
The Reading mode screen opened with Opt+s: brightness, sound, display timeout and wake on new message, kept in NVS, then the actions "Reset pairing" and "Clear messages", each run on a second Enter.
_Avoid_: "menu", "preferences".

### Call screen
The full-screen view of an iPhone call that is ringing, shown over any screen in either mode: caller's name, Enter answers, ` declines. While it is open no key reaches the iPhone.
_Avoid_: "call popup", "Banner" (a Banner is the 3-second box for a Message in Keyboard mode).

### Screen stack
The screens open on the Cardputer, one over another: the Inbox always at the bottom, then a Chat screen, the Settings screen or Keyboard mode opened over it. Only the top screen receives keys; closing it returns to the one below, unchanged.
_Avoid_: "navigation", "router" (one name for the stack).

### Banner
A 3-second box over the Keyboard mode context area showing a Message that arrived live while typing.
_Avoid_: "popup", "toast".

## Decisions

- **On-screen text stays in English**: status bar, labels and placeholders. Why: chosen by the user.
- **Emoji and non-Latin-1 characters show as □**: one □ per emoji sequence (joined, flagged, skin-toned). Why: the efont family has no emoji, and an emoji font does not fit the flash budget.
- **The Chat screen opens at the start of the newest Message**: earlier lines above it when it is short; a screen at its end follows a new Message from its start. Why: on hardware, opening at the last line forced the user to scroll up to read a long Message.
- **The key that wakes the screen depends on the mode**: in Reading mode it only turns the screen on; in Keyboard mode it also reaches the iPhone. Why: chosen by the user, so that typing never loses a key.
- **Banner only for Messages arriving live**: not for notifications resent at reconnection, nor for updated Messages. Why: the Banner announces a reply worth reading now, not old notifications.
- **Every screen is a Screen on the Screen stack**: one file per screen, and anything that opens over other screens (Keyboard mode, Call screen, Menu) is opened on the stack and closed back to the screen below. Why: "back where you were" stays one rule instead of flags in the main loop; agreed with the user before incoming calls and the music menu.
- **A covered Settings screen cancels its waiting prompt**: Keyboard mode opening over it cancels "Press Enter to confirm" at once. Why: same as any other key; the user accepted it in place of the 3 s expiry.

## Gotchas

- **Only notifications reach the Cardputer**: no chat history, no Messages the user sent, no muted chats, and Enter cannot reply. Replying means opening the chat on the iPhone and typing in Keyboard mode. A full WhatsApp view would need the optional WiFi bridge of phase 2.
- **No full-screen sprite**: a 240×135 16-bit buffer takes about 64 KB, more than the Message store budget with no PSRAM; each screen band clears and redraws its own rectangle on a dirty flag.
