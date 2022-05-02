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
 * @file core2foraws_common.h
 * @brief Core2 for AWS IoT EduKit helper library used across BSP drivers
 */

#ifndef _CORE2FORAWS_COMMON_H_
#define _CORE2FORAWS_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <esp_err.h>

/**
 * @brief The port used by the internal I2C peripherals.
 */
/* @[declare_core2foraws_common_i2c_internal] */
#define COMMON_I2C_INTERNAL I2C_NUM_0
/* @[declare_core2foraws_common_i2c_internal] */

/**
 * @brief The port used by the external I2C peripherals.
 */
/* @[declare_core2foraws_common_i2c_external] */
#define COMMON_I2C_EXTERNAL I2C_NUM_1
/* @[declare_core2foraws_common_i2c_external] */

/**
 * @brief The port used by the I2S peripherals (speaker & microphone).
 */
/* @[declare_core2foraws_common_i2s_internal] */
#define COMMON_I2S_INTERNAL I2S_NUM_0
/* @[declare_core2foraws_common_i2s_internal] */

/**
 * @brief FreeRTOS semaphore to be used when performing any
 * operation on the display or SD card reader.
 * 
 * The display and SD card share a SPI bus.
 *
 * @note To avoid conflicts with multiple threads attempting to
 * write to the SPI bus, take this FreeRTOS semaphore first,
 * use the [LVGL API(s)](https://docs.lvgl.io/7.11/overview/index.html)
 * or SD APIs of choice, and then give the semaphore.
 *
 * **Example:**
 *
 * Create a LVGL label widget, set the text of the label to "Hello 
 * World!", and align the label to the center of the screen.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include "core2foraws.h"
 * 
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 * 
 *      xSemaphoreTake( core2foraws_common_spi_semaphore, portMAX_DELAY );
 *
 *      lv_obj_t * hello_label = lv_label_create( NULL, NULL );
 *      lv_label_set_text_static( hello_label, "Hello World!" );
 *      lv_obj_align( hello_label, NULL, LV_ALIGN_CENTER, 0, 0 );
 *
 *      xSemaphoreGive( core2foraws_common_spi_semaphore );
 *  }
 *  
 * @endcode
 */
/* @[declare_core2foraws_common_spi_semaphore] */
extern SemaphoreHandle_t core2foraws_common_spi_semaphore;
/* @[declare_core2foraws_common_spi_semaphore] */

/**
 * @brief Function used to standardize error returns.
 * 
 * This is a helper function used to return any non-zero error codes as
 * ESP_FAIL, and zero value as ESP_OK.
 * 
 * @param[in] error_code The error code for a library.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_common_error] */
esp_err_t core2foraws_common_error( int8_t error_code );
/* @[declare_core2foraws_common_error] */

#ifdef __cplusplus
}
#endif
#endif
