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
 * @file core2foraws_display.h
 * @brief Core2 for AWS IoT EduKit display hardware driver APIs
 */

#ifndef _CORE2FORAWS_DISPLAY_H_
#define _CORE2FORAWS_DISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_err.h>

#include "lvgl.h"

#ifdef CONFIG_SOFTWARE_WIFI_SUPPORT
#include "lv_qrcode.h"
#endif

/**
 * @brief FreeRTOS task handle for the LVGL guiTask.
 *
 * This is the task handle that can be used to control the display's
 * FreeRTOS task using one of the [FreeRTOS task control](https://www.freertos.org/a00112.html) functions.
 *
 * **Example:**
 *
 * Increase the task priority of the guiTask to improve the 
 * screen write and touch input performance.
 * @code{c}
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  
 *  #include "core2foraws.h"
 * 
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 * 
 *      UBaseType_t gui_task_priorty = uxTaskPriorityGet( core2foraws_display_task_handle );
 *      
 *      vTaskPrioritySet( core2foraws_display_task_handle, gui_task_priorty + 1 );
 *  }
 *  
 * @endcode
 */
/* @[declare_core2foraws_display_task_handle] */
extern TaskHandle_t core2foraws_display_task_handle;
/* @[declare_core2foraws_display_task_handle] */

/**
 * @brief Pointer to the registered LVGL display struct.
 *
 * This is the pointer to the LVGL display and can be used to directly
 * get or modify the display parameters at runtime.
 * 
 * For more details, visit the [LVGL docs](https://docs.lvgl.io/8.0/porting/display.html) 
 * regarding the display interface and other available APIs.
 *
 * **Example:**
 *
 * Create a LVGL label widget, set the text of the label to "Hello 
 * World!", and align the label to the center of the screen. After
 * 1 second, rotate the display 180 degrees.
 * @code{c}
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  
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
 * 
 *      vTaskDelay( pdMS_TO_TICKS( 1000 ) );
 * 
 *      xSemaphoreTake( core2foraws_common_spi_semaphore, portMAX_DELAY );
 *      lv_disp_set_rotation( core2foraws_display_ptr, LV_DISP_ROT_180 );
 *      xSemaphoreGive( core2foraws_common_spi_semaphore );
 *  }
 *  
 * @endcode
 *
 */
/* @[declare_core2foraws_display_ptr] */
extern lv_disp_t *core2foraws_display_ptr;
/* @[declare_core2foraws_display_ptr] */

/**
 * @brief Initializes the display controller and touch driver.
 * 
 * Wraps the [LVGL](https://lvgl.io/) and [LVGL_ESP32_Driver](https://github.com/lvgl/lvgl_esp32_drivers) 
 * libraries to initialize the ILI9342C display controller and
 * FT6336U touch driver on the SPI bus.
 * @note The core2foraws_init() calls this function
 * when the hardware feature is enabled.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_STATE : Library is unable to initialize
 *  - ESP_ERR_NO_MEM        : Out of memory
 */
/* @[declare_xore2foraws_display_init] */
esp_err_t core2foraws_display_init( void );
/* @[declare_xore2foraws_display_init] */

#ifdef __cplusplus
}
#endif
#endif