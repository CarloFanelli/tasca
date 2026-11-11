# Task 02: serial-log-and-tests

## Objective

Every serial line follows the tagged `[L][TAG] message` format, the boot report settles PSRAM and memory, and host tests for the log format run on the Mac without the Cardputer.

## Context

**Satisfies**: REQ-4, REQ-6, REQ-8, REQ-9, REQ-10, REQ-14, REQ-15

The log format is reused by every later milestone, and the boot report answers the PSRAM question that sizes M3's buffers. The host test environment is set up here around the first piece of pure logic (log-line formatting); Task 03 adds key tests to it. Display-init failure lives here because it is reported through this log.

## Depends on

- Task 01

## Expected Behavior

- Every serial line has the form `[L][TAG] message`, with L one of D, I, W, E and TAG the module name.
- Lines below the build-time minimum level are not printed; the default minimum is Debug.
- At boot one report is logged with firmware version, PSRAM presence and size, free heap and flash size.
- If the display fails to initialize, the firmware keeps running and logs an `E` line tagged `SYS`.
- `pio test -e native` runs on the Mac with no Cardputer connected and passes.
- Host tests cover log-line formatting for all four levels, truncation and empty inputs.
- The log-formatting code compiles in the native environment without any Arduino, ESP-IDF or M5Stack header.
- CLAUDE.md gains the host test command.

## Acceptance Criteria

- [ ] Every line printed on serial matches `[L][TAG] message` with L in D, I, W, E (REQ-8)
- [ ] With the minimum level raised at build time, lower-level lines disappear from serial (REQ-9)
- [ ] One boot report shows version, PSRAM status and size, free heap and flash size (REQ-10)
- [ ] `pio test -e native` passes on the Mac with no Cardputer connected (REQ-14)
- [ ] Host tests cover formatting at all four levels, long messages, empty messages and long tags (REQ-15)
- [ ] CLAUDE.md lists the host test command (REQ-4)

## Edge Cases

- Display fails to initialize → firmware keeps running, logs an `E` line tagged `SYS`; serial logging still works (REQ-6 from the spec edge-case table)
- Message longer than 256 characters → line truncated at 256 characters and ends with `...` (REQ-8 from the spec edge-case table)
- Empty message → line still printed with level and tag (REQ-8 from the spec edge-case table)
- Tag longer than 6 characters → tag truncated to 6 characters (REQ-8 from the spec edge-case table)
- Minimum level set to Error at build time → only `E` lines printed, including no boot report if it is logged at Info (REQ-9 from the spec edge-case table)
- Board has no PSRAM → boot report shows `psram=none`, boot continues normally (REQ-10 from the spec edge-case table)
- PSRAM detected but fails to initialize → boot report shows `psram=error`, boot continues (REQ-10 from the spec edge-case table)
- Test code accidentally includes a hardware header → native build fails, `pio test -e native` reports failure (REQ-14 from the spec edge-case table)
