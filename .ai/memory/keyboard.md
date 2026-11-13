# Keyboard

Sending keys from the Cardputer to the iPhone as a Bluetooth keyboard, and the modes that decide where keys go.

## Terms

### Reading mode
The mode in which keys stay on the Cardputer and nothing reaches the iPhone; active at boot. G0 switches to **Keyboard mode**.
_Avoid_: "read mode", "Lettura" in English artifacts (one name per mode).

### Keyboard mode
The mode in which every key is sent to the iPhone as an HID report, by key position. G0 switches back to **Reading mode**.
_Avoid_: "typing mode", "HID mode" (names of the mechanism, not of the mode).

## Decisions

- **Keycaps type their legend, accents through combinations**: the iPhone hardware layout stays "Automatic" (behaves as U.S.); Fn+vowel types the grave vowels and Ctrl+e types é, sent as Option dead-key sequences. Why: the user wants keys to type what is printed on them; the Italian layout made `;` type `ò`.
- **Echo line cleared after 10 s without a key**: besides Enter and leaving Keyboard mode. Why: iOS sends a keyboard no app or text-field change, so the echo would otherwise keep text from the previous app.
- **` right after Fn+Enter closes Siri with Esc, within 60 s**: if Siri already closed on its own, Esc reaches the app underneath (WhatsApp goes back to the chat list). Why: iOS never tells a keyboard when Siri closes; the user accepted the side effect.
- **No typed text in the serial log in Keyboard mode**: only mode changes, errors and a count of keys. Why: typed text includes passwords.

## Gotchas

- **NimBLE notifies without a subscription**: `notify()` on the keyboard input report sends even when the iPhone has not enabled it, so readiness is tracked from the `onSubscribe` callback (which NimBLE also fires as a restore for a bonded peer at reconnection). Never treat a successful `notify()` as "the iPhone is listening".
- **Held letters open the iOS accent popup**: with a hardware keyboard iOS shows the accent picker for a held letter instead of repeating it (repeat happens only if the popup is disabled on the iPhone). Accepted: the user finds the popup handy.
- **Siri needs a held Voice Command after a wake key**: a tap of the consumer Voice Command (0xCF) only wakes the iPhone; Siri starts when it is held (≥ 800 ms), and on a locked iPhone with notifications the first key only lights the screen, so a Shift tap goes first and the hold starts 400 ms later. Verified on the iPhone.
- **Siri cannot be answered by voice through the Cardputer**: the ESP32-S3 has no Bluetooth Classic nor LE Audio, so the microphone is the iPhone's or the earphones'. A locked iPhone asking to unlock is answered by typing the passcode in Keyboard mode.
- **Fn+Space Eject works on iOS**: the consumer Eject usage (0xB8) toggles the on-screen keyboard while the hardware keyboard stays connected; verified on the iPhone.
