# Tasca

Firmware for the M5Stack Cardputer v1.1 (ESP32-S3) that shows WhatsApp notifications from a paired iPhone over BLE (ANCS) and acts as a BLE HID keyboard to reply. C++, PlatformIO, Arduino framework.

## Commands

All commands run from the repository root. The first build needs network access: PlatformIO downloads the pinned platform, toolchain and libraries into `.pio/` and `~/.platformio/`.

| Action | Command |
| --- | --- |
| Build | `pio run -e cardputer` |
| Flash | `pio run -e cardputer -t upload` |
| Serial monitor | `pio device monitor -e cardputer` |
| Flash, then monitor | `pio run -e cardputer -t upload -t monitor` |
| Clean | `pio run -e cardputer -t clean` |

PlatformIO CLI is installed with Homebrew (`brew install platformio`).

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
