---
feature: music-menu
spec-version: 1.0
status: approved
---

# Architecture Plan: Music and Menu

## Technical Approach

### Overview

The Menu and the Music screen are two new Reading mode screens (`ReadingScreen`) on the Screen stack; Fn+G0 is caught in `pollG0` (`src/main.cpp:80`) before G0's mode switch. Every Menu item first closes everything above the Inbox (Keyboard mode included), then opens its target, so an item always shows its screen from the start and the stack never exceeds 7 screens. AMS is a second GATT client on the connection ANCS already discovers: discovery moves from `ancs::setUp` to the BLE manager, which takes the client once per connection and hands it to `ancs` and the new `ams` module. AMS decoding and the elapsed-time extrapolation live in `src/core/` with host tests; NimBLE callbacks only copy packets onto a FreeRTOS queue, as ANCS does.

### Shared Concerns

#### Notification queue to the main loop

- **Appears in**: Task 02, Task 03
- **Reference example**: `src/ble/ancs_client.cpp` (`onNotificationSource` → `nsQueue`, drained in `poll`)
- **Rule**: AMS notifications are copied into fixed-size packets on a queue; only `ams::poll` decodes them and changes state.

## Per-Task Technical Guidance

### Task 01: menu

**Files to create:**
- `src/screens/menu_screen.h/.cpp` — three rows, selection, Enter, `` ` ``; pattern: `src/screens/settings_screen.cpp` (rows, `kSelectedBackground`). Unread count from `app::chats`.
- `src/screens/music_screen.h/.cpp` — placeholder "Nothing playing"; `` ` `` closes to the Inbox.
- `src/core/menu_nav.h/.cpp` + `test/test_menu_nav/` — selection without wrap; pattern: `src/core/settings_nav.h`.

**Files to modify:**
- `src/main.cpp` — `pollG0`: with Fn (`{2,0}`) held, open or close the Menu instead of switching mode; G0 alone with the Menu on top closes it, then switches mode.
- `src/app.h/.cpp` — a helper closing every screen above the Inbox (Chat via `chatScreen.close`, Keyboard via `keyboardScreen.close`).
- `src/screens/settings_screen.*`, `src/core/settings_nav.*` — reset the selection when opened from the Menu.

**Key decisions:**
- The Menu closes back over Keyboard mode with `keyboardScreen.resume(pressed)`, as the Connection screen does.

**Watch out for:**
- Fn held in Keyboard mode before G0 is read by the keymap as a modifier (`src/core/keymap.cpp:122`) → Fn alone sends no key; check on device that Fn+G0 types nothing.
- Settings' pending prompt must cancel when covered — `settings_screen.h:18` already does it in `onCovered`; keep the Menu opened through `screens.open`.

---

### Task 02: now-playing

**Files to create:**
- `src/core/ams.h/.cpp` + `test/test_ams/` — decode Entity Update (entity, attribute, flags, UTF-8 value), Remote Command list, PlaybackInfo "state,rate,elapsed"; `NowPlaying` state with elapsed extrapolated from a wrapping ms clock. Pattern: `src/core/ancs_request.cpp`.
- `src/ble/ams_client.h/.cpp` — subscribe Entity Update (before any write), then write Player(Name, PlaybackInfo, Volume) and Track(Artist, Title, Duration); read Entity Attribute for a truncated Title/Artist, 128 bytes max.

**Files to modify:**
- `src/ble/ble_manager.cpp` — on Secured, take the GATT client once and run `ancs` then `ams` setup; subscribe the iPhone's Service Changed (0x2A05) and rerun both on an indication.
- `src/ble/ancs_client.*` — `setUp` takes the client instead of calling `getClient`.
- `src/screens/music_screen.cpp`, `menu_screen.cpp` — draw the data; `update` redraws once per second while the screen is on.

**Key decisions:**
- "iPhone not connected" whenever AMS is not subscribed; with no track, "Nothing playing".

**Watch out for:**
- A second `getClient` drops every discovered service, so ANCS goes silent — `NimBLEServer.cpp:1113` → one `getClient` per connection, shared; test: messages still arrive with AMS on.
- Rediscovery after Service Changed frees ANCS characteristics still held in `controlPoint` — `ancs_client.cpp:207` → clear both modules' pointers before rediscovery.
- Titles reaching the log break the spec's privacy rule — no existing guard → log only lengths and attribute IDs, never values, at any level.

---

### Task 03: music-controls

**Files to modify:**
- `src/ble/ams_client.*` — write one Remote Command byte with response, from `poll` only; `W` line with `getLastError()` on refusal (mirror `ancs_client.cpp:272`); keep the available-commands list from Remote Command notifications.
- `src/screens/music_screen.cpp` — Enter TogglePlayPause, `,` Previous, `/` Next, `;` `.` VolumeUp/Down; key hints dimmed when not offered.

**Files to create:**
- `src/core/key_repeat.h/.cpp` + `test/test_key_repeat/` — fires on press, then every 200 ms while held; pattern: `src/core/combo_hold.cpp`.

**Watch out for:**
- A blocking write from a NimBLE callback deadlocks the host task — CLAUDE.md concurrency rules → commands only from `handleKeys` in the main loop.

---

### Task 04: iphone-check

**Files to modify:**
- `README.md` — Fn+G0 Menu and Music screen keys, beside the Reading mode key table.
- `.ai/features/music-menu/tasks/04-iphone-check.md` — checklist and outcome with Spotify and Apple Music.
