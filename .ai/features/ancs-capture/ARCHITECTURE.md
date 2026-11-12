---
feature: ancs-capture
spec-version: 1.0
status: approved
---

# Architecture Plan: ANCS Capture

## Technical Approach

### Overview

Adds NimBLE-Arduino 2.5.2 (pinned) and the `src/ble/` modules planned in the Tasca spec §5.2: `ble_manager` (init, security, advertising, connection state, bonds), `hid_keyboard` (GATT table) and `ancs_client` (discovery, subscriptions, Control Point queue). NimBLE callbacks run on the host task: they only copy into FreeRTOS queues, and `loop()` in `src/main.cpp` drains them, logs, draws and issues every blocking GATT call. Pure logic (hex dump, request encoding, response completion, combo hold) goes in `src/core/` with host tests, following `src/core/log_format.*` and `test/test_log_format/`. The library's own `examples/ANCS/ANCS.ino` is the reference for the same-connection client (`NimBLEServer::getClient`), not for its structure (it blocks in callbacks and in `loop`).

### Shared Concerns

#### Host-task to loop hand-off

- **Appears in**: Task 01, Task 03, Task 04
- **Reference example**: CLAUDE.md § Concurrency rules
- **Rule**: callbacks build a fixed-size event (kind, conn handle, up to 256 data bytes) and `xQueueSend` it with zero timeout; only `loop()` reads queues, calls `logMessage`, draws or touches NimBLE client objects.
- **Watch out for**: a full queue drops silently — count drops and log a `W` line from `loop()`.

#### Status line

- **Appears in**: Task 01, Task 02, Task 03
- **Reference example**: `src/main.cpp:16` `drawKeyLabel`
- **Rule**: one band between the version (y 66) and the key area (`kKeyAreaTop` 88), redrawn only when phase or count changes, skipped when `displayReady` is false.

## Per-Task Technical Guidance

### Task 01: ble-pairing

**Files to modify:**
- `platformio.ini` — add `h2zero/NimBLE-Arduino@2.5.2` to `[env:cardputer]` `lib_deps`.
- `src/main.cpp` — call BLE init in `setup()`, poll it in `loop()`, draw the status band.
- `CLAUDE.md` — tags `BLE`/`ANCS`, "Forget This Device" recovery.

**Files to create:**
- `src/ble/ble_manager.{h,cpp}` — `NimBLEDevice::init("Tasca")`, security, server callbacks, advertising, phase enum, event queue.
- `src/ble/hid_keyboard.{h,cpp}` — `NimBLEHIDDevice`: report map with keyboard (ID 1) and consumer control (ID 2), PnP, HID info, battery; no report is ever notified.

**Patterns to reuse:**
- Host-task to loop hand-off; Status line (Shared Concerns).
- `examples/ANCS/ANCS.ino` for server callbacks and `advertiseOnDisconnect(true)`.

**Key decisions:**
- Security: `setSecurityAuth(true, false, true)`, IO cap `BLE_HS_IO_NO_INPUT_OUTPUT` (Just Works, LESC).
- Advertising built with `NimBLEAdvertisementData`: flags, `setAppearance(0x03C1)`, 16-bit 0x1812, plus raw AD type 0x15 (128-bit solicitation) via `addData`; name only in scan response.
- Single bond enforced in `onAuthenticationComplete`: a second peer's bond is deleted and the link dropped.

**Watch out for:**
- iPhone uses a rotating private address: without identity keys the bond is not recognized after reboot — `NimBLEDevice.cpp:1000` distributes only ENC keys → set init and resp key dist to ENC|ID; test: reboot, reconnect without pairing.
- `onAuthenticationComplete` fires on failure too — `NimBLEServer.cpp:634` → check `connInfo.isEncrypted()` before "secured"; test: cancel pairing on the iPhone.
- Default store keeps 3 bonds and evicts the oldest — `NimBLEDevice.cpp:1397` → enforce one bond ourselves; test: pair a second phone.

