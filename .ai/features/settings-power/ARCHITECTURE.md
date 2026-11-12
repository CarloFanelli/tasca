---
feature: settings-power
spec-version: 1.0
status: approved
---

# Architecture Plan: Settings and Power

## Technical Approach

### Overview

What needs no hardware lives in `src/core/` and is host-tested:
- the Settings model: values, steps, defaults, labels, and the encoding of what is stored;
- the screen sleep timer;
- the Settings navigation with its confirmation prompt.

Storage goes through the Arduino `Preferences` library (NVS), which is part of the pinned Arduino core (`~/.platformio/packages/framework-arduinoespressif32/libraries/Preferences`), so no new dependency is added. It sits in a small `src/settings_store.*` outside core.

`main.cpp` keeps drawing and key dispatch, as for the Inbox (`src/main.cpp:175-371`). The Settings screen is a third Reading mode screen next to Inbox and Chat screen. Link timings belong to `ble_manager`, and the category filter to `ancs_client`.

### Shared Concerns

#### Reading mode screens are dispatched in `pollReadingMode`

- **Appears in**: Task 01, Task 02, Task 03
- **Reference example**: `src/main.cpp:460-496`, keys matched by position with `isKey`, Fn ignored.
- **Rule**: Settings keys follow the same dispatch. The screen-off check comes first, because the key that wakes the screen in Reading mode is consumed.

#### Live arrivals drive beep, wake and Banner

- **Appears in**: Task 03, Task 04
- **Reference example**: `pollArrivals()` in `src/main.cpp:298`, based on `MessageStore::liveAdded()`.
- **Rule**: beep, wake and the Banner react to the same live-arrival check. Resent notifications and updated Messages never trigger them.

## Per-Task Technical Guidance

### Task 01: settings-screen

**Files to create:**
- `src/core/settings.{h,cpp}`:
  - the four settings with their value lists, defaults and labels ("60 %", "30 s", "Never");
  - stepping clamped at the ends;
  - validation of a stored value, where an out-of-range value falls back to the default.
  - Pattern: `src/core/inbox.*`. Test: `test/test_settings`.
- `src/settings_store.{h,cpp}`: load and save through `Preferences` under one namespace; a failed `begin` or `put` logs `W` tagged `SYS`. Pattern: `src/log.cpp` for a small hardware-side module.
- Settings selection, in `src/core/reading_nav.*` or a new core file beside it: selected row, clamped. Test in `test/test_reading_nav`.

**Files to modify:**
- `src/main.cpp`: load settings in `setup()` before the first draw; Opt (`kModOpt`, `src/core/key_events.h:18`) + s (2,3) opens Settings; draw it like `drawInbox`; apply brightness with `M5Cardputer.Display.setBrightness` (0–255 scale).

**Key decisions:**
- `main.cpp` is 586 lines: the Settings drawing goes into it beside the other screens, while the model and its storage go to the new files. The screens share fonts, layout constants and dirty flags.

---

### Task 02: settings-actions

**Files to modify:**
- Settings navigation: a pending-confirmation state with its start time; Enter confirms; another key or 3 s cancels. Pattern: the time-based state in `src/core/combo_hold.*`. Tests in `test/test_reading_nav` or `test/test_settings`.
- `src/core/message_store.{h,cpp}`: empty every Message, memory count back to 0. Test in `test/test_message_store`.
- `src/ble/ancs_client.{h,cpp}`: a loop-only call that clears the store and logs the counts (`logCounts`, `src/ble/ancs_client.cpp:97`).
- `src/main.cpp`: Reset pairing reuses the Fn+Del path (`src/main.cpp:461-469`), restarting even when `ble::resetPairing()` returns false because there was no bond. After clearing, rebuild the Inbox and close the Chat screen.

**Watch out for:**
- Clearing while the reconnection import runs leaves nothing linked, so `removeUnlinked` removes nothing more (`src/ble/ancs_client.cpp` `finishImport`). Harmless: resent notifications are added again as new Messages.

---

### Task 03: beep-and-sleep

**Files to create:**
- `src/core/screen_timer.{h,cpp}`: the last-activity time, the timeout from the setting (Never = off), and whether the screen should be off. Pattern: `src/core/reconnect_import.*`. Test: `test/test_screen_timer`.

**Files to modify:**
- `src/main.cpp`:
  - activity comes from any key press, G0, or a live arrival with wake on;
  - turn the screen off with `Display.sleep()` and on with `Display.wakeup()` (M5GFX `LGFXBase.hpp:1445-1446`); no drawing while it is off;
  - beep with `M5Cardputer.Speaker.tone` (`Speaker_Class.hpp:198`), short and with `stop_current_sound` so beeps never overlap;
  - in `pollG0` and `pollReadingMode`, a key or press that wakes the screen does nothing else; `pollKeyboardMode` always sends.

**Watch out for:**
- Drawing while asleep wastes time, and the dirty flags would then be cleared unseen (`src/main.cpp:575-581`). Keep the flags set while asleep and redraw everything on wake. Test: none (hardware).

---

### Task 04: screen-off-timings

**Files to modify:**
- `src/ble/ble_manager.{h,cpp}`:
  - a loop-only call that requests slow or usual timings for the current connection through `NimBLEServer::updateConnParams` (`NimBLEServer.h:69`);
  - log the granted values from the `onConnParamsUpdate` callback (`NimBLEServer.h:221`), via the event queue (`src/ble/ble_manager.cpp:40`), never logging from the host task.
- `src/main.cpp`: call it on each screen on/off change.

**Key decisions:**
- Timings follow Apple's accessory rules:
  - minimum interval ≥ 15 ms;
  - maximum ≥ minimum + 15 ms;
  - latency ≤ 30;
  - supervision timeout from 2 to 6 s, greater than maximum × (latency + 1) × 3.
- Screen off: interval 120–150 ms, latency 4, timeout 6 s. Screen on: the timings iOS chose at connection.

**Watch out for:**
- With the screen off, a key typed in Keyboard mode waits up to a slow interval. Request the usual timings on wake, before sending. Test: none (hardware: keys lag under 300 ms).

---

### Task 05: whatsapp-category-filter

**Files to modify:**
- `src/core/ancs_request.{h,cpp}`: decode CategoryID (byte 2 of the Notification Source event, ANCS spec) into `AncsEvent`. Test in `test/test_ancs_request`.
- `src/ble/ancs_client.cpp`:
  - first, log the category of every event at `D` and confirm WhatsApp's on hardware;
  - then skip and count events of other categories in `handleEvent` (`src/ble/ancs_client.cpp:180`), with the accepted categories in one list beside `kAcceptedApps`.

**Watch out for:**
- If WhatsApp uses a category outside the list, its Messages are never requested. The reconnection cleanup then deletes every stored Message as gone from the iPhone (`finishImport`). Confirm the category on hardware first; test: a skipped category is counted.

---

### Task 06: autonomy-readme

**Files to create:**
- `README.md`: purpose, iPhone setup (from `CLAUDE.md` § BLE and pairing and the project spec §6), measured autonomy (start charge, settings, hours to shutdown), privacy notice.

**Key decisions:**
- Measure with a serial capture logging battery and uptime every few minutes, using the existing battery poll (`src/main.cpp` `pollBattery`). Log at `I` with tag `SYS` once a minute. Keep the log line: it costs nothing and records the curve.
