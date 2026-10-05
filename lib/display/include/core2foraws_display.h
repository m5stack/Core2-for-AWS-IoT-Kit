/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_display.h
 * @brief Core2 for AWS IoT Kit display hardware driver APIs
 */

#ifndef _CORE2FORAWS_DISPLAY_H_
#define _CORE2FORAWS_DISPLAY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <esp_err.h>
#include <esp_lcd_touch.h>
#include <esp_lvgl_port.h>
#include <stdint.h>

#include "lvgl.h"

  /**
   * @brief Pointer to the registered LVGL display struct.
   */
  /* @[declare_core2foraws_display_ptr] */
  extern lv_display_t *core2foraws_display_ptr;
  /* @[declare_core2foraws_display_ptr] */

  /**
     * @brief Gets the esp_lcd_touch driver handle for the FT6336U.
   *
   * Can be used by other subsystems (e.g. virtual buttons) to read raw
   * touch coordinates from the capacitive touch panel.
  *
  * @deprecated Use core2foraws_display_touch_data_get() so access is
  * serialized with all other internal-I2C peripherals.
     *
     * @param[out] touch_handle Receives the touch driver handle.
     * @return
     *  - ESP_OK                : Success
     *  - ESP_ERR_INVALID_ARG   : @p touch_handle is `NULL`
     *  - ESP_ERR_INVALID_STATE : The touch driver is not initialized
   */
  /* @[declare_core2foraws_display_touch_handle] */
    esp_err_t core2foraws_display_get_touch_handle(
      esp_lcd_touch_handle_t *touch_handle );
  /* @[declare_core2foraws_display_touch_handle] */

  /**
   * @brief Reads one or more FT6336 touch points under the shared internal-I2C
   * lock.
   *
   * Use this function instead of calling `esp_lcd_touch_read_data()` directly
   * so touch, PMU, RTC, IMU, and secure-element transactions remain serialized.
   *
   * The LVGL input device and the virtual buttons share one sample: a reading
   * less than 15 ms old is returned without another I2C transfer.
   *
   * @param[out] points Touch-point output array.
   * @param[out] point_count Number of reported points.
   * @param[in] max_points Capacity of @p points.
   * @return ESP_OK on success or an argument, state, lock, or I2C error.
   */
  /* @[declare_core2foraws_display_touch_data_get] */
  esp_err_t core2foraws_display_touch_data_get(
      esp_lcd_touch_point_data_t *points, uint8_t *point_count,
      uint8_t max_points );
  /* @[declare_core2foraws_display_touch_data_get] */

  /**
   * @brief Initializes the display controller and touch driver.
   *
   * Uses esp_lcd to drive the ILI9342C or ILI9342E display via SPI, esp_lcd_touch for
   * the FT6336U capacitive touch controller over I2C, and esp_lvgl_port to
   * integrate both with the LVGL graphics library. The LCD controller is
   * read back before its init sequence is chosen; core2foraws_board_info_get()
   * reports the result.
   *
   * @note The core2foraws_init() calls this function
   * when the hardware feature is enabled.
   *
   * @note The two LVGL draw buffers are sized by
   * `CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES` and must be allocated as
   * contiguous, DMA-capable internal DRAM. If that allocation cannot be met,
   * initialization fails with `ESP_ERR_NO_MEM` and logs the required and
   * available sizes; lower that option or free internal DRAM.
  * A BSP-owned RGB565 flush callback handles byte swapping and SPI ownership.
  * SPI timeouts skip and complete the flush without issuing a panel transfer;
  * the skipped region is repainted on its next application invalidation.
   *
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_STATE : Library is unable to initialize
   *  - ESP_ERR_NO_MEM        : Insufficient DMA-capable DRAM for the draw
   *                            buffers, or another allocation failed
   */
  /* @[declare_core2foraws_display_init] */
  esp_err_t core2foraws_display_init( void );
  /* @[declare_core2foraws_display_init] */

  /**
   * @brief Stops LVGL and releases the display, touch, and panel-I/O resources.
   *
   * The shared SPI bus remains initialized because the SD card may still use
  * it. Touch teardown takes the internal-I2C lock so an LVGL or button-task
  * read cannot overlap removal of the FT6336 panel-I/O device. This function
  * is idempotent. A SPI-barrier timeout retains all resources and leaves
  * refresh paused. Release the competing SPI operation, then retry deinit or
  * call lvgl_port_resume() and invalidate the screen to resume rendering.
   *
  * @return ESP_OK on success, ESP_ERR_TIMEOUT on a SPI-barrier timeout, or
  * the internal-I2C lock/device-removal error.
   */
  /* @[declare_core2foraws_display_deinit] */
  esp_err_t core2foraws_display_deinit( void );
  /* @[declare_core2foraws_display_deinit] */

#ifdef __cplusplus
}
#endif
#endif