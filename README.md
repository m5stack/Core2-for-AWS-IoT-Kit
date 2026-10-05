# M5Stack Core2 for AWS IoT Kit Board Support Package (BSP)

Version 3.0.0. Licensed under MIT with attribution to Rashed Talukder (2022) and M5Stack (2022); see [LICENSE](LICENSE) and [NOTICE](NOTICE). Upstream attribution is retained in [THIRD-PARTY-LICENSES.txt](THIRD-PARTY-LICENSES.txt). The Bosch BMI270 configuration file remains BSD-3-Clause, and datasheets and managed dependencies retain their own terms.
Generate the API and engineering reference with `doxygen Doxyfile` from this directory; open `generated-docs/html/index.html` to browse it.

Doxygen 1.13.2 is validated for this reference. On macOS, Homebrew Doxygen
1.18.0 can crash intermittently in its Markdown scanner. An isolated tool
environment avoids replacing the system installation:

```sh
conda create --yes --prefix "$HOME/.local/share/core2-doxygen" -c conda-forge doxygen=1.13.2
conda run --prefix "$HOME/.local/share/core2-doxygen" doxygen Doxyfile
```

This repository contains the drivers specific to the [M5Stack Core2 for AWS IoT Kit](https://m5stack.com/products/m5stack-core2-esp32-iot-development-kit-for-aws-iot-kit) reference Hardware. This BSP is used in the microcontroller tutorials presented in the [AWS IoT Kit](https://aws-iot-kit-docs.m5stack.com) program.

One build supports both kit models. The [Core2 for AWS v1.3](https://docs.m5stack.com/en/products/sku/K010-AWS-V13) replaces the MPU6886 IMU with a BMI270 and the SPM1423 microphone with an LMD4737T261; units of either model built from 2026-08-07 use an ILI9342E instead of an ILI9342C LCD controller. The BSP detects the IMU and LCD controller at run time; call `core2foraws_board_info_get()` for the model and part names.

## Repository layout

```text
core2foraws.c             BSP initialization entry point
include/                 Public umbrella header
lib/<module>/            Module implementation, public headers, chip datasheets
datasheet/               Board schematics and wiring schema
docs/                    Architecture and engineering documentation
tests/host/              Native regression cases and runner
tests/host/mocks/include/ Host-only SDK declarations
tests/hardware/          Explicitly opted-in standalone ESP-IDF test application
.claude/rules/           Agent maintenance policies
```

Start with [the design](docs/design.md), [hardware wiring](datasheet/schema.yml),
or [test instructions](tests/README.md). Production source and public include
paths remain stable. Tests and mock headers are never compiled into ordinary
BSP consumers; generated build/dependency/configuration files stay untracked.

## Cloning

To clone using HTTPS:

```shell
git clone -b BSP-dev https://github.com/m5stack/Core2-for-AWS-IoT-Kit.git
```

Using SSH:

```shell
git clone -b BSP-dev git@github.com:m5stack/Core2-for-AWS-IoT-Kit.git
```

**Windows users:** If the repository or any project consuming it contains symbolic links, set `core.symlinks true` (`git config --global core.symlinks true`) and either enable [Developer Mode](https://docs.microsoft.com/en-us/windows/apps/get-started/enable-your-device-for-development) or run git commands from an elevated console.

## Usage

It is recommended to use the [project template](https://github.com/m5stack/Project_Template-Core2_for_AWS) instead of the BSP directly. The project template contains the application configuration and managed dependencies required by the Core2 for AWS IoT Kit.

This repository is an ESP-IDF component, not a standalone application. Current validation uses [PlatformIO](https://github.com/platformio/platform-espressif32) `espressif32` 7.1.3 with ESP-IDF 6.1.0. Earlier ESP-IDF 5.3/6.0 validations predate the latest dependency update and have not been repeated. This component does not contain its own `platformio.ini`. Follow the [AWS IoT Kit — Getting Started](https://aws-iot-kit-docs.m5stack.com/en/getting-started/kit) tutorial for environment setup.

`core2foraws_init()` requires internal I2C and PMU initialization to succeed before attempting downstream peripherals. A failure in either returns its original error immediately. Later enabled automatic modules are attempted and failures are aggregated as `ESP_FAIL`. Applications that need module-specific recovery can initialize those modules independently. Wi-Fi initialization returns NVS and network errors without erasing the default NVS partition; `core2foraws_wifi_reset()` clears only persistent Wi-Fi configuration.

The master `SOFTWARE_BSP_SUPPORT` Kconfig option controls the common layer and all hardware modules. When it is disabled, common APIs are not compiled or exposed and `core2foraws_init()` is a successful no-op.

The internal I2C bus is a permanent, recursively locked board resource. The ATECC608 wake token, including its expected address-zero NACK, is handled through that shared bus rather than by taking direct GPIO ownership. External Port A I2C can host multiple managed devices and can be closed/reopened independently. Display and SD share one common-owned SPI2 bus; SD transfers are chunked so large file operations do not monopolize display refresh.

API safety notes for this revision:

- Failed audio/RGB teardown retains owned resources: retry disable/deinit before re-enabling or sending data. This also covers failed initialization rollback.
- Resetting or reassigning one active I2C/UART expansion pin releases the full pair. Re-register Port A devices after reopening; old handles are invalid.
- Motion range/cache changes are serialized with scaled reads. This prevents mixed software scale factors, not physical sensor settling after range changes.
- On the v1.3, the first `core2foraws_init()` after power-on takes about 0.26 s longer while the BMI270 configuration file is uploaded. ESP32-only resets keep the loaded file.
- Direct MCU-rail changes are protected: DCDC1 cannot be disabled or set to anything other than 3350 mV, including through BSP raw PMU helpers. Normal whole-device shutdown remains available through `core2foraws_power_off()` (AXP192 REG32H bit 7), preserving rail/startup configuration for power-key restart. Stop workers, close SD, and commit NVS first. Direct I2C outside the power module is not covered; see [docs/design.md](docs/design.md), section 6.1.
- Wi-Fi is owned by one BSP state machine; applications must not call `esp_wifi_connect()`, `esp_wifi_disconnect()`, `esp_wifi_set_config()`, or `esp_wifi_stop()` directly. All lifecycle, scan, connect, and provisioning calls share one lock. First-use mutex waiters block briefly instead of starving lower-priority initialization tasks.
- Tests may write the dedicated SD file and NVS namespace, but must not modify secure-element slots, keys, locks, counters, or MCU rail/startup configuration. A supervised normal off/on test is permitted; making the MCU unreachable on restart is not.
- Use `core2foraws_display_touch_data_get()` instead of reading the raw touch handle; it participates in internal-I2C serialization.
- Physical touch reads are interrupt-gated and use a BSP-owned 100 ms transport timeout, preventing an unresponsive controller from monopolizing the shared internal-I2C bus.
- ATECC608 signatures are fixed 64-byte raw P-256 `R || S` values; verification rejects encoded or differently sized signatures with `ESP_ERR_INVALID_SIZE`.
- `core2foraws_expports_uart_read()` requires the destination buffer capacity before the output byte count.
- Audio lock waits and I2S transfers each use `AUDIO_IO_TIMEOUT_MS`, with the correct units for each API, and serialize against speaker/microphone disable.
- `core2foraws_display_deinit()` retains resources on a SPI-barrier timeout; otherwise it releases display, touch, and LVGL resources while leaving shared SPI2 available to SD.
- Display flushes skip panel transfers on SPI lock timeout, rather than running unsynchronized. Re-invalidate the affected screen after contention clears.
- SD close errors are returned; a SPI-blocked close is retained and retried before another SD operation.
- Guard LVGL object access with `lvgl_port_lock()`, **not** `core2foraws_common_spi_semaphore`. The SPI semaphore is held across the display's DMA transfer; taking it around an LVGL call that can trigger a refresh will deadlock.
- Every wait on the shared SPI bus is bounded by `CORE2FORAWS_SPI_LOCK_TIMEOUT_MS` (500 ms), so SD APIs can return `ESP_ERR_TIMEOUT` if a display transfer stalls.

## Internal DRAM budget

Test coverage and remaining limitations are in [tests/README.md](tests/README.md).
Historical performance figures below were not re-established by the September
2026 smoke test.

The board has 8 MB of PSRAM but only a small internal DRAM pool, and PSRAM is **not** DMA-addressable. Every DMA-driven BSP buffer — the LVGL draw buffers, audio I2S, shared SPI2, and the SK6812 RMT buffer — must live in internal DRAM. PSRAM is never a fallback for these.

The largest single consumer is the pair of LVGL draw buffers, sized by `CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES`:

| Lines | Per buffer | Total (double-buffered) |
| --- | --- | --- |
| 10 | 6,400 B | 12,800 B |
| 20 (default) | 12,800 B | 25,600 B |
| 40 | 25,600 B | 51,200 B |

Each buffer needs a single **contiguous** DMA-capable block. Contiguity, not total free heap, is what fails once Wi-Fi and BLE are running, so a build that boots fine on the bench can still fail to bring up the display in the field. `core2foraws_display_init()` checks this before allocating and logs the required versus available sizes, returning `ESP_ERR_NO_MEM` rather than failing silently.

Call `core2foraws_common_heap_report()` after `core2foraws_init()` and again once the network is up to measure current free memory, the minimum-ever free watermark, and the largest contiguous block. Validate any change to the draw buffer height against those numbers.

The 20-line default was validated on hardware with 72 tests spanning display, audio, storage durability, fault injection, connectivity, concurrency, memory pressure, and soak. Compared with 40 lines it reclaimed about 25 KB of internal DMA memory, while the 60-flush stress test moved from 949 ms to 983 ms.

Measure with the radios up. With the Wi-Fi and BLE stacks brought up, the low-water marks were 85,887 bytes of internal DRAM and 80,479 bytes of DMA-capable DRAM, against a 110,592-byte largest contiguous block. The same suite without the radios reports roughly 128 KB and 100 KB — comfortable numbers that hide the real budget, which is why a bench measurement is not evidence that the display will come up in the field.

If you need more internal DRAM, apply these in your application's `sdkconfig` (a component cannot set them for you), roughly in order of payoff:

| Setting | Effect |
| --- | --- |
| `CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP=y` | Moves Wi-Fi and LWIP buffers to PSRAM. Usually the largest win (~62 KB internal DRAM freed in testing). |
| `CONFIG_LV_USE_CLIB_MALLOC=y` | LVGL's heap uses the C-library allocator so it can spill to PSRAM, instead of reserving a fixed 64 KB pool in internal DRAM up-front. The DMA draw buffers still stay internal, so display throughput is unaffected. |
| `CONFIG_CORE2FORAWS_WIFI_RELEASE_BLE_WHEN_PROVISIONED=y` | Frees the Bluetooth controller's reserved DRAM once Wi-Fi connects or a provisioning session ends. BLE and re-provisioning are then unavailable until reboot. |
| `CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=4096` | Routes general heap allocations ≥ 4 KB to PSRAM by default (lower than the 16 KB IDF default). |
| `CONFIG_MBEDTLS_DYNAMIC_BUFFER=y` | Releases the large TLS handshake buffers after each handshake completes, lowering peak internal usage. |
| `CONFIG_ESP_WIFI_STATIC_TX_BUFFER_NUM` (lower it cautiously) | Each static TX buffer costs ~1.6 KB of internal DRAM. IDF 6 requires static TX while `CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP=y`; dynamic TX cannot be combined with that larger PSRAM saving. |
| `CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM` (lower it) | Each static RX buffer costs ~1.6 KB of internal DRAM. |
| `CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES` (lower it) | Last resort — costs display throughput. |

On an ESP32-D0WDQ6-V3 with the display initialized, enabling `CONFIG_LV_USE_CLIB_MALLOC` together with `CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=4096` raised the largest **contiguous** DMA-capable block from ~28 KB to ~86 KB (roughly 3×) — enough headroom for the draw buffers to survive display re-init while Wi-Fi and BLE are running.

When provisioning actually runs, the provisioning manager's `FREE_BTDM` scheme handler already releases Bluetooth memory once provisioning ends; no application action is needed for that path.

We also have code examples, drivers, or content available in other frameworks:

- [Arduino](https://github.com/m5stack/aws-iot-kit-examples/tree/main/Basic_Arduino)
- [UIFlow](https://docs.m5stack.com/en/quick_start/core2_for_aws/uiflow)
- [MicroPython](https://github.com/m5stack/Core2forAWS-MicroPython)

## Support

To get support with AWS IoT Kit, post your question in the [content repo's discussions](https://github.com/m5stack/aws-iot-kit-tutorials/discussions).
For issues with the AWS IoT Kit this repo, please [submit an issue](https://github.com/m5stack/Core2-for-AWS-IoT-Kit/issues) to this repository.
