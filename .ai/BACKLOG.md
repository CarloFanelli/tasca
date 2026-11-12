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
