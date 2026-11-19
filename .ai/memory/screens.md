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

### Menu
The Reading mode list opened with Fn+G0 from any screen and mode: Music, Find iPhone, Mesh, WhatsApp (the **Inbox**) and Settings. ` or Fn+G0 closes it back where the user was. From the **Connection screen** only Mesh and Settings open.
_Avoid_: "launcher", "home" ("home" is the Inbox's avoided name too).

### Music screen
The Reading mode screen of what the iPhone is playing in any app, read over AMS: app, title, artist, progress, volume, with keys for play/pause, previous, next and volume.
_Avoid_: "Spotify screen" (it works with any player), "now playing" as a screen name.

### Screen stack
The screens open on the Cardputer, one over another: the Inbox always at the bottom, then a Chat screen, the Settings screen or Keyboard mode opened over it. Only the top screen receives keys; closing it returns to the one below, unchanged.
_Avoid_: "navigation", "router" (one name for the stack).

### Connection screen
Full screen shown while no iPhone is connected: "Tasca" in large type, one line under it ("Connecting…", "Connect your iPhone" or "Forget Tasca on the iPhone") and the battery level. It opens at boot and on every disconnection, over the Screen stack, and closes when the link is secured; keys do nothing on it except the pairing reset.
_Avoid_: "startup screen", "splash" (it also returns after a disconnection).

### Find iPhone screen
The Reading mode screen opened from the **Menu** that guides to the iPhone by the link's signal strength: a bar, one word (Far, Near, Very close) and a Beep faster closer to it.
_Avoid_: "radar", "finder" (one name for the screen).

### Mesh screen
The Reading mode screen, opened from the **Menu** even without the iPhone, that lists the Meshtastic public channel's text messages and sends one from a compose line opened with Enter.
_Avoid_: "LoRa chat", "radio screen" (one name for the screen; LoRa is the radio, Mesh the network).

### Node list
The list of Meshtastic nodes heard, opened with Tab from the **Mesh screen**: names, signal strength, time since last heard.
_Avoid_: "contacts" (nodes are radios, not people saved by the user).

### Mesh name
The name the Cardputer announces to other Meshtastic nodes, set in the **Settings screen**; its first 4 characters that fit in 4 bytes are the short name (an accented letter takes 2).
_Avoid_: "nickname", "callsign".

### Out-of-range alert
The repeated triple **Beep** and the "iPhone out of range" line after the link to the iPhone is lost by signal. A link the iPhone closes (Bluetooth off, iPhone off) starts none.
_Avoid_: "disconnection alert" (not every disconnection starts it).

### Banner
A 3-second box over the Keyboard mode context area showing a Message that arrived live while typing.
_Avoid_: "popup", "toast".

### Beep
The built-in sound: one 2000 Hz, 80 ms beep per live Message, a double beep every 2 s for a ringing call. The default Notification sound and Ringtone, and the fallback for any sound error.
_Avoid_: "default ringtone", "tone" (one name for the built-in sound).

### Notification sound
The sound played for live Messages: Beep, or a WAV file from `Tasca/Notifications` on the microSD card, chosen in the Settings screen.
_Avoid_: "message tone", "alert".

### Ringtone
The sound looped while a call rings: Beep, or a WAV file from `Tasca/Ringtones` on the microSD card, chosen in the Settings screen.
_Avoid_: "ring sound", "suoneria" in artifacts.

## Decisions

- **On-screen text stays in English**: status bar, labels and placeholders. Why: chosen by the user.
- **Emoji and non-Latin-1 characters show as □**: one □ per emoji sequence (joined, flagged, skin-toned). Why: the efont family has no emoji, and an emoji font does not fit the flash budget.
- **The Chat screen opens at the start of the newest Message**: earlier lines above it when it is short; a screen at its end follows a new Message from its start. Why: on hardware, opening at the last line forced the user to scroll up to read a long Message.
- **The key that wakes the screen depends on the mode**: in Reading mode it only turns the screen on; in Keyboard mode it also reaches the iPhone. Why: chosen by the user, so that typing never loses a key.
- **Banner only for Messages arriving live**: not for notifications resent at reconnection, nor for updated Messages. Why: the Banner announces a reply worth reading now, not old notifications.
- **Every screen is a Screen on the Screen stack**: one file per screen, and anything that opens over other screens (Keyboard mode, Call screen, Menu) is opened on the stack and closed back to the screen below. Why: "back where you were" stays one rule instead of flags in the main loop; agreed with the user before incoming calls and the music menu.
- **A covered Settings screen cancels its waiting prompt**: Keyboard mode opening over it cancels "Press Enter to confirm" at once. Why: same as any other key; the user accepted it in place of the 3 s expiry.
- **The Connection screen covers everything while the iPhone is away**: at boot and on every disconnection, with no keys but the pairing reset; Messages cannot be read until it reconnects. Why: chosen by the user over keeping the Inbox readable.
- **Large text is efontCN_16 scaled, thickened by redrawing it one font pixel apart**: "Tasca" is 5× with 2-pixel strokes. Why: a FreeSansBold 18 pt font cost about 9 KB of flash; the user compared sizes and strokes on the device.
- **The Music screen shows only what AMS sends**: no playlists, library or lyrics. Why: AMS carries only the current track, player and queue position; the Wi-Fi alternatives (Spotify Web API, LRCLIB lyrics over the iPhone's hotspot) were offered and the user kept it as is (2026-11-14).

## Gotchas

- **Only notifications reach the Cardputer**: no chat history, no Messages the user sent, no muted chats, and Enter cannot reply. Replying means opening the chat on the iPhone and typing in Keyboard mode. A full WhatsApp view would need the optional WiFi bridge of phase 2.
- **No full-screen sprite**: a 240×135 16-bit buffer takes about 64 KB, more than the Message store budget with no PSRAM; each screen band clears and redraws its own rectangle on a dirty flag.
- **Thickening a scaled bitmap font by 3 font pixels merges the letters**: the small counters of efontCN_16 (a, s) fill in at any scale; 2 font pixels is the limit.
- **Previous and next skip 15 s on podcasts**: the player offers both commands, so the keys are not dimmed, and iOS turns them into 15-second skips. Seen on the user's iPhone; not a Cardputer bug.
