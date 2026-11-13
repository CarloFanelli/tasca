# Tasca

Firmware for the M5Stack Cardputer v1.1 that shows the WhatsApp notifications of a paired iPhone and types replies into it as a Bluetooth keyboard. It needs no app on the iPhone: notifications arrive over Apple's Notification Center Service (ANCS), and replies are keys sent over Bluetooth HID.

Only what reaches the iPhone as a notification is visible: no chat history, no messages you sent, nothing from muted chats.

## Privacy

Messages on the Cardputer are readable by anyone holding it: there is no screen lock or PIN. They are kept in RAM only: after a restart the Cardputer shows only the notifications still on the iPhone. A Message read or cleared on the iPhone disappears from the Cardputer too.

## Build and flash

With PlatformIO (`brew install platformio`), from the repository root:

```sh
pio run -e cardputer -t upload
```

Host tests: `pio test -e native`. More commands and troubleshooting in `CLAUDE.md`.

## iPhone setup

Do these steps in order. Menu names may differ slightly between iOS versions.

1. **Pair.** Settings > Bluetooth > select "Tasca". Pairing needs no PIN. A board that ran other firmware may show up under its old name (e.g. "ESP32") until the first connection.
2. **Share notifications.** Accept the request to share notifications. If it does not appear: Settings > Bluetooth > (i) next to "Tasca" > turn on "Share System Notifications", then restart the Cardputer.
3. **WhatsApp previews.** Settings > Notifications > WhatsApp: notifications allowed, previews "Always". In WhatsApp > Settings > Notifications: "Show Preview" on. Without previews the text does not arrive.
4. **Keyboard layout.** With "Tasca" connected: Settings > General > Keyboard > Hardware Keyboard, layout "Automatic" (or "U.S."). With another layout, keys type that layout's characters.
5. **Focus.** Chats filtered by a Focus or muted send no notifications: allow WhatsApp in the Focus modes you use.

Only one iPhone can be paired. To pair again from scratch, do both: "Forget This Device" on the iPhone, and "Reset pairing" on the Cardputer (Settings, or Fn+Del held for 3 s).

## Using the Cardputer

**Connection screen** — while the iPhone is not connected, at startup and whenever it disconnects, the Cardputer shows "Tasca" in large type, the battery, and one line:

| Line | Meaning |
| --- | --- |
| Connecting… | An iPhone is paired: the Cardputer waits for it to come back in range with Bluetooth on. |
| Connect your iPhone | No iPhone is paired: pair one (see iPhone setup). |
| Forget Tasca on the iPhone | Pairing failed because the iPhone remembers an old pairing: forget "Tasca" there, then pair again. |

Keys and G0 do nothing on this screen, except Fn+Del held 3 s. When the iPhone connects, the screen you were on comes back. If "Connecting…" never ends after you forgot "Tasca" on the iPhone, hold Fn+Del for 3 s and pair again.

G0 (the button on the side) switches between Reading mode and Keyboard mode. The status bar at the top shows the mode, the link state, the unread count and the battery.

**Reading mode** — keys stay on the Cardputer.

| Key | Action |
| --- | --- |
| `;` / `.` | Move up / down in the Inbox, scroll a Chat |
| Enter | Open the selected Chat |
| `` ` `` | Back to the Inbox, or out of Settings |
| Opt+s | Open Settings |
| Fn+Enter | Start Siri (see below) |
| Fn+Del, held 3 s | Reset pairing and restart |

**Settings** — `;` and `.` select a row, `,` and `/` change its value: brightness, sound, display timeout (15 s to 5 min, or Never) and wake on new message. "Reset pairing" and "Clear messages" run on Enter, then Enter again to confirm. Settings are kept across restarts.

**Keyboard mode** — every key goes to the iPhone, typing its keycap legend. Fn+a/e/i/o/u give à è ì ò ù, Ctrl+e gives é. To reply, open the chat on the iPhone, then type on the Cardputer.

The screen turns off after the display timeout; a key or G0 turns it back on. In Reading mode that key does nothing else; in Keyboard mode it is also sent to the iPhone. Each Message arriving live beeps when sound is on, and turns the screen on when wake on new message is on.

## Siri

Fn+Enter starts Siri on the iPhone, from either mode, locked or unlocked, without changing the Cardputer's screen. Hold it while you start speaking, as a headset button. You speak to Siri through the iPhone's or the earphones' microphone: the Cardputer has no Bluetooth audio and cannot act as a microphone.

- `` ` `` right after Fn+Enter closes Siri (it sends Esc; if Siri already closed, Esc reaches the app, e.g. WhatsApp goes back to the chat list).
- Replies by voice work: "Reply to Mario on WhatsApp …". On a locked iPhone, Siri asks to unlock before sending: press G0 for Keyboard mode, type the passcode on the Cardputer, Enter.
- iPhone settings: Settings > Siri: "Press Side Button for Siri" or "Listen for" on, and "Allow Siri When Locked" on. Keep "Type to Siri" off: it replaces voice Siri.

