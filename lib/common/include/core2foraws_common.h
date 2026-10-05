/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_common.h
 * @brief Core2 for AWS IoT Kit helper library used across BSP drivers
 */

#ifndef _CORE2FORAWS_COMMON_H_
#define _CORE2FORAWS_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_err.h>

#include "core2foraws_i2c.h"

/**
 * @brief The port used by the internal I2C peripherals.
 */
/* @[declare_core2foraws_common_i2c_internal] */
#define COMMON_I2C_INTERNAL CORE2FORAWS_I2C_INTERNAL
/* @[declare_core2foraws_common_i2c_internal] */

/**
 * @brief The port used by the external I2C peripherals.
 */
/* @[declare_core2foraws_common_i2c_external] */
#define COMMON_I2C_EXTERNAL CORE2FORAWS_I2C_EXTERNAL
/* @[declare_core2foraws_common_i2c_external] */

/**
 * @brief The port used by the I2S peripherals (speaker & microphone).
 */
/* @[declare_core2foraws_common_i2s_internal] */
#define COMMON_I2S_INTERNAL 0
/* @[declare_core2foraws_common_i2s_internal] */

/**
 * @brief Converts a minimum delay in milliseconds to a `vTaskDelay()` tick
 * count that is guaranteed to wait at least that long.
 *
 * `pdMS_TO_TICKS()` rounds down and `vTaskDelay( n )` can return up to one
 * tick early, so at the default 100 Hz tick rate `pdMS_TO_TICKS( 1 )` is 0 and
 * `pdMS_TO_TICKS( 10 )` can end almost immediately. Use this for datasheet
 * minimum waits.
 */
/* @[declare_core2foraws_delay_ms_to_ticks] */
#define CORE2FORAWS_DELAY_MS_TO_TICKS( ms )                                    \
    ( ( TickType_t )( ( ( ( uint64_t )( ms ) * configTICK_RATE_HZ ) + 999U ) / \
                      1000U ) + 1U )
/* @[declare_core2foraws_delay_ms_to_ticks] */

/**
 * @brief Largest single transfer the shared SPI2 bus must support, in bytes.
 *
 * The LVGL draw buffer is the largest consumer of the shared bus, so this is
 * derived from `CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES` at 320 pixels per line
 * and 2 bytes per RGB565 pixel. When the display is not built, the SD card is
 * the only consumer and a 4 KB ceiling is sufficient.
 */
/* @[declare_core2foraws_common_spi_max_transfer_bytes] */
#ifdef CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES
#define CORE2FORAWS_SPI_MAX_TRANSFER_BYTES \
    ( 320 * CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES * 2 )
#else
#define CORE2FORAWS_SPI_MAX_TRANSFER_BYTES ( 4096 )
#endif
/* @[declare_core2foraws_common_spi_max_transfer_bytes] */

/**
 * @brief FreeRTOS binary semaphore used to serialize display and SD card SPI
 * transfers.
 * 
 * The display and SD card share a SPI bus.
 *
 * @note The BSP display and SD modules take this semaphore automatically.
 * Application code should use the LVGL port lock for LVGL object access and
 * only take this semaphore directly when adding another device or raw
 * transaction to the shared SPI bus. Never hold it across an LVGL call that
 * can trigger a refresh.
 */
/* @[declare_core2foraws_common_spi_semaphore] */
extern SemaphoreHandle_t core2foraws_common_spi_semaphore;
/* @[declare_core2foraws_common_spi_semaphore] */

/**
 * @brief Maximum time to wait for the shared display/SD SPI semaphore.
 *
 * A full-screen flush at 40 MHz takes roughly 25 ms, so this leaves ample
 * margin while still bounding every wait. Nothing waits on the shared SPI
 * semaphore indefinitely: a stalled transfer surfaces as a logged timeout
 * error instead of an unrecoverable hang.
 */
/* @[declare_core2foraws_spi_lock_timeout_ms] */
#define CORE2FORAWS_SPI_LOCK_TIMEOUT_MS 500U
/* @[declare_core2foraws_spi_lock_timeout_ms] */

/**
 * @brief Creates the shared display/SD SPI semaphore if needed.
 *
 * This function is idempotent. The semaphore is binary rather than a mutex so
 * an asynchronous LCD transfer can take it from the LVGL task and release it
 * from the SPI completion callback.
 *
 * @return
 *  - ESP_OK         : Success or already initialized
 *  - ESP_ERR_NO_MEM : Semaphore allocation failed
 */
/* @[declare_core2foraws_common_spi_semaphore_init] */
esp_err_t core2foraws_common_spi_semaphore_init( void );
/* @[declare_core2foraws_common_spi_semaphore_init] */

/**
 * @brief Initializes the board's shared SPI2 bus for the LCD and SD card.
 *
 * The bus has fixed board wiring and is initialized once for the BSP lifetime.
 * The function is thread-safe and idempotent, so display and SD can initialize
 * in either order.
 *
 * @return ESP_OK on success, otherwise an ESP-IDF SPI initialization error.
 */
