/*
 * Core2 for AWS IoT EduKit BSP v2.0.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://aws.amazon.com/iot/edukit
 *
 */

/**
 * @file core2foraws_sd.h
 * @brief Core2 for AWS IoT EduKit SD card hardware driver APIs
 */

#ifndef _CORE2FORAWS_SD_H_
#define _CORE2FORAWS_SD_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_freertos_hooks.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <esp_err.h>

#ifndef SD_ACCESS_DELAY_MS
/**
 * @brief Delay time added to SD card operations
 */
/* @[declare_sd_access_delay_ms] */
#define SD_ACCESS_DELAY_MS 100U
/* @[declare_sd_access_delay_ms] */
#endif

/**
 * @brief Initializes and mounts the SD card.
 *
 * @note The SD card must be mounted before use. The SD card
 * and the screen share the same SPI bus, with each device 
 * having it's own CS line. Frequent writes to the SD card 
 * can cause slow down of the screen writes. In order to protect
 * access to the device, a mutex has been placed for each of SD
 * card APIs to make it thread-safe in the event there are
 * multiple tasks attempting to access the device at once.
 *
 * The example code below mounts the SD card on the mount point 
 * `/sdcard` and then writes a file named `edukit.txt` with the 
 * contents "Hello from AWS IoT EduKit!":
 *
 * **Example:**
 * @code{c}
 *  #include <stdint.h>
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_SD_DEMO";
 * 
 *  void app_main( void )
 *  {
 *      char *file_path = "/edukit.txt";
 *  
 *      esp_err_t err = core2foraws_sd_mount();
 *      if( err == ESP_OK )
 *      {
 *          ESP_LOGI( TAG, "\tMounted SD card" ); *  
 *          size_t written_length = 0;
 *          err = core2foraws_sd_write( file_path, "Hello from AWS IoT EduKit!", &length );
 *          ESP_LOGI( TAG, "\tSD write %s, wrote %d bytes", err == ESP_OK ? "success" : "fail", written_length );
 *  
 *          char str[64];
 *          err = core2foraws_sd_read( file_path, str, 64 );
 *          ESP_LOGI( TAG, "\tSD read %s,\nString: %s", err == ESP_OK ? "success" : "fail", str );
 *  
 *          err = core2foraws_sd_unmount();
 *          ESP_LOGI( TAG, "\tSD unmount %s", err == ESP_OK ? "success" : "fail" );
 *      }
 *      else
 *      {
 *          ESP_LOGI( TAG, "\tFailed to mount SD" );
 *      }
 *  }
 * @endcode
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK      : Success
 *  - ESP_FAIL    : Failed to mount the SD card
 */
/* @[declare_core2foraws_sd_mount] */
esp_err_t core2foraws_sd_mount( void );
/* @[declare_core2foraws_sd_mount] */

/**
 * @brief Reads a specified length of characters from the specificied file 
 * on the FAT file system SD card.
 *
 * @note The SD card must be mounted before use and formatted with a 
 * FAT/FAT32 file system. The `to_read_length` can be higher than the 
 * total number of characters in the file. The function will return 
 * once the EOF character is read.
 *
 * The example code below mounts the SD card on the mount point 
 * `/sdcard` and then writes a file named `edukit.txt` with the 
 * contents "Hello from AWS IoT EduKit!":
 *
 * **Example:**
 * @code{c}
 *  #include <stdint.h>
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_SD_DEMO";
 * 
 *  void app_main( void )
 *  {
 *      char *file_path = "/edukit.txt";
 *  
 *      esp_err_t err = core2foraws_sd_mount();
 *      if( err == ESP_OK )
 *      {
 *          ESP_LOGI( TAG, "\tMounted SD card" );
 *  
 *          size_t written_length = 0;
 *          err = core2foraws_sd_write( file_path, "Hello from AWS IoT EduKit!", &length );
 *          ESP_LOGI( TAG, "\tSD write %s, wrote %d bytes", err == ESP_OK ? "success" : "fail", written_length );
 *  
 *          char str[64];
 *          err = core2foraws_sd_read( file_path, str, 64 );
 *          ESP_LOGI( TAG, "\tSD read %s,\nString: %s", err == ESP_OK ? "success" : "fail", str );
 *  
 *          err = core2foraws_sd_unmount();
 *          ESP_LOGI( TAG, "\tSD unmount %s", err == ESP_OK ? "success" : "fail" );
 *      }
 *      else
 *      {
 *          ESP_LOGI( TAG, "\tFailed to mount SD" );
 *      }
 *  }
 * @endcode
 *
 * @param[in] file_name The pointer to the file name.
 * @param[out] message The string read and copied from the file on the SD card.
 * @param[in] to_read_length The number of characters to read from the file.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK      : Success
 *  - ESP_FAIL    : Failed to read file from SD
 */
