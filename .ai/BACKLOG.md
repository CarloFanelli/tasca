# Backlog

### B-1 · open
- **What**: GitHub CI pipeline that builds the firmware and runs the host tests on every push. Only meaningful once a remote exists.
- **From**: firmware-skeleton (interview, 2026-11-11)

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

### B-5 · open
- **What**: Spike on replying through Siri without taking the iPhone out: send the consumer Voice Command usage from the Cardputer and dictate "send a WhatsApp to …" through earphones. To verify: whether Siri answers with the iPhone locked, and whether WhatsApp accepts the dictated message.
- **From**: settings-power (interview, 2026-11-12)

### B-6 · open
- **What**: Measure the connected-standby autonomy with the screen off and the slow link timings (from full charge to shutdown, battery and uptime logged once a minute) and state it in the README, against the 12 h target of the project spec.
- **From**: settings-power (review, 2026-11-12)