/* @[declare_core2foraws_common_spi_bus_init] */
esp_err_t core2foraws_common_spi_bus_init( void );
/* @[declare_core2foraws_common_spi_bus_init] */

/**
 * @brief Function used to standardize error returns.
 * 
 * This is a helper function used to return any non-zero error codes as
 * ESP_FAIL, and zero value as ESP_OK.
 * 
 * @param[in] error_code The error code for a library.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_common_error] */
esp_err_t core2foraws_common_error( int32_t error_code );
/* @[declare_core2foraws_common_error] */

/**
 * @brief Report the minimum free stack ("high-water mark") of a task.
 *
 * Gets, and logs at info level, the smallest amount of unused stack the task
 * has ever had since it started, expressed in bytes. Use it to size
 * FreeRTOS task stacks: run your real workload, read the watermark, and keep
 * a safety margin above the peak usage (allocation minus reported free).
 *
 * The BSP creates two long-lived tasks whose stacks you may want to measure:
 * the virtual-button poll task (named `"buttonPress"`) and the LVGL
 * render/flush task (named `"taskLVGL"`). Look either up with
 * [`xTaskGetHandle()`](https://www.freertos.org/a00021.html#xTaskGetHandle)
 * or pass `NULL` to measure the calling task.
 *
 * **Example:**
 *
 * Log how much stack headroom the LVGL task still has.
 * @code{c}
 *  #include "core2foraws.h"
 *
 *  core2foraws_init();
 *  TaskHandle_t lvgl = xTaskGetHandle( "taskLVGL" );
 *  size_t free_stack_bytes;
 *  esp_err_t err = core2foraws_common_task_stack_watermark(
 *      "APP", lvgl, &free_stack_bytes );
 * @endcode
 *
 * @param[in] tag  Log tag to print under. If `NULL`, a default tag is used.
 * @param[in] task Handle of the task to inspect, or `NULL` for the caller.
 * @param[out] watermark_bytes Minimum free stack in bytes since the task
 * started.
 * @return
 *  - ESP_OK              : Success
 *  - ESP_ERR_INVALID_ARG : @p watermark_bytes is `NULL`
 */
/* @[declare_core2foraws_common_task_stack_watermark] */
esp_err_t core2foraws_common_task_stack_watermark( const char *tag,
                                                   TaskHandle_t task,
                                                   size_t *watermark_bytes );
/* @[declare_core2foraws_common_task_stack_watermark] */

/**
 * @brief Snapshot of the heap pools the BSP cares about.
 */
/* @[declare_core2foraws_common_heap_stats_t] */
typedef struct
{
    size_t internal_free;          /**< Current free internal DRAM. */
    size_t internal_minimum_free;  /**< Lowest free internal DRAM since boot. */
    size_t internal_largest_block; /**< Largest contiguous internal block. */
    size_t dma_free;               /**< Current free DMA-capable DRAM. */
    size_t dma_minimum_free;       /**< Lowest free DMA DRAM since boot. */
    size_t dma_largest_block;      /**< Largest contiguous DMA-capable block. */
    size_t spiram_free;            /**< Current free external PSRAM. */
    size_t spiram_minimum_free;    /**< Lowest free external PSRAM since boot. */
} core2foraws_common_heap_stats_t;
/* @[declare_core2foraws_common_heap_stats_t] */

/**
 * @brief Report free internal DRAM, DMA-capable DRAM, and PSRAM.
 *
 * Gets, and logs at info level, the current and minimum-ever free size plus the
 * largest contiguous block of each pool the BSP allocates from. Use it to
 * validate memory budgets after a change: the LVGL draw buffers, audio I2S,
 * shared-SPI, and SK6812 RMT buffers all require *contiguous* DMA-capable
 * internal DRAM, and contiguity fails before total free size does once Wi-Fi
 * and BLE are running.
 *
 * Call it after `core2foraws_init()` and again once the network is up, since
 * the Wi-Fi and BLE stacks are the largest internal-DRAM consumers.
 *
 * **Example:**
 *
 * Check DMA headroom after bring-up.
 * @code{c}
 *  #include "core2foraws.h"
 *
 *  core2foraws_init();
 *  core2foraws_common_heap_stats_t heap;
 *  core2foraws_common_heap_report( "APP", &heap );
 * @endcode
 *
 * @param[in] tag Log tag to print under. If `NULL`, a default tag is used.
 * @param[out] stats Receives the snapshot. May be `NULL` to log only.
 * @return
 *  - ESP_OK : Success
 */
/* @[declare_core2foraws_common_heap_report] */
esp_err_t core2foraws_common_heap_report(
    const char *tag, core2foraws_common_heap_stats_t *stats );
/* @[declare_core2foraws_common_heap_report] */

#ifdef __cplusplus
}
#endif
#endif
