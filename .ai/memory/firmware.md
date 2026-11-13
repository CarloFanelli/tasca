# Firmware

Cardputer firmware build, toolchain, hardware facts and on-device behavior.

## Decisions

- **No PSRAM budget**: memory plans assume the 8 MB flash and about 330 KB free heap only. Why: the boot report on the Cardputer v1.1 shows `psram=none`.
- **Host tests compile only `src/core/`**: hardware-free logic lives there and must not include Arduino, ESP-IDF or M5Stack headers. Why: `pio test -e native` builds it with the Mac's compiler, so the same code is tested without the board.
- **Every dependency pinned exactly, transitive ones included**: M5Cardputer pulls M5Unified, M5GFX and IRremote with `*`, so they are listed explicitly in `platformio.ini`. Why: two clean builds of the same commit must use identical versions.
- **Fn+Del pairing reset restarts the board**: after erasing the bond the Cardputer shows "pairing reset" for 1.5 s and restarts. Why: without a restart iOS did not list "Tasca" again after "Forget This Device", although it kept advertising.
- **Failed encryption shows "pairing failed"**, not "pairing invalid — Fn+Del". Why: after "Forget This Device" the iPhone pairs again on its own, so Fn+Del is not the fix the message would suggest.

- **Slow link timings while the screen is off**: 120–150 ms, latency 4, 6 s timeout; with the screen on, a fixed 15–30 ms window, latency 0, 2 s. Why: power saving within Apple's accessory rules, which require the requested maximum interval to exceed the minimum by at least 15 ms, so the exact timings iOS picked cannot be requested back.

## Gotchas

- **Display rotation**: with M5GFX 0.3.0 the Cardputer panel is already 240x135 landscape at rotation 0; the official M5Stack examples call `setRotation(1)`, which turns the text 90°. Don't copy that call.
- **Serial port auto-detection**: on macOS PlatformIO picked Bluetooth serial ports (headphones) for upload; `platformio.ini` restricts upload and monitor to `/dev/cu.usbmodem*`.
- **Boot report can be missed**: it is printed once, after waiting at most 1.5 s for a monitor; open the monitor and reset the board (RTS toggle) to capture it.
- **Keyboard change detection**: the M5Cardputer `isChange()` compares only how many keys are down, so a release and a press in the same scan go unnoticed; diff the physical key list instead.
- **Broken Apple C++ headers**: leftover files in `/Library/Developer/CommandLineTools/usr/include/c++` hid libc++ and broke host builds (`'string' file not found`); reinstalling the Command Line Tools fixed it.
- **iOS hides a just-forgotten device**: after "Forget This Device", iOS did not list "Tasca" under Other Devices while it kept advertising (the Mac saw it); a restart of the Cardputer made it appear at once. That is why the Fn+Del reset restarts the board.
- **Stale BLE events after a reset**: iOS repeats "secured" events; one queued before the bond was erased can arrive after it, so don't trust queued events across a bond reset.
- **ANCS silent when sharing is off**: with "Share System Notifications" off, iOS accepts the Notification Source subscription and sends nothing, so a "denied" state cannot be detected from the subscription result; check the switch first when no NS packet arrives.
- **ANCS UIDs restart at 0 on every connection**: a Notification Source packet left from a closed connection must not become an attribute request on the next one, where the same UID names another notification.
- **std::vector costs flash on the ESP32-S3**: a vector of screen pointers added about 2.5 KB of template code; a fixed array did the same job for nothing. Prefer fixed-size containers for small bounded lists in firmware-only code. A `std::deque<uint32_t>` cost about 15 KB; a `std::vector` of a type already used elsewhere costs almost nothing.
