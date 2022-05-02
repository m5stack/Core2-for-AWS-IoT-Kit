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
 * @file core2foraws_button.c
 * @brief Core2 for AWS IoT EduKit virtual button hardware driver APIs
 */

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "core2foraws_common.h"
#include "core2foraws_button.h"

#include "lvgl_touch/ft6x36.h"

static SemaphoreHandle_t _button_mutex;

static const char *_TAG = "CORE2FORAWS_BUTTON";

struct 
{
    const uint16_t x;                   /**< @brief Virtual button touch starting point in the X-coordinate plane. */
    const uint16_t y;                   /**< @brief Virtual button touch starting point in the Y-coordinate plane */
    const uint16_t w;                   /**< @brief Virtual button touched width from the starting point in the X-coordinate plane. */
    const uint16_t h;                   /**< @brief Virtual button touched height from the starting point in the y-coordinate plane. */
    bool is_touched;                    /**< @brief Current virtual button touched state */
    bool last_touched;                  /**< @brief Previous virtual button touched state */
    uint32_t last_press_time;           /**< @brief FreeRTOS ticks when virtual button was last touched */
    uint32_t long_press_time;           /**< @brief Number of FreeRTOS ticks to elapse to consider holding the touch button a long press */
    press_event_t state;                /**< @brief The button press event */
    enum core2foraws_button_btns id;    /**< @brief The id of the button from the enumerated list */
} static _touch_buttons[] = 
{
    { 10, 241, 86, 38, false, false, 0, 0, 0, BUTTON_LEFT },
    { 117, 241, 86, 38, false, false, 0, 0, 0, BUTTON_MIDDLE },
    { 224, 241, 86, 38, false, false, 0, 0, 0, BUTTON_RIGHT }
};

#if CONFIG_LV_FT6X36_COORDINATES_QUEUE
static void button_press_task( void *pvParameters )
{
    ft6x36_touch_t touch_received;
    for ( ;; )
    {
        xQueueReceive( ft6x36_touch_queue_handle, &touch_received, 0 );
        for ( uint8_t i = 0; i < sizeof( _touch_buttons ) / sizeof ( _touch_buttons[ 0 ] ); i++ )
        {
            if ( xSemaphoreTake(_button_mutex, portMAX_DELAY) == pdPASS )
            {
                bool touched = ( touch_received.current_state == LV_INDEV_STATE_PR ) & 
                                !((touch_received.last_x < _touch_buttons[ i ].x) || 
                                (touch_received.last_x > (_touch_buttons[ i ].x + _touch_buttons[ i ].w)) || 
                                (touch_received.last_y < _touch_buttons[ i ].y) || 
                                (touch_received.last_y > (_touch_buttons[ i ].y + _touch_buttons[ i ].h)));
                ESP_LOGD( _TAG, "Touch button id=%i, touched=%d", i, touched );

                uint32_t now_ticks = xTaskGetTickCount();
                if ( touched != _touch_buttons[ i ].last_touched )
                {
                    if ( touched == 1 )
                    {
                        _touch_buttons[ i ].state |= PRESS;
                        _touch_buttons[ i ].last_press_time = now_ticks;
                    }
                    else
                    {
                        if ( _touch_buttons[ i ].long_press_time && ( now_ticks - _touch_buttons[ i ].last_press_time > _touch_buttons[ i ].long_press_time ) )
                        {
                            _touch_buttons[ i ].state |= LONGPRESS;
                        }
                        else
                        {
                            _touch_buttons[ i ].state |= RELEASE;
                        }
                    }
                    _touch_buttons[ i ].last_touched = touched;
                }
                _touch_buttons[ i ].last_touched = touched;
                _touch_buttons[ i ].is_touched = touched;

                xSemaphoreGive(_button_mutex);   
            }
        }
    }

    vTaskDelay( pdMS_TO_TICKS( 30 ) );
}
#endif

esp_err_t core2foraws_button_tapped( enum core2foraws_button_btns button, bool *state )
{
    BaseType_t err;
    err = xSemaphoreTake(_button_mutex, portMAX_DELAY);
    
    if ( err == pdPASS )
    {
        *state = ( _touch_buttons[ button ].state & PRESS ) > 0;
        _touch_buttons[ button ].state &= ~PRESS;
        xSemaphoreGive( _button_mutex );
    }
    
    return core2foraws_common_error( err );
}

esp_err_t core2foraws_button_pressing( enum core2foraws_button_btns button, bool *state )
{
    BaseType_t err;
    err = xSemaphoreTake(_button_mutex, portMAX_DELAY);
    
    if ( err == pdPASS )
    {
        *state = _touch_buttons[ button ].is_touched;
        xSemaphoreGive( _button_mutex );
    }

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_button_held( enum core2foraws_button_btns button, bool *state )
{
    BaseType_t err;
    err = xSemaphoreTake(_button_mutex, portMAX_DELAY);
    
    if ( err == pdPASS )
    {
    uint32_t ticks = pdMS_TO_TICKS( LV_INDEV_DEF_LONG_PRESS_TIME );
    
        _touch_buttons[ button ].long_press_time = ticks;
        *state = ( _touch_buttons[ button ].state & LONGPRESS ) > 0;
        _touch_buttons[ button ].state &= ~LONGPRESS;
        xSemaphoreGive( _button_mutex );
    }
    
    return core2foraws_common_error( err );
}

esp_err_t core2foraws_button_released( enum core2foraws_button_btns button, bool *state )
{
    BaseType_t err;
    err = xSemaphoreTake(_button_mutex, portMAX_DELAY);
    
    if ( err == pdPASS )
    {
        *state = (_touch_buttons[ button ].state & RELEASE) > 0;
        _touch_buttons[ button ].state &= ~RELEASE;
        xSemaphoreGive(_button_mutex);
    }

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_button_init( void )
{
    ESP_LOGI( _TAG, "\tInitializing" );
    BaseType_t err = pdFAIL;
#if CONFIG_LV_FT6X36_COORDINATES_QUEUE
    _button_mutex = xSemaphoreCreateMutex();

    if ( _button_mutex != NULL )
    {
        err = xTaskCreatePinnedToCore( button_press_task, "buttonPress", configMINIMAL_STACK_SIZE * 3, NULL, 0, ( TaskHandle_t * ) NULL, 1 );
    }
#else
    ESP_LOGE( _TAG, "Must enable CONFIG_LV_FT6X36_COORDINATES_QUEUE using menuconfig to use this driver" );
#endif
    return core2foraws_common_error( !err );
}