---

### Task 02: bond-reset

**Files to modify:**
- `src/main.cpp` — feed the pressed-key snapshot of `pollKeyboard()` (line 41) to the hold detector; on trigger call the manager's reset.
- `src/ble/ble_manager.{h,cpp}` — reset: disconnect if connected, `NimBLEDevice::deleteAllBonds()`, restart advertising, log and set a "pairing reset" status.

**Files to create:**
- `src/core/combo_hold.{h,cpp}` — pure detector: fires once when a given key set stays held for a duration, given timestamps; follows `src/core/key_events.*`.
- `test/test_combo_hold/test_main.cpp` — follows `test/test_key_events/test_main.cpp`.

**Patterns to reuse:**
- `KeyPos` from `src/core/key_events.h:9`; Fn is (2,0), Del is (0,13) per `src/core/key_events.cpp:13`.

**Key decisions:**
- Time is passed in (`millis()` from `loop()`), keeping the detector host-testable.
- Fires once per hold: keeping the keys down after 3 s does not repeat the reset.

**Watch out for:**
- `getNumBonds()` reads the store: calling reset from a callback would race the host — `src/main.cpp:104` loop → call only from `loop()`.

---

### Task 03: ancs-subscription

**Files to modify:**
- `src/ble/ble_manager.{h,cpp}` — on encryption success, hand the connection to the ANCS client; on disconnect, reset it.
- `src/main.cpp` — drain ANCS events, dump them, update count and "ANCS active/denied".

**Files to create:**
- `src/ble/ancs_client.{h,cpp}` — from `loop()`: `NimBLEServer::getClient(connInfo)`, discover the ANCS service by UUID, subscribe DS then NS with callbacks that only enqueue.
- `src/core/hex_dump.{h,cpp}` — splits a packet into ≤32-byte lines `NS len=N +off: xx xx …`; follows `src/core/log_format.*`.
- `test/test_hex_dump/test_main.cpp`.

**Patterns to reuse:**
- Host-task to loop hand-off; Status line.
- UUIDs and subscribe calls in `examples/ANCS/ANCS.ino:5-8`.

**Key decisions:**
- Separate queues: NS (8-byte items, depth 128, for PreExisting floods) and DS (256-byte items, used in Task 04).
- A line of 32 bytes is 95 hex chars plus prefix: under the 256-char log limit (`src/core/log_format.h:8`).

**Watch out for:**
- `getClient()` deletes the discovered services each call — `NimBLEServer.cpp:1113` → call once per encrypted connection, not per loop.
- Discovery and `subscribe(true, cb, true)` block on the host — never call them from a callback (deadlock); `loop()` only.

---

### Task 04: attribute-requests

**Files to modify:**
- `src/ble/ancs_client.{h,cpp}` — request queue (cap 64), one in flight, 2 s timeout via `millis()`, `writeValue(..., true)` on Control Point, clear on disconnect.
- `src/main.cpp` — drain DS events, dump with channel `DS`.

**Files to create:**
- `src/core/ancs_request.{h,cpp}` — encodes Get Notification Attributes (CommandID 0, UID little-endian, attributes 0,1+64,2+64,3+512,5) and tracks response completion over fragments: header, then 5 tuples (id, 2-byte length, value).
- `test/test_ancs_request/test_main.cpp`.

**Patterns to reuse:**
- Hex dump from Task 03; Host-task to loop hand-off.

**Key decisions:**
- Completion counts the five requested tuples; it is not a parser (values are not interpreted).

**Watch out for:**
- A Control Point error comes back as a failed write, not on Data Source — `NimBLERemoteValueAttribute.h:61` returns false → log `W`, start the next request; test: remove the notification before its request.
- Late fragments of a timed-out response would be counted for the next request → restart completion only on a packet starting with CommandID 0 and the in-flight UID; stale fragments are dumped but not counted; test: timeout then late packet.
