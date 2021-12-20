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
 * @file core2foraws_display.c
 * @brief Core2 for AWS IoT EduKit display hardware driver APIs
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

#define LV_TICK_PERIOD_MS 1

SemaphoreHandle_t core2foraws_display_semaphore;
TaskHandle_t core2foraws_display_task_handle;

static const char *_s_TAG = "CORE2FORAWS_DISPLAY";

static void _s_lv_tick_task( void *arg );
static void _s_gui_task( void *pvParameter );

static void _s_lv_tick_task( void *arg )
{
    ( void ) arg;
    lv_tick_inc( LV_TICK_PERIOD_MS );
}

static void _s_gui_task( void *pvParameter )
{
    
    ( void ) pvParameter;

    while ( 1 )
    {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay( pdMS_TO_TICKS( 10 ) );

        /* Try to take the semaphore, call lvgl related function on success */
        if ( pdTRUE == xSemaphoreTake( core2foraws_display_semaphore, portMAX_DELAY ) ) 
		{
            lv_task_handler();
            xSemaphoreGive( core2foraws_display_semaphore );
        }
    }

    /* A task should NEVER return */
    vTaskDelete( NULL );
}

esp_err_t core2foraws_display_init( void ) 
{
    ESP_LOGI( _s_TAG, "\tInitializing" );

    lvgl_i2c_locking( i2c_manager_locking() );

	core2foraws_display_semaphore = xSemaphoreCreateMutex();

    xSemaphoreTake( core2foraws_display_semaphore, portMAX_DELAY );
    	
	lv_init();
	lvgl_driver_init();

	/* Use double buffered when not working with monochrome displays. 
	 * Application should allocate two buffers buf1 and buf2 of size
	 * (DISP_BUF_SIZE * sizeof(lv_color_t)) each
	 */

    static lv_disp_buf_t disp_buf;
    uint32_t size_in_px = DISP_BUF_SIZE;
    lv_color_t *buf1 = heap_caps_malloc( DISP_BUF_SIZE * sizeof( lv_color_t ), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT ); //Assuming max size of lv_color_t = 16bit, DISP_BUF_SIZE calculated from max horizontal display size 480
    lv_color_t *buf2 = heap_caps_malloc( DISP_BUF_SIZE * sizeof( lv_color_t ), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT ); //Assuming max size of lv_color_t = 16bit, DISP_BUF_SIZE calculated from max horizontal display size 480

	// Set up the frame buffers
	lv_disp_buf_init( &disp_buf, buf1, buf2, size_in_px );

	// Set up the display driver
	lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register( &disp_drv );

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
        .callback = &_s_lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    esp_err_t err;

	err = esp_timer_create( &periodic_timer_args, &periodic_timer );
	if ( err != ESP_OK )
	{
		ESP_LOGE( _s_TAG, "Error creating periodic ESP timer for LVGL" );
		return err;
	}

    err = esp_timer_start_periodic( periodic_timer, LV_TICK_PERIOD_MS * 1000 );
	if ( err != ESP_OK )
	{
		ESP_LOGE( _s_TAG, "Error starting periodic ESP timer for LVGL" );
		return err;
	}

	xSemaphoreGive( core2foraws_display_semaphore );

	/* If you want to use a task to create the graphic, you NEED to create a Pinned task
     * Otherwise there can be problem such as memory corruption and so on.
     * NOTE: If you're not using Wi-Fi or Bluetooth, you can pin the _s_gui_task to core 0 */
	xTaskCreatePinnedToCore( _s_gui_task, "gui", 4096*2, NULL, 4, &core2foraws_display_task_handle, 1 );

	return ESP_OK;
}