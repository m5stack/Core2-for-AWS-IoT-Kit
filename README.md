# M5Stack Core2 for AWS IoT Kit BSP

An ESP-IDF board support package (BSP) for the [M5Stack Core2 for AWS IoT Kit](https://shop.m5stack.com/products/m5stack-core2-for-aws-kit-v1-3). It wraps the kit's display, touch, power management, audio, motion sensor, RGB LEDs, RTC, microSD, secure element, expansion ports, and Wi-Fi behind a single `core2foraws_init()` call, and it is the BSP used by the [AWS IoT Kit](https://aws-iot-kit-docs.m5stack.com) tutorials.

## Supported hardware

One firmware image runs on all hardware revisions. The BSP detects the IMU and LCD controller at run time; call `core2foraws_board_info_get()` to get the model and part names.

| Model | IMU | Microphone | LCD controller |
| --- | --- | --- | --- |
| Core2 for AWS | MPU6886 | SPM1423 | ILI9342C, or ILI9342E on units built from 2026-08-07 |
| [Core2 for AWS v1.3](https://docs.m5stack.com/en/products/sku/K010-AWS-V13) | BMI270 | LMD4737T261 | ILI9342C, or ILI9342E on units built from 2026-08-07 |

## Modules

| Module | Hardware |
| --- | --- |
| `audio` | NS4168 speaker amplifier and PDM microphone |
| `board` | Model and part detection |
| `button` | Virtual touch buttons on the FT6336 touch panel |
| `common` | Shared internal/external I2C and SPI2 buses, locks, heap reporting |
| `crypto` | ATECC608 secure element |
| `display` | ILI9342C/E LCD with LVGL 9 and touch input |
| `expports` | Port A (I2C), Port B (GPIO/ADC/DAC), Port C (UART) |
| `motion` | MPU6886 or BMI270 IMU |
| `power` | AXP192 PMU: battery, backlight, vibration motor, speaker enable |
| `rgb_led` | 10-pixel SK6812 LED bar |
| `rtc` | BM8563 real-time clock |
| `sd` | microSD card |
| `wifi` | Wi-Fi station and BLE provisioning |

Each module can be turned on or off in `menuconfig` under **Core2 for AWS hardware features**.

## Requirements

- ESP-IDF 6.1.0 (validated). PlatformIO users can build with `espressif32` 7.1.3 from the consuming application.
- The IDF Component Manager, which resolves the dependencies in [idf_component.yml](idf_component.yml) (LVGL, esp_lvgl_port, esp-cryptoauthlib, network_provisioning, and others).

Follow [AWS IoT Kit — Getting Started](https://aws-iot-kit-docs.m5stack.com/en/getting-started/kit) to set up your environment.

## Getting started

The easiest way to start is the [project template](https://github.com/m5stack/Project_Template-Core2_for_AWS), which already includes this BSP and the application configuration it needs.

To add the BSP to an existing ESP-IDF project, add it as a component:

```shell
git submodule add -b BSP-dev https://github.com/m5stack/Core2-for-AWS-IoT-Kit.git components/Core2-for-AWS-IoT-Kit
```

Then initialize the board from your application:

```c
#include "core2foraws.h"

void app_main( void )
{
    ESP_ERROR_CHECK( core2foraws_init() );
}
```

`core2foraws_init()` brings up the internal I2C bus and the PMU first and returns immediately if either fails. It then starts the other enabled modules and returns `ESP_FAIL` if any of them fail. Audio, SD, expansion-port sessions, and Wi-Fi are started explicitly by the application.

> **Windows:** if your project uses symbolic links, run `git config --global core.symlinks true` and either enable [Developer Mode](https://docs.microsoft.com/en-us/windows/apps/get-started/enable-your-device-for-development) or run git from an elevated console.

## Usage notes

- **LVGL:** guard LVGL calls with `lvgl_port_lock()`, not `core2foraws_common_spi_semaphore`. The SPI semaphore is held during display DMA, so taking it around an LVGL call that triggers a refresh will deadlock.
- **Shared SPI:** the display and SD card share SPI2. Every wait is bounded by `CORE2FORAWS_SPI_LOCK_TIMEOUT_MS` (500 ms), so SD calls can return `ESP_ERR_TIMEOUT` while the display is busy.
- **Touch:** read touch with `core2foraws_display_touch_data_get()` rather than the raw touch handle, so reads are serialized on the internal I2C bus.
- **Wi-Fi:** the BSP owns the Wi-Fi state machine. Do not call `esp_wifi_connect()`, `esp_wifi_disconnect()`, `esp_wifi_set_config()`, or `esp_wifi_stop()` directly. `core2foraws_wifi_forget()` clears only the stored Wi-Fi configuration.
- **Audio:** the speaker and microphone share GPIO0, so only one can be active at a time.
- **Expansion ports:** resetting or reassigning either pin of an active I2C or UART pair releases the whole peripheral. Re-register Port A devices after reopening the bus.
- **Power:** the ESP32 rail (DCDC1) cannot be disabled or changed through the BSP. Use `core2foraws_power_off()` for a normal shutdown, after stopping workers, closing the SD card, and committing NVS.
- **Secure element:** ATECC608 signatures are raw 64-byte P-256 `R || S` values.
- **Teardown:** if a deinit or disable call fails, the BSP keeps its resources. Retry the call before re-enabling the module.
- **Startup time:** on the v1.3, the first `core2foraws_init()` after power-on takes about 0.26 s longer while the BMI270 configuration is uploaded.

See the [design document](docs/design.md) for the full ownership, locking, and power rules.

## Internal DRAM budget

The board has 8 MB of PSRAM, but PSRAM is not DMA-capable. The LVGL draw buffers, audio I2S, shared SPI2, and SK6812 RMT buffers must stay in internal DRAM, which is limited.

The largest consumer is the pair of LVGL draw buffers, sized by `CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES`:

| Lines | Per buffer | Total (double-buffered) |
| --- | --- | --- |
| 10 | 6,400 B | 12,800 B |
| 20 (default) | 12,800 B | 25,600 B |
| 40 | 25,600 B | 51,200 B |

Each buffer needs one contiguous DMA-capable block. That usually becomes the limit once Wi-Fi and BLE are running, even when total free heap looks fine. `core2foraws_display_init()` checks this up front and returns `ESP_ERR_NO_MEM` with the required and available sizes.

Call `core2foraws_common_heap_report()` after `core2foraws_init()` and again once the network is up to see free memory, the low-water mark, and the largest contiguous block. Measure with the radios running before changing the draw buffer height.

To free internal DRAM, set these in your application's `sdkconfig`, roughly in order of impact:

| Setting | Effect |
| --- | --- |
| `CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP=y` | Moves Wi-Fi and LWIP buffers to PSRAM (about 62 KB freed in testing). |
| `CONFIG_LV_USE_CLIB_MALLOC=y` | Lets LVGL's heap use PSRAM instead of a fixed 64 KB internal pool. |
| `CONFIG_CORE2FORAWS_WIFI_RELEASE_BLE_WHEN_PROVISIONED=y` | Releases Bluetooth controller memory after Wi-Fi connects. BLE and re-provisioning are unavailable until reboot. |
| `CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=4096` | Sends allocations of 4 KB or more to PSRAM. |
| `CONFIG_MBEDTLS_DYNAMIC_BUFFER=y` | Frees TLS handshake buffers after each handshake. |
| `CONFIG_ESP_WIFI_STATIC_TX_BUFFER_NUM` / `CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM` | Lower cautiously; each buffer costs about 1.6 KB. |
| `CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES` | Lower as a last resort; reduces display throughput. |

## Testing

Run the host regression tests from this directory:

```sh
sh tests/host/run.sh
```

On-device tests are in a separate opt-in ESP-IDF app. See [tests/README.md](tests/README.md) for details.

## Documentation

- [Design document](docs/design.md): architecture, ownership, power, and memory rules
- [Hardware schema](datasheet/schema.yml) and [schematic reference](datasheet/schematic.md): board wiring
- Chip datasheets live in each module's `datasheet/` folder under `lib/`

To build the API reference, run Doxygen from this directory and open `generated-docs/html/index.html`:

```sh
doxygen Doxyfile
```

Doxygen 1.13.2 is the validated version. Homebrew Doxygen 1.18.0 on macOS can crash intermittently; you can install 1.13.2 in an isolated environment instead:

```sh
conda create --yes --prefix "$HOME/.local/share/core2-doxygen" -c conda-forge doxygen=1.13.2
conda run --prefix "$HOME/.local/share/core2-doxygen" doxygen Doxyfile
```

## Other frameworks

- [Arduino](https://github.com/m5stack/aws-iot-kit-examples/tree/main/Basic_Arduino)
- [UIFlow](https://docs.m5stack.com/en/quick_start/core2_for_aws/uiflow)
- [MicroPython](https://github.com/m5stack/Core2forAWS-MicroPython)

## Support

Ask questions in the [AWS IoT Kit tutorial discussions](https://github.com/m5stack/aws-iot-kit-tutorials/discussions). Report BSP bugs in this repository's [issues](https://github.com/m5stack/Core2-for-AWS-IoT-Kit/issues).

## License

MIT. See [LICENSE](LICENSE) and [NOTICE](NOTICE). The Bosch BMI270 configuration file is BSD-3-Clause; datasheets and managed dependencies keep their own terms. Third-party attribution is in [THIRD-PARTY-LICENSES.txt](THIRD-PARTY-LICENSES.txt).
