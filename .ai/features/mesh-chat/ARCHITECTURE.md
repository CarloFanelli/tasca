---
feature: mesh-chat
spec-version: 1.0
status: approved
---

# Architecture Plan: Mesh chat

## Technical Approach

### Overview

The LoRa unit is a RAK3172 (RUI 4.0.6) on `Serial1`, RX G1, TX G2, 115200 baud, driven by AT lines; the spike set it with `AT+RFFREQUENCY=869525000`, `AT+SPREADINGFACTOR=9`, `AT+BANDWIDTH=1`, `AT+CODINGRATE=0`, `AT+PREAMBLELENGTH=16`, `AT+IQINVER=0`, `AT+SYNCWORD=24B4`, `AT+ENCRY=0`, then `AT+PRECV=65535`, and received `+EVT:RXP2P:rssi:snr:hex`. Everything that can be tested without the board lives in `src/core/`: the AT line protocol, the Meshtastic packet (16-byte header, AES-128-CTR with the published key, protobuf `Data`/`User`), the Mesh store, the clock and the history text format. A thin `src/mesh/lora_unit.cpp` owns the serial port and is polled from `loop()` like `ams::poll`. No new library: AES is a small portable implementation in `src/core/` (mbedtls is not available to host tests).

### Shared Concerns

#### Text entry

- **Appears in**: Task 03, Task 04
- **Reference example**: `src/screens/keyboard_screen.cpp:38` (`KeyboardMapper` feeding `EchoLine`)
- **Rule**: local typing reuses `KeyboardMapper` reports applied to an `EchoLine`, so legends, Fn+vowel and Ctrl+e match Keyboard mode; a byte or character limit drops the extra character.

#### Mesh screens on top

- **Appears in**: Task 02, Task 05, Task 07
- **Reference example**: `src/screens/music_screen.cpp` (a `ReadingScreen` opened from the Menu)
- **Rule**: the Mesh screen and the Node list are `ReadingScreen`s; `closeToInbox` (`src/screens/menu_screen.cpp:29`) removes both.

## Per-Task Technical Guidance

### Task 01: mesh-receive

**Files to create:**
- `src/core/at_link.h/.cpp` + `test/test_at_link/` — line splitter, OK/ERROR matching, `+EVT:RXP2P` parsing, detection state (probe `AT` every 2 s while absent, every 5 s while present, absent after 2 missed answers).
- `src/core/aes128.h/.cpp` + test — FIPS-197 vector.
- `src/core/meshtastic.h/.cpp` + `test/test_meshtastic/` — header, channel hash, CTR nonce (packet id 64-bit LE, sender 32-bit LE, zeros), `Data` and `User` decoding; test with the packet captured on 2026-11-19 (`RESS`).
- `src/core/mesh_store.h/.cpp` + test — messages deduplicated by (sender, id), unread count, nodes by ID.
- `src/mesh/lora_unit.h/.cpp` — `Serial1`, config sequence, `poll()`.
- `src/screens/mesh_screen.h/.cpp` — pattern `src/screens/chat_screen.cpp` (`wrapText`, scroll).

**Files to modify:**
- `src/main.cpp` — `mesh::poll()` beside `ams::poll()`; dirty flags when the store changes.
- `src/screens/menu_screen.cpp` — "Mesh" item after Find iPhone, unread value in green.

**Key decisions:**
- Node ID: the last 4 bytes of the Bluetooth MAC, as Meshtastic does.

**Watch out for:**
- `Serial1` read only in `loop()`; one RX line is about 520 characters → `setRxBufferSize(2048)` before `begin`; parse at most a few lines per loop so drawing is not delayed.

---

### Task 02: menu-without-iphone

**Files to modify:**
- `src/main.cpp:103` (`pollG0`) — allow Fn+G0 when the Connection screen is open; G0 alone stays inert there.
- `src/screens/connection_screen.cpp:29` (`sync`) — do not open while the Mesh screen or Node list is on top; open on the next loop once they leave.
- `src/screens/menu_screen.cpp` — items needing the iPhone do nothing unless `ble::phase()` is Secured; `closeToInbox` must not remove the Connection screen.
- `src/screens/mesh_screen.cpp` — a key down silences `outOfRangeAlert`, as `connection_screen.cpp:58`.

**Watch out for:**
- `sync` runs before `pollG0` every loop (`src/main.cpp:198`) → the Menu opens above the Connection screen, which stays in the stack; `remove` on Secured takes it out from under the Menu. Test: connect while the Menu is open.
- The Call screen closes under the Connection screen (`src/main.cpp:196`): keep that order.

