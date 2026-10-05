# BSP regression tests

## Host

From the BSP directory:

```sh
sh tests/host/run.sh
```

Requires a C11 compiler with AddressSanitizer and UndefinedBehaviorSanitizer.
The runner uses a temporary directory and removes its binaries on exit. Tests
compile the actual implementation files against small ESP-IDF/FreeRTOS mocks.
Mock headers are isolated in [host/mocks](host/mocks/README.md), never in the
production include paths.

- Startup: disabled BSP, successful ordering, failure in each automatic module,
  and early termination after mandatory I2C/PMU failures.
- I2C: uninitialized access, shared references, removal retry, stale handles,
  register framing, stack/heap payload paths, and deterministic close interleavings.
- Audio: 100 Hz RTOS versus millisecond I2S waits, partial transfers, invalid
  arguments, mode exclusion, lock timeouts, and failed startup/PMU shutdown/
  channel disable/delete retries without lost handles or premature pin resets.
- RGB: GRB framing, in-flight buffer ownership, and failed wait/disable/encoder
  delete/channel delete/startup rollback retries.
- Expansion: mock pin routing rejects reset while I2C/UART is active; GPIO-to-
  paired-bus begin, single-pin release, reopen, delete failures and UART setup
  retry. ADC/DAC stubs do not validate analog behavior or calibration.
- Motion: sample conversion holds ownership through output assignment, and
  range writes update the cache before unlock. Failed transfers preserve the
  cache; all four ranges are checked against fixed sample values. This checks
  software serialization, not sensor settling or independent calibration.
- BMI270: cold-start configuration upload through INIT_ADDR-addressed chunks,
  reuse of a configuration that survived an ESP32-only reset, rejected-file
  timeout, wrong CHIP_ID, little-endian scaling for every range, failed range
  writes, and the invalid-temperature marker. The mock checks the file bytes,
  not the IMU's internal processing.
- Board: MPU6886/BMI270 identification (including a BMI270-like value in the
  MPU6886 trim register), absent IMU, probe caching, and LCD reporting.
- SD: native temporary files, close/flush errors, close-time SPI timeouts, retry
  before unmount, and output initialization. No SD card is touched.
- RTC: independently specified epochs for 1900, 1970, leap day 2000, the 2038
  boundary, and 2099; extreme input years must not reach I2C.
- Power: forbidden direct MCU voltage/disable requests, including raw-register
  and masked-update entry points, produce no mocked I2C writes. Normal startup
  and peripheral controls still work. Normal shutdown sets only REG32H bit 7,
  leaving every other mocked register unchanged. Dangerous rail requests are
  tested only here; the host model does not simulate actual power loss/restart.
- First-use waits: audio, I2C, and SD mocks allow the initializer to finish only
  when the waiter blocks; spinning with `taskYIELD()` fails the regression.

These mocks do not reproduce real scheduling, DMA, interrupts, electrical faults,
or card durability. Host `long` is 64-bit on macOS; the RTC boundary tests alone
cannot reproduce ESP32's 32-bit intermediate overflow. The implementation must
retain its explicitly 64-bit day count, and the firmware build checks the target.

## Connected board

The extended smoke run also cycles RGB and paired Port A/Port C release/reopen
ten times and performs 100 concurrent motion-range updates while sampling. Port
tests do not require accessories or send UART payloads; host tests independently
verify routing order. IMU ranges return to their defaults and LEDs are cleared.

[hardware/main/smoke.c](hardware/main/smoke.c) is an alternative `app_main`, not a
production BSP source. The [standalone hardware application](hardware/README.md)
provides its own build files, defaults, and explicit opt-in instructions.
Building or running the host tests never flashes the device.

The smoke image enables the factory hardware modules. It checks repeated BSP init,
board detection (a known kit model and LCD controller),
sensor and secure-element serial reads, invalidated external-I2C handles, 20
audio mode cycles using silent playback, forced SPI contention, display teardown
timeout recovery, three display reinitializations, Wi-Fi reconnect or provisioning start, 1,000 shared-I2C
iterations, concurrent Wi-Fi lifecycle/getter calls, and heap integrity. It also
tests SD/NVS/display sharing and reversible peripheral-power controls. Only this
test deliberately holds SPI while
requesting a redraw, to exercise the timeout path; applications must not do so.

The final `BSP_TEST: RESULT PASS` refers to API assertions. Wi-Fi association is
reported separately and is not a pass condition. Sensor values are logged, not
calibrated against independent instruments. The test writes and removes
`/sd_card/bsp-review-test.txt`, verifies its 16 KiB payload after unmount/remount,
and commits/reads/cleans the `bsp_review` NVS namespace while another task requests
redraws. It does not erase unrelated NVS namespaces or change RTC time. Redraw
counts measure requests, not independently measured panel pixels or frame rate.

Power testing saves and restores backlight, LED, and vibration state and verifies
MCU rail readback is unchanged. The unattended smoke test never directly disables
or reconfigures DCDC1, requests software power-off, changes input-power/VOFF/boot settings, or cuts the active
LCD/SD logic supply. Peripheral power checks are permitted; making the MCU
unreachable is not. The BSP also enforces its MCU guard in code.

A separate supervised shutdown/restart test is allowed: finish storage cleanup,
call `core2foraws_power_off()`, press the physical power key, and verify a fresh
boot. Do not substitute `rail_state_set(POWER_RAIL_ESP32, false)` or assume the
USB serial reset toggles PEK. The default smoke run intentionally leaves the
board powered so the host can restore factory firmware without physical help.

Secure-element testing remains read-only: no slot allocation/writes, key
generation, provisioning, zone locks, or counter changes. Audible/optical output
quality and power-failure durability are not independently qualified by this test.

Before flashing, read the actual partition table and confirm the active app slot
from the boot log. Write only that slot, not bootloader, partition table, OTA
selection data, factory NVS, or application NVS. On the tested board it was
`0xA0000`; do not assume this for another board. Restore the factory image after
testing. Serial transfer was reliable at 115200 baud; 921600 failed in this run.