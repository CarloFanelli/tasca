# Tasca

Firmware for the M5Stack Cardputer v1.1 (ESP32-S3) that shows WhatsApp notifications from a paired iPhone over BLE (ANCS) and acts as a BLE HID keyboard to reply. C++, PlatformIO, Arduino framework.

## Commands

All commands run from the repository root. The first build needs network access: PlatformIO downloads the pinned platform, toolchain and libraries into `.pio/` and `~/.platformio/`.

| Action | Command |
| --- | --- |
| Build | `pio run -e cardputer` |
| Flash | `pio run -e cardputer -t upload` |
| Serial monitor | `pio device monitor -e cardputer` |
| Host tests | `pio test -e native` |
| Flash, then monitor | `pio run -e cardputer -t upload -t monitor` |
| Clean | `pio run -e cardputer -t clean` |

PlatformIO CLI is installed with Homebrew (`brew install platformio`). Host tests need a working Apple C++ compiler (Command Line Tools, `xcode-select --install`).

## Host tests

`pio test -e native` compiles only `src/core/` together with the tests in `test/`, using the Mac's compiler. Code under `src/core/` must not include Arduino, ESP-IDF or M5Stack headers: anything hardware-specific stays outside it.

## Serial log

Every line is `[L][TAG] message`, with L one of D, I, W, E and TAG a module name of at most 6 characters (`SYS`, `KEY`, `BTN`, later `BLE`, `ANCS`, `HID`). Log through `logMessage()` in `src/log.h`, never with `Serial.print` directly. Messages longer than 256 characters are truncated with `...`.

The minimum level is set at build time with `-DTASCA_LOG_MIN_LEVEL=` (0 Debug, the default, 1 Info, 2 Warn, 3 Error), e.g. `PLATFORMIO_BUILD_FLAGS="-DTASCA_LOG_MIN_LEVEL=1" pio run -e cardputer`.

At startup the firmware waits up to 1.5 s for a serial monitor, then logs one boot report: `[I][SYS] boot version=… psram=… heap=… flash=…` (`psram=none` on the Cardputer v1.1). A monitor opened later misses it: reset the board with the monitor open to see it again.

## Serial port

On macOS the Cardputer's native USB appears as `/dev/cu.usbmodem*`. `platformio.ini` restricts both upload and monitor to that pattern, so Bluetooth serial ports (headphones, `Bluetooth-Incoming-Port`) are never picked. With no Cardputer connected, flashing fails immediately with "Please specify `upload_port`".

- List ports: `pio device list`
- Override the port: `pio run -e cardputer -t upload --upload-port /dev/cu.usbmodemXXXX` and `pio device monitor -e cardputer -p /dev/cu.usbmodemXXXX`

## Recovery: download mode

If the board does not show up on USB (e.g. the firmware crashes in a boot loop), hold the G0 button while plugging in the USB cable, then flash. In download mode the firmware does not run; press reset after flashing.

## Dependencies

The platform and every library, transitive ones included, are pinned to exact versions in `platformio.ini`. When adding a library, pin it and list any of its unpinned dependencies explicitly.

## Concurrency rules

- NimBLE callbacks run on the BLE host task: they never touch the display, the SD card or the message store. They copy the data and push it onto a FreeRTOS queue.
- The main loop is the only reader of that queue, the only writer of the store and the only code that draws.
- GATT writes (ANCS Control Point, HID reports) are issued only from the main loop.
- Redraw only on state change (dirty flag), never on every loop iteration.

## References

- [Apple ANCS specification](https://developer.apple.com/library/content/documentation/CoreBluetooth/Reference/AppleNotificationCenterServiceSpecification/Specification/Specification.html): write the ANCS parser against the spec, not from memory.
- [M5Stack Cardputer v1.1 docs](https://docs.m5stack.com/en/core/Cardputer_v1.1)
