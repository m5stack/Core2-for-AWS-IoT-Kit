M5Stack Core2 ESP32 IoT Development Kit for AWS Features Demo
===

A demonstration application and BSP library for the M5Stack Core2 ESP32 IoT Development Kit for AWS IoT EduKit. For use with ESP-IDF and part of the AWS IoT EduKit program.

## Description

Only supports ESP-IDF v4.1 or v4.2. Prefer to use 4.2 due to improved mbedTLS performance and functionality.
`git clone -b release/v4.2 --recursive https://github.com/espressif/esp-idf.git`


## C Drivers Included
| Part | Description |
| --- | ----------- |
| AXP192 | Power Management Unit (PMU). Vibration motor, battery, and green LED connected to PMU. |
| ILI9342C | TFT Display (320px * 240px) using the [LVGL library](https://docs.lvgl.io/) |
| SK6812 | 10x i2c LED bar |
| NS4168 | Speaker amplifier (attached to 1w speaker) |
| BM8563 | Real Time Clock |
| FT6336U | Capacitive Touch |
| SPM1423 | PDM Microphone |
| ATECC608 | Microchip Trust&GO secure element |
| MPU6886 | 6-axis IMU w/ internal temperature sensor |
| - | SD Card (up to 16GB)* |

* Actual SD card compatibility will vary. Must be formatted FAT32 or FAT16 with 512byte sector size.