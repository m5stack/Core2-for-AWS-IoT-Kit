# Hardware smoke application

This is a standalone test consumer of the BSP, not part of its production source
list. It requires the Core2 for AWS board, its AWS bottom board, and a writable
SD test card. The complete test coverage and limits are in [../README.md](../README.md).

## Build explicitly

From the BSP root, using PlatformIO Core on PATH:

```sh
CORE2FORAWS_HARDWARE_TESTS=1 pio run -d tests/hardware -e hardware-tests
```

The acknowledgement is required during CMake configuration because the image
writes SD/NVS test data and changes peripheral state. It is not a security
boundary and does not make flashing safe by itself. Building does not flash or
execute the tests. Normal BSP consumers never compile this application's main.

The test app pins Espressif32 7.1.3 / ESP-IDF 6.1.0 and uses its own
[sdkconfig.defaults](sdkconfig.defaults). It finds the BSP through
`EXTRA_COMPONENT_DIRS`; no temporary factory-project edits are needed.
Managed downloads, generated configurations, and build output are ignored by Git.
The manifest pins direct dependencies; regenerate the ignored local dependency
lock when deliberately updating them.

For an ESP-IDF installation with its tools exported:

```sh
cd tests/hardware
CORE2FORAWS_HARDWARE_TESTS=1 idf.py set-target esp32
CORE2FORAWS_HARDWARE_TESTS=1 idf.py build
```

Historical hardware measurements used the consuming factory project's
configuration. The packaged test defaults are separately build-validated;
do not assume its heap figures or timings are identical until measured.

## Flash deliberately

Do not use a blanket `pio run -t upload` or `idf.py flash` on a provisioned board:
those commands can also write bootloader, partition table, and OTA-selection data.
First read the board's partition table and confirm its boot-selected application
slot. [partitions.csv](partitions.csv) matches the tested factory layout; it is
not permission to overwrite another device's layout.

Write only the test application's binary to the verified active app slot with
the appropriate flashing tool. On the previously tested board that slot was
`0xA0000`, and reliable serial transfer used 115200 baud. Keep a built normal
factory image available, and restore it after collecting the test result.

Expected completion is `BSP_TEST: RESULT PASS`. Wi-Fi association is reported
separately, not included in that PASS condition. The test writes/removes
`/sd_card/bsp-review-test.txt` and clears only its `bsp_review` NVS namespace.
Use a test card: an existing file with that name will be overwritten.

## Safety limits

- Never allocate/write secure-element slots, generate keys, lock zones, or
  change counters. This application only reads the secure element.
- Never disable/reconfigure the MCU rail or startup defaults. Normal software
  shutdown remains a supported BSP API, but this unattended app leaves power on.
- Backlight, LED, vibration, RGB, I2C/UART, audio, and motion state are exercised.
  Expansion tests do not transmit UART payloads or require accessories.
- Stop on failure and inspect the first error; do not weaken guards to get PASS.

See [the design safety policy](../../docs/design.md#61-mcu-supply-and-recovery-safeguards)
before extending these tests.