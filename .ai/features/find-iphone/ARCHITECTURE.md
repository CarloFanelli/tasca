---
feature: find-iphone
spec-version: 1.0
status: approved
---

# Architecture Plan: Find iPhone

## Technical Approach

### Overview

A fifth setting, Find iPhone, joins `Settings` and its stored keys; the Settings list scrolls with the selection. The BLE manager already logs every disconnection reason: it now reports once when the link was lost by supervision timeout, and reads the link's signal strength on request. Timing and averaging live in `src/core/` with host tests: an `OutOfRangeAlert` (triple Beep due every 5 s for up to 2 minutes) and a `SignalMeter` (2 s average, three levels, Beep period). The main loop runs the alert next to `pollArrivals`; the Find iPhone screen is a new `ReadingScreen` opened from the Menu, like the Music screen.

### Shared Concerns

#### Find iPhone sounds ignore the sound switch

- **Appears in**: Task 02, Task 03
- **Reference example**: `src/main.cpp:49` (`settings.sound()` gating `queueBeeps`)
- **Rule**: alert and Find iPhone screen Beeps check `settings.findIphone()` only, never `settings.sound()`; both go through `app::queueBeeps`.

## Per-Task Technical Guidance

### Task 01: find-iphone-setting

**Files to modify:**
- `src/core/settings.h/.cpp` — `Setting::FindIphone`, `kSettingCount` 5, spec `{"Find iPhone", 2, 1}`, accessor `findIphone()`; pattern: `WakeOnMessage`.
- `src/settings_store.cpp` — key `"findiphone"` in `kKeys`; an absent key keeps the default (`settings_store.cpp:33`).
- `src/core/settings_nav.h/.cpp` — first visible row for a window of rows, following the selection.
- `src/screens/settings_screen.cpp` — draw only the visible window (6 rows: `(135 - kChatTextTop) / kLineHeight`).
- `test/test_settings/` — default On, label, first visible row at both ends.

**Key decisions:**
- Scrolling lives in `SettingsNav` so it is host-tested; the screen only draws rows from the first visible one.

**Watch out for:**
- Seven rows overflow: `kChatTextTop` 36 + 7 × 16 = 148 > 135 (`src/ui/draw.h:14-15`) → draw the window only; test "Clear messages" selected is fully visible.

---

### Task 02: out-of-range-alert

**Files to create:**
- `src/core/out_of_range.h/.cpp` + `test/test_out_of_range/` — start, silence, stop, due every 5 s for 120 s, still active after silence; pattern: `RingTimer` in `src/core/incoming_call.h:61`.

**Files to modify:**
- `src/ble/ble_manager.h/.cpp` — on `Disconnect`, keep "lost by signal" when the reason is supervision timeout; `takeLinkLost()` returns it once; pattern: `ancs::takeNotCleared`.
- `src/core/connection_line.h/.cpp` + test — `outOfRange` argument: "iPhone out of range" after a failed pairing, before "Connecting…".
- `src/main.cpp` — `pollOutOfRange` beside `pollArrivals`: start with Find iPhone on (wake the screen), stop when secured or Find iPhone off, `queueBeeps(3)` when due; G0 silences.
- `src/screens/connection_screen.cpp` — a key down silences; pass the alert to `connectionLine`.

**Watch out for:**
- The reason is a host code: supervision timeout arrives as `BLE_HS_HCI_ERR(BLE_ERR_CONN_SPVN_TMO)` = 0x208 (`ble_hs.h:174`, `ble.h:205`), not 0x08 → compare with the macro.
- A pairing reset disconnects locally (not 0x208): no alert; never derive it from the phase alone.

---

### Task 03: find-iphone-screen

**Files to create:**
- `src/core/signal_meter.h/.cpp` + `test/test_signal_meter/` — readings with a wrapping ms clock, 2 s average, level (None, Far, Near, VeryClose), bar percent, Beep period 2000/1000/250 ms; pattern: elapsed time in `src/core/ams.cpp:138`.
- `src/screens/find_screen.h/.cpp` — bar, word, Beep cadence, 5-minute stop without a key, ` to the Inbox; pattern: `src/screens/music_screen.cpp`.

**Files to modify:**
- `src/ble/ble_manager.h/.cpp` — `linkRssi()`: `ble_gap_conn_rssi` on `securedConn`; 0 when none.
- `src/screens/menu_screen.cpp` — item after Music (`menu_screen.cpp:22`); `closeToInbox` removes the Find iPhone screen too; opening turns the setting on and saves it (`settings_store::save`).
- `src/main.cpp` — redraw the Find iPhone screen on a meter change.

**Key decisions:**
- Thresholds are constants in `signal_meter.cpp`, tuned in Task 04.

**Watch out for:**
- A failed read returns 0 (`NimBLEClient.cpp:623-637`); 0 is never a real reading (always negative) → skip it, "Searching…" with none in 2 s.
- One Beep queued at a time: `queueBeeps` accumulates (`app.cpp:112`); 80 + 120 ms fits the 250 ms period.

---

### Task 04: iphone-check

**Files to modify:**
- `src/core/signal_meter.cpp` — thresholds from readings next to the iPhone, at 2–3 m and at 5 m; test expectations follow.
- `README.md` — Find iPhone row in Settings, the Out-of-range alert, the Find iPhone screen beside the Music section.
- `.ai/features/find-iphone/tasks/04-iphone-check.md` — readings, thresholds and disconnection reasons seen.
