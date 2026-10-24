Pre-installed firmware for the M5Stack Core2 ESP32 IoT Development Kit for AWS
===

## Notes
Compatible with ESP-IDF v4.4 and PlatformIO with [Espressif Platform](https://github.com/platformio/platform-espressif32/releases) v5.4.0 (native on Apple Silicon; `platformio.ini` overrides the platform's x86-only `tool-cmake`). 

To use the ESP-IDF directly, follow their [get started guide](https://docs.espressif.com/projects/esp-idf/en/release-v4.4/esp32/get-started/index.html) install the ESP-IDF and export to path. To compile and upload the firmware to flash memory, use `idf.py build flash` from this directory in your locally cloned repository. 

To use PlatformIO, from this (Factory-Firmware) directory in your locally cloned repository, use the command `pio run -e core2foraws -t flash -t upload` to compile and upload the firmware.