/* @[declare_core2foraws_sd_read] */
esp_err_t core2foraws_sd_read( const char *file_name, char *message, size_t to_read_length );
/* @[declare_core2foraws_sd_read] */

/**
 * @brief Writes a specified string to the specificied file on the SD card.
 *
 * @note The SD card must be mounted before use and formatted with a 
 * FAT/FAT32 file system.
 *
 * The example code below mounts the SD card on the mount point 
 * `/sdcard` and then writes a file named `edukit.txt` with the 
 * contents "Hello from AWS IoT EduKit!":
 *
 * **Example:**
 * @code{c}
 *  #include <stdint.h>
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_SD_DEMO";
 * 
 *  void app_main( void )
 *  {
 *      char *file_path = "/edukit.txt";
 *  
 *      esp_err_t err = core2foraws_sd_mount();
 *      if( err == ESP_OK )
 *      {
 *          ESP_LOGI( TAG, "\tMounted SD card" );
 *  
 *          size_t written_length = 0;
 *          err = core2foraws_sd_write( file_path, "Hello from AWS IoT EduKit!", &written_length );
 *          ESP_LOGI( TAG, "\tSD write %s, wrote %d bytes", err == ESP_OK ? "success" : "fail", written_length );
 *  
 *          char str[64];
 *          err = core2foraws_sd_read( file_path, str, 64 );
 *          ESP_LOGI( TAG, "\tSD read %s,\nString: %s", err == ESP_OK ? "success" : "fail", str );
 *  
 *          err = core2foraws_sd_unmount();
 *          ESP_LOGI( TAG, "\tSD unmount %s", err == ESP_OK ? "success" : "fail" );
 *      }
 *      else
 *      {
 *          ESP_LOGI( TAG, "\tFailed to mount SD" );
 *      }
 *  }
 * @endcode
 *
 * @param[in] file_name The pointer to the file name.
 * @param[in] message The pointer to the string to write to the file on the SD card.
 * @param[out] wrote_length The pointer to the number of characters that was written to the file.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK      : Success
 *  - ESP_FAIL    : Failed to write to SD card
 */
/* @[declare_core2foraws_sd_write] */
esp_err_t core2foraws_sd_write( const char *file_name, const char* message, size_t *wrote_length );
/* @[declare_core2foraws_sd_write] */

/**
 * @brief Removes the FAT partition and unmounts the SD-Card. Unmount 
 * before removing SD card from the device.
 *
 * @note The SD Card must be mounted before use. The SD card
 * and the screen uses the same SPI bus. In order to avoid
 * conflicts with the screen, you must take the spi_mutex
 * semaphore, then call spi_poll() before accessing the SD card.
 * Once done, give/free the semaphore so the display can take it.
 * The display task will then continue to update the display
 * controller on the SPI bus.
 *
 * To learn more about using the SD card, visit Espressif's virtual
 * [file system component](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/storage/vfs.html)
 * docs for usage.
 *
 * The example code below, mounts the SD card, writes a file named
 * `hello.txt` with the contents "Hello!", and then unmounts the card:
 *
 * **Example:**
 * @code{c}
 *  #include <stdint.h>
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_SD_DEMO";
 * 
 *  void app_main( void )
 *  {
 *      esp_err_t err = core2foraws_sd_mount();
 * 
 *      if( err == ESP_OK )
 *      {
 *          ESP_LOGI( TAG, "\tMounted SD card" );
 * 
 *          size_t written_length = 0;
 *          err = core2foraws_sd_write( file_path, "Hello!", &written_length );
 *
 *          err = core2foraws_sd_unmount();
 *          ESP_LOGI( TAG, "\tUnmounted SD card" );
 *      }
 *      else
 *      {
 *          ESP_LOGI( TAG, "\tFailed to mount SD" );
 *      }
 *  }
 * @endcode
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_STATE : Failed to unmount. Must call @ref core2foraws_sd_mount first
 */
/* @[declare_core2foraws_sdcard_unmount] */
esp_err_t core2foraws_sd_unmount( void );
/* @[declare_core2foraws_sdcard_unmount] */

#ifdef __cplusplus
}
#endif
#endif
