# Backlog

### B-1 · done
- **What**: GitHub CI pipeline that builds the firmware and runs the host tests on every push. Only meaningful once a remote exists.
- **From**: firmware-skeleton (interview, 2026-11-11)
- **Done** (2026-11-13): `.github/workflows/ci.yml` runs the host tests and the firmware build on every push; green on the private GitHub repository, no feature of its own.

### B-2 · picked-up
- **What**: Spec for milestone M1 (ANCS): advertising, pairing with the iPhone and serial dump of WhatsApp notifications, with the GATT table already complete with HID.
- **From**: firmware-skeleton (interview, 2026-11-11)
- **Picked up by**: ancs-capture (2026-11-12)

### B-3 · picked-up
- **What**: Fetch the attributes (sender, text) of every PreExisting notification at connection: today only the first 64 attribute requests are queued and the rest of the ~110-notification flood are dropped. Needed once notifications are shown on screen.
- **From**: hid-keyboard (dev task 04, 2026-11-12)
- **Picked up by**: message-store (2026-11-12)

### B-4 · picked-up
- **What**: Request attributes only for notifications in the Social category (if WhatsApp uses it): the ~120 notifications of other apps fetched one at a time at every connection delay a new WhatsApp message by up to ~11 s.
- **From**: reading-screens (hardware test, 2026-11-12)
- **Picked up by**: settings-power (2026-11-12)

### B-5 · picked-up
- **What**: Spike on replying through Siri without taking the iPhone out: send the consumer Voice Command usage from the Cardputer and dictate "send a WhatsApp to …" through earphones. To verify: whether Siri answers with the iPhone locked, and whether WhatsApp accepts the dictated message.
- **From**: settings-power (interview, 2026-11-12)
- **Picked up by**: siri-reply (2026-11-12)

### B-6 · open
- **What**: Measure the connected-standby autonomy with the screen off and the slow link timings (from full charge to shutdown, battery and uptime logged once a minute) and state it in the README, against the 12 h target of the project spec.
- **From**: settings-power (review, 2026-11-12)

### B-7 · picked-up
- **What**: Incoming calls on the Cardputer: show the caller from the ANCS incoming-call notification and answer or decline with a key through ANCS notification actions; the audio stays on the iPhone or earphones.
- **From**: siri-reply (interview, 2026-11-12)
- **Picked up by**: incoming-calls (2026-11-13)

### B-8 · open
- **What**: Show missed-call notifications in the Inbox next to the Chats, removed when cleared on the iPhone.
- **From**: incoming-calls (interview, 2026-11-13)

### B-9 · open
- **What**: Find out whether the Cardputer can hang up an answered call (ANCS sends nothing once a call is answered; check HID telephony or consumer usages on iOS).
- **From**: incoming-calls (interview, 2026-11-13)

### B-10 · open
- **What**: Test a WhatsApp message dictated to Siri started from the Cardputer (Fn+Enter), once with earphones and once with only the iPhone's microphone (iPhone in a pocket), and record in the README whether it is sent.
- **From**: siri-reply (dev task 02, 2026-11-13)
