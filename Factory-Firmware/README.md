Pre-installed firmware for the M5Stack Core2 ESP32 IoT Development Kit for AWS
===

## Notes
Compatible with ESP-IDF v4.2 and PlatformIO with [Espressif Platform](https://github.com/platformio/platform-espressif32/releases) v3.0.0-3.2.0. 

To use the ESP-IDF directly, follow their [get started guide](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/get-started/index.html) install the ESP-IDF and export to path. To compile and upload the firmware to flash memory, use `idf.py build flash` from this directory in your locally cloned repository. 

To use PlatformIO, from this (Factory-Firmware) directory in your locally cloned repository, use the command `pio run -e core2foraws -t flash -t upload` to compile and upload the firmware.
