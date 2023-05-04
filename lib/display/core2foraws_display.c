/*
 * Core2 for AWS IoT Kit BSP v2.0.0
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
 */

/**
 * @file core2foraws_display.c
 * @brief Core2 for AWS IoT Kit display hardware driver APIs
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <esp_freertos_hooks.h>
#include <esp_system.h>
#include <driver/gpio.h>

#include "i2c_manager.h"
#include "lvgl_helpers.h"

#include "core2foraws_display.h"
#include "core2foraws_common.h"

#define LV_TICK_PERIOD_MS 1

SemaphoreHandle_t core2foraws_common_spi_semaphore;
TaskHandle_t core2foraws_display_task_handle;
lv_disp_t *core2foraws_display_ptr;

static const char *_TAG = "CORE2FORAWS_DISPLAY";
static lv_color_t *_buf1 = NULL;
static lv_color_t *_buf2 = NULL;

static void _lv_tick_task( void *arg );
static void _gui_task( void *pvParameter );

static void _lv_tick_task( void *arg )
{
    ( void ) arg;
    lv_tick_inc( LV_TICK_PERIOD_MS );
}

static void _gui_task( void *pvParameter )
{
    
    ( void ) pvParameter;

    while ( 1 )
    {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay( pdMS_TO_TICKS( 10 ) );

        /* Try to take the semaphore, call lvgl related function on success */
        if ( pdTRUE == xSemaphoreTake( core2foraws_common_spi_semaphore, pdMS_TO_TICKS( CONFIG_ESP_TASK_WDT_TIMEOUT_S * 1000 - 1000 ) ) ) 
		{
            uint32_t ms_to_next_call = lv_task_handler();
            ESP_LOGV( _TAG, "%dms until next LVGL timer call.", ms_to_next_call );
            xSemaphoreGive( core2foraws_common_spi_semaphore );
        }
    }

    /* A task should NEVER return */
    free( _buf1 );
    free( _buf2 );
    vTaskDelete( NULL );
}

esp_err_t core2foraws_display_init( void ) 
{
    ESP_LOGI( _TAG, "\tInitializing" );

    esp_timer_handle_t periodic_timer;
    esp_err_t err;

    if( core2foraws_common_spi_semaphore == NULL )
	    core2foraws_common_spi_semaphore = xSemaphoreCreateMutex();

    lvgl_i2c_locking( i2c_manager_locking() );

    xSemaphoreTake( core2foraws_common_spi_semaphore, pdMS_TO_TICKS( 80 ) );
    	
	lv_init();

	/* Initialize the needed peripherals */
    lvgl_interface_init();

    /* Initialize needed GPIOs, e.g. backlight, reset GPIOs */
    lvgl_display_gpios_init();

	/* Use double buffered when not working with monochrome displays. 
	 * Application should allocate two buffers buf1 and buf2 of size
	 * (DISP_BUF_SIZE * sizeof(lv_color_t)) each
	 */

    static lv_disp_buf_t disp_buf;
    size_t display_buffer_size = lvgl_get_display_buffer_size();
    _buf1 = heap_caps_malloc( display_buffer_size * sizeof( lv_color_t ), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT ); //Assuming max size of lv_color_t = 16bit, display buffer size calculated from max horizontal display size 480
    _buf2 = heap_caps_malloc( display_buffer_size * sizeof( lv_color_t ), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT ); //Assuming max size of lv_color_t = 16bit, display buffer size calculated from max horizontal display size 480
    assert( _buf1 != NULL );
    assert( _buf2 != NULL );
    
	// Set up the frame buffers
    uint32_t size_in_px = display_buffer_size;
	lv_disp_buf_init( &disp_buf, _buf1, _buf2, size_in_px );

	// Set up the display driver
	lv_disp_drv_t disp_drv;
    ili9341_init( &disp_drv );
    lv_disp_drv_init( &disp_drv );
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.buffer = &disp_buf;
    core2foraws_display_ptr = lv_disp_drv_register( &disp_drv );

    /* Register an input device when enabled on the menuconfig */
#if CONFIG_LV_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register( &indev_drv );
#endif

	/* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &_lv_tick_task,
        .name = "periodic_gui"
    };

	err = esp_timer_create( &periodic_timer_args, &periodic_timer );
	if ( err != ESP_OK )
	{
		ESP_LOGE( _TAG, "Error creating periodic ESP timer for LVGL" );
		return err;
	}

    err = esp_timer_start_periodic( periodic_timer, LV_TICK_PERIOD_MS * 1000 );
	if ( err != ESP_OK )
	{
		ESP_LOGE( _TAG, "Error starting periodic ESP timer for LVGL" );
		return err;
	}

	xSemaphoreGive( core2foraws_common_spi_semaphore );

	/* If you want to use a task to create the graphic, you NEED to create a Pinned task
     * Otherwise there can be problem such as memory corruption and so on.
     * NOTE: If you're not using Wi-Fi or Bluetooth, you can pin the _gui_task to core 0 */
	xTaskCreatePinnedToCore( _gui_task, "gui", 4096*2, NULL, 4, &core2foraws_display_task_handle, 1 );

	return ESP_OK;
}