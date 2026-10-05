/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_sd.h
 * @brief Core2 for AWS IoT Kit SD card hardware driver APIs
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

/**
 * @brief Initializes and mounts the SD card.
 *
 * @note The SD card must be mounted before use. The SD card
 * and the screen share the same SPI bus, with each device 
 * having it's own CS line. Frequent writes to the SD card 
 * can slow screen writes. File I/O is split into bounded chunks so
 * display flushes can acquire the shared bus between chunks. To protect
 * access to the device, a mutex has been placed for each of SD
 * card APIs to make it thread-safe in the event there are
 * multiple tasks attempting to access the device at once.
 *
 * The example code below mounts the SD card on the mount point 
 * `/sdcard` and then writes a file named `kit.txt` with the 
 * contents "Hello from AWS IoT Kit!":
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
 *      char *file_path = "/kit.txt";
 *  
 *      esp_err_t err = core2foraws_sd_mount();
 *      if( err == ESP_OK )
 *      {
 *          ESP_LOGI( TAG, "\tMounted SD card" );
 *  
 *          size_t written_length = 0;
 *          err = core2foraws_sd_write( file_path, "Hello from AWS IoT Kit!", &written_length );
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
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK          : Success
 *  - ESP_ERR_TIMEOUT : Shared SPI bus or SD lock was not free in time
 *  - ESP_FAIL        : Failed to mount the SD card
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
 * `/sdcard` and then writes a file named `kit.txt` with the 
 * contents "Hello from AWS IoT Kit!":
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
 *      char *file_path = "/kit.txt";
 *  
 *      esp_err_t err = core2foraws_sd_mount();
 *      if( err == ESP_OK )
 *      {
 *          ESP_LOGI( TAG, "\tMounted SD card" );
 *  
 *          size_t written_length = 0;
 *          err = core2foraws_sd_write( file_path, "Hello from AWS IoT Kit!", &written_length );
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
 * @note With valid arguments, message is initialized to an empty string before
 * acquiring locks. Close failures are returned. If closing times out on SPI,
 * the BSP retains the file and retries its close before the next SD operation.
 *
 * @note @p message may be in PSRAM. Data passes through the C library and
 * FAT file-system buffers before reaching the SD driver, so a PSRAM buffer
 * reads at about the same speed as an internal one (measured on this board:
 * roughly 650 vs 730 KB/s for a 32 KB file).
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK          : Success
 *  - ESP_ERR_TIMEOUT : Shared SPI bus or SD lock was not free in time.
 *                      @p message holds any bytes read before the timeout.
 *  - ESP_FAIL        : Failed to read file from SD
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
 * `/sdcard` and then writes a file named `kit.txt` with the 
 * contents "Hello from AWS IoT Kit!":
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
 *      char *file_path = "/kit.txt";
 *  
 *      esp_err_t err = core2foraws_sd_mount();
 *      if( err == ESP_OK )
 *      {
 *          ESP_LOGI( TAG, "\tMounted SD card" );
 *  
 *          size_t written_length = 0;
 *          err = core2foraws_sd_write( file_path, "Hello from AWS IoT Kit!", &written_length );
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
 * @note ESP_OK requires a successful close, including any buffered flush.
 * wrote_length counts bytes accepted by stdio, not durable bytes after an error.
 * A SPI timeout during close retains the file for retry before the next SD
 * operation, including unmount; no additional file is opened in the meantime.
 *
 * @note @p message may be in PSRAM. Data passes through the C library and
 * FAT file-system buffers before reaching the SD driver, so a PSRAM buffer
 * writes at about the same speed as an internal one (measured on this board:
 * roughly 260 vs 240-270 KB/s for a 32 KB file).
 *
 * @note The SD card shares its SPI bus with the display, and the time the
 * card spends busy programming flash counts against the display's
 * `CORE2FORAWS_SPI_LOCK_TIMEOUT_MS` wait. The BSP releases the bus between
 * 4 KB chunks, but long or frequent writes can still delay or drop UI
 * frames; write in the background at a modest rate if the UI must stay
 * smooth.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK          : Success
 *  - ESP_ERR_TIMEOUT : Shared SPI bus or SD lock was not free in time.
 *                      @p wrote_length holds the bytes written before the
 *                      timeout.
 *  - ESP_FAIL        : Failed to write file to SD
 */
/* @[declare_core2foraws_sd_write] */
esp_err_t core2foraws_sd_write( const char *file_name, const char* message, size_t *wrote_length );
/* @[declare_core2foraws_sd_write] */

/**
 * @brief Removes the FAT partition and unmounts the SD-Card. Unmount 
 * before removing SD card from the device.
 *
 * @note The SD Card must be mounted before use. The SD card
 * and the screen use the same SPI bus. The BSP serializes unmount against
 * file operations and display DMA automatically; applications must not take
 * the shared SPI semaphore around this API.
 *
 * @note Every wait on the shared SPI bus is bounded by
 * @ref CORE2FORAWS_SPI_LOCK_TIMEOUT_MS, so a stalled display transfer returns
 * `ESP_ERR_TIMEOUT` rather than blocking the caller indefinitely.
 *
 * To learn more about using the SD card, visit Espressif's virtual
 * [file system component](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/storage/vfs.html)
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
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
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