---

### Task 03: mesh-send

**Files to modify:**
- `src/core/meshtastic.h/.cpp` — encode a text `Data`, header with hop limit 3, hop start 3, our ID, a random packet ID.
- `src/core/mesh_store.h/.cpp` — own messages with mark sent / heard / not heard / not sent, 30 s timer; a received packet with our sender and ID marks "heard".
- `src/core/lora_airtime.h/.cpp` + test — LoRa time on air (SF9, 250 kHz, CR 4/5, preamble 16) and a 1-hour sliding total against 360 s.
- `src/mesh/lora_unit.cpp` — `AT+PRECV=0`, `AT+PSEND=<hex>`, wait for `+EVT:TXP2P DONE`, `AT+PRECV=65535`.
- `src/screens/mesh_screen.cpp` — compose line (Text entry, Shared Concerns), bytes left.

**Watch out for:**
- Whether RUI accepts `AT+PSEND` during continuous receive is unproven → stop receive first and restart after the TX event; check on the device in this task.

---

### Task 04: mesh-settings

**Files to modify:**
- `src/core/settings.h/.cpp` — `Setting::MeshSound` (On by default) after `FindIphone`, accessor.
- `src/settings_store.h/.cpp` — key `"meshsound"`; Mesh name as a string key `"meshname"`, read at `load`.
- `src/core/settings_nav.h/.cpp` — a text row between settings and actions: Enter edits, no prompt.
- `src/screens/settings_screen.cpp` — draw the row, edit it in place (Text entry, Shared Concerns), 24 characters.
- `src/core/meshtastic.h/.cpp` — encode `User` (id `!xxxxxxxx`, long, short, role CLIENT_MUTE).
- `src/mesh/lora_unit.cpp` — announce at detection, on change, every 3 h.
- `src/main.cpp` — Mesh arrivals: beep if `meshSound()` and no call, wake if `wakeOnMessage()`; pattern `pollArrivals` (`src/main.cpp:44`).

**Watch out for:**
- `kSettingCount` sizes `kKeys` and `kSpecs` (`src/core/settings.cpp:16`, `src/settings_store.cpp:15`) → add to both; test the default is On.

---

### Task 05: node-list

**Files to create:**
- `src/screens/node_list_screen.h/.cpp` — rows of name, short name, RSSI, age; pattern `src/screens/inbox_screen.cpp`.

**Files to modify:**
- `src/core/mesh_store.h/.cpp` + test — nodes ordered by last heard.
- `src/screens/mesh_screen.cpp` — Tab opens it.
- `src/screens/connection_screen.cpp`, `src/screens/menu_screen.cpp` — include it where the Mesh screen is named (Shared Concerns).

---

### Task 06: iphone-clock

**Files to create:**
- `src/core/wall_clock.h/.cpp` + test — set from date and time at a `millis()` value, current time with wrap-safe elapsed, "HH:MM".
- `src/ble/cts_client.h/.cpp` — read Current Time (service 0x1805, characteristic 0x2A2B); pattern `src/ble/ams_client.cpp` `setUp`.

**Files to modify:**
- `src/ble/ble_manager.cpp:116` — `cts::setUp(gattClient)` beside `ams::setUp`.
- `src/core/mesh_store.h/.cpp`, `src/screens/mesh_screen.cpp` — arrival time when known.

**Watch out for:**
- A client read only: the GATT server table must not change (CLAUDE.md) → no new local service or characteristic.

---

### Task 07: mesh-history

**Files to create:**
- `src/core/mesh_history.h/.cpp` + test — one line per message or node, tab-separated, escaped text; parse tolerant of a cut last line.
- `src/mesh/mesh_storage.h/.cpp` — `SD` on its own `SPIClass` (SCK 40, MISO 39, MOSI 14, CS 12, from M5Cardputer `examples/Basic/sdcard`), `Tasca/Mesh/`, append, rewrite past 500.

**Files to modify:**
- `src/main.cpp` — load at startup; mark "not saved" on failure.
- `src/screens/mesh_screen.cpp` — Del twice via `ClearPrompt` (`src/core/clear_prompt.h`).

**Watch out for:**
- Writes are blocking → only from `loop()`, appended per message; the 500 rewrite happens at most once per 50 new messages.

---

### Task 08: device-check

**Files to modify:**
- `README.md` — Mesh section beside Find iPhone; history privacy.
- `.ai/features/mesh-chat/tasks/08-device-check.md` — what was heard, where, RSSI.
