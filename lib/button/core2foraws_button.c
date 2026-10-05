/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_button.c
 * @brief Core2 for AWS IoT Kit virtual button hardware driver APIs
 */

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <sdkconfig.h>
#include <freertos/task.h>
#include <stdint.h>

#include "core2foraws_button.h"
#include "core2foraws_common.h"
#include "core2foraws_display.h"

#ifndef CONFIG_CORE2FORAWS_BUTTON_DEBOUNCE_MS
#define CONFIG_CORE2FORAWS_BUTTON_DEBOUNCE_MS 30
#endif

#define TOUCH_BUTTON_COUNT                                                      \
  ( sizeof( _touch_buttons ) / sizeof( _touch_buttons[ 0 ] ) )
#define BUTTON_DEBOUNCE_TICKS                                                   \
  pdMS_TO_TICKS( CONFIG_CORE2FORAWS_BUTTON_DEBOUNCE_MS )

static SemaphoreHandle_t _button_mutex;
static TaskHandle_t _button_task_handle = NULL;

static const char *_TAG = "CORE2FORAWS_BUTTON";

typedef struct
{
  button_event_cb_t press_cb;
  button_event_cb_t release_cb;
  button_event_cb_t longpress_cb;
} button_callbacks_t;

struct
{
  const uint16_t x;  /**< @brief Virtual button touch starting point in the
                        X-coordinate plane. */
  const uint16_t y;  /**< @brief Virtual button touch starting point in the
                        Y-coordinate plane */
  const uint16_t w;  /**< @brief Virtual button touched width from the starting
                        point in the X-coordinate plane. */
  const uint16_t h;  /**< @brief Virtual button touched height from the starting
                        point in the y-coordinate plane. */
  bool is_touched;   /**< @brief Current debounced touched state */
  bool last_touched; /**< @brief Most recent raw touched state */
  bool longpress_reported; /**< @brief True after long press callback fires */
  uint32_t debounce_start_time; /**< @brief FreeRTOS ticks when the raw touch
                                   state last changed */
  uint32_t last_press_time; /**< @brief FreeRTOS ticks when virtual button was
                               last touched */
  uint32_t long_press_time; /**< @brief Number of FreeRTOS ticks to elapse to
                               consider holding the touch button a long press */
  press_event_t state;      /**< @brief The button press event */
  enum core2foraws_button_btns
      id; /**< @brief The id of the button from the enumerated list */
  button_callbacks_t
      callbacks; /**< @brief Callback functions for button events */
} static _touch_buttons[] = { { 10,
                                241,
                                86,
                                38,
                                false,
                                false,
                                false,
                                0,
                                0,
                                0,
                                0,
                                BUTTON_LEFT,
                                { NULL, NULL, NULL } },
                              { 117,
                                241,
                                86,
                                38,
                                false,
                                false,
                                false,
                                0,
                                0,
                                0,
                                0,
                                BUTTON_MIDDLE,
                                { NULL, NULL, NULL } },
                              { 224,
                                241,
                                86,
                                38,
                                false,
                                false,
                                false,
                                0,
                                0,
                                0,
                                0,
                                BUTTON_RIGHT,
                                { NULL, NULL, NULL } } };

#define BUTTON_POLL_INTERVAL_MS 20
#define BUTTON_TASK_STACK_SIZE  4096

/* The FT6336U capacitive controller reports a maximum of two concurrent
 * touch points (datasheet section 14.1). */
#define BUTTON_TOUCH_POINTS 2

static bool _touch_in_button_region( uint16_t x, uint16_t y,
                                     uint8_t button_index )
{
  return x >= _touch_buttons[ button_index ].x &&
         x < ( _touch_buttons[ button_index ].x +
               _touch_buttons[ button_index ].w ) &&
         y >= _touch_buttons[ button_index ].y &&
         y < ( _touch_buttons[ button_index ].y +
               _touch_buttons[ button_index ].h );
}

static void _dispatch_button_callback( button_event_cb_t callback,
                                       enum core2foraws_button_btns button,
                                       press_event_t event )
{
  if( callback != NULL )
  {
    callback( button, event );
  }
}

static bool _debounce_elapsed( uint32_t now_ticks,
                               uint32_t start_ticks )
{
  return BUTTON_DEBOUNCE_TICKS == 0 ||
         ( now_ticks - start_ticks ) >= BUTTON_DEBOUNCE_TICKS;
}

static void button_press_task( void *pvParameters )
{
  (void)pvParameters;

  for( ;; )
  {
    vTaskDelay( pdMS_TO_TICKS( BUTTON_POLL_INTERVAL_MS ) );

    /* The FT6336U reports up to two simultaneous touch points (datasheet
     * section 14.1). Reading both lets the user press two of the separate
     * virtual buttons at the same time. If the controller reports fewer
     * points, the unused entries are simply ignored. */
    uint8_t  touch_cnt = 0;
    esp_lcd_touch_point_data_t point_data[ BUTTON_TOUCH_POINTS ];

    esp_err_t touch_err = core2foraws_display_touch_data_get(
        point_data, &touch_cnt, BUTTON_TOUCH_POINTS );
    if( touch_err != ESP_OK )
    {
      continue;
    }

    if( touch_cnt > BUTTON_TOUCH_POINTS )
    {
      touch_cnt = BUTTON_TOUCH_POINTS;
    }

    button_event_cb_t press_callbacks[ TOUCH_BUTTON_COUNT ] = { NULL };
    button_event_cb_t release_callbacks[ TOUCH_BUTTON_COUNT ] = { NULL };
    button_event_cb_t longpress_callbacks[ TOUCH_BUTTON_COUNT ] = { NULL };

    if( xSemaphoreTake( _button_mutex, portMAX_DELAY ) != pdPASS )
    {
      continue;
    }

    uint32_t now_ticks = xTaskGetTickCount();

    for( uint8_t i = 0; i < TOUCH_BUTTON_COUNT; i++ )
    {
      bool raw_touched = false;
      for( uint8_t p = 0; p < touch_cnt; p++ )
      {
        if( _touch_in_button_region( point_data[ p ].x, point_data[ p ].y,
                                     i ) )
        {
          raw_touched = true;
          break;
        }
      }

      if( raw_touched != _touch_buttons[ i ].last_touched )
      {
        _touch_buttons[ i ].last_touched = raw_touched;
        _touch_buttons[ i ].debounce_start_time = now_ticks;
      }

      if( raw_touched != _touch_buttons[ i ].is_touched &&
          _debounce_elapsed( now_ticks,
                             _touch_buttons[ i ].debounce_start_time ) )
      {
        _touch_buttons[ i ].is_touched = raw_touched;

        if( raw_touched )
        {
          _touch_buttons[ i ].last_press_time = now_ticks;
          _touch_buttons[ i ].longpress_reported = false;
          press_callbacks[ i ] = _touch_buttons[ i ].callbacks.press_cb;
        }
        else
        {
          release_callbacks[ i ] = _touch_buttons[ i ].callbacks.release_cb;
          _touch_buttons[ i ].longpress_reported = false;
        }
      }

      if( _touch_buttons[ i ].is_touched &&
          !_touch_buttons[ i ].longpress_reported &&
          _touch_buttons[ i ].long_press_time > 0 &&
          ( now_ticks - _touch_buttons[ i ].last_press_time ) >=
              _touch_buttons[ i ].long_press_time )
      {
        _touch_buttons[ i ].longpress_reported = true;
        longpress_callbacks[ i ] = _touch_buttons[ i ].callbacks.longpress_cb;
      }
    }

    xSemaphoreGive( _button_mutex );

    for( uint8_t i = 0; i < TOUCH_BUTTON_COUNT; i++ )
    {
      _dispatch_button_callback( press_callbacks[ i ], _touch_buttons[ i ].id,
                                 PRESS );
      _dispatch_button_callback( longpress_callbacks[ i ],
                                 _touch_buttons[ i ].id, LONGPRESS );
      _dispatch_button_callback( release_callbacks[ i ],
                                 _touch_buttons[ i ].id, RELEASE );
    }

    /* Report this task's stack headroom once, after the first full poll,
     * so its footprint is visible in the logs for right-sizing without
     * instrumenting the application. */
    static bool _watermark_logged = false;
    if( !_watermark_logged )
    {
      _watermark_logged = true;
      size_t watermark_bytes;
      core2foraws_common_task_stack_watermark( _TAG, NULL,
                                                &watermark_bytes );
    }
  }
}

esp_err_t
core2foraws_button_register_callback( enum core2foraws_button_btns button,
                                      press_event_t events,
                                      button_event_cb_t callback )
{
  if( button < BUTTON_LEFT || button > BUTTON_RIGHT || callback == NULL ||
      events == 0 || ( events & ~( PRESS | RELEASE | LONGPRESS ) ) != 0 )
  {
    ESP_LOGE( _TAG, "Invalid button (%d) or callback is NULL", button );
    return ESP_ERR_INVALID_ARG;
  }

  if( _button_mutex == NULL )
  {
    ESP_LOGE( _TAG, "Button system not initialized - mutex is NULL" );
    return ESP_FAIL;
  }

  BaseType_t err = xSemaphoreTake(
      _button_mutex,
      pdMS_TO_TICKS( 1000 ) ); // Use timeout instead of waiting forever

  if( err == pdPASS )
  {
    if( events & PRESS )
    {
      _touch_buttons[ button ].callbacks.press_cb = callback;
      ESP_LOGD( _TAG, "Registered PRESS callback for button %d", button );
    }
    if( events & RELEASE )
    {
      _touch_buttons[ button ].callbacks.release_cb = callback;
      ESP_LOGD( _TAG, "Registered RELEASE callback for button %d", button );
    }
    if( events & LONGPRESS )
    {
      _touch_buttons[ button ].callbacks.longpress_cb = callback;
      // Set long press time if not already set
      if( _touch_buttons[ button ].long_press_time == 0 )
      {
        _touch_buttons[ button ].long_press_time =
            pdMS_TO_TICKS( 400 ); /* LVGL default long press time ms */
      }
      ESP_LOGD( _TAG, "Registered LONGPRESS callback for button %d", button );
    }
    xSemaphoreGive( _button_mutex );
    ESP_LOGI( _TAG,
              "Successfully registered callback for button %d, events: 0x%02X",
              button, events );
    return ESP_OK; // Return ESP_OK on success
  }
  else
  {
    ESP_LOGE( _TAG, "Failed to take button mutex for callback registration" );
    return ESP_FAIL; // Return ESP_FAIL on failure
  }
}

esp_err_t
core2foraws_button_unregister_callback( enum core2foraws_button_btns button,
                                        press_event_t events )
{
  if( button < BUTTON_LEFT || button > BUTTON_RIGHT || events == 0 ||
      ( events & ~( PRESS | RELEASE | LONGPRESS ) ) != 0 )
  {
    ESP_LOGE( _TAG, "Invalid button (%d)", button );
    return ESP_ERR_INVALID_ARG;
  }

  if( _button_mutex == NULL )
  {
    ESP_LOGE( _TAG, "Button system not initialized - mutex is NULL" );
    return ESP_FAIL;
  }

  BaseType_t err = xSemaphoreTake( _button_mutex, pdMS_TO_TICKS( 1000 ) );

  if( err == pdPASS )
  {
    if( events & PRESS )
    {
      _touch_buttons[ button ].callbacks.press_cb = NULL;
    }
    if( events & RELEASE )
    {
      _touch_buttons[ button ].callbacks.release_cb = NULL;
    }
    if( events & LONGPRESS )
    {
      _touch_buttons[ button ].callbacks.longpress_cb = NULL;
      _touch_buttons[ button ].long_press_time = 0;
    }
    xSemaphoreGive( _button_mutex );
    return ESP_OK;
  }

  ESP_LOGE( _TAG, "Failed to take button mutex for callback unregistration" );
  return ESP_FAIL;
}

esp_err_t core2foraws_button_init( void )
{
  ESP_LOGI( _TAG, "\tInitializing" );

  if( _button_mutex != NULL && _button_task_handle != NULL )
  {
    ESP_LOGD( _TAG, "Button system is already initialized" );
    return ESP_OK;
  }

  if( _button_mutex != NULL || _button_task_handle != NULL )
  {
    ESP_LOGE( _TAG, "Button system is only partially initialized" );
    return ESP_ERR_INVALID_STATE;
  }

  BaseType_t err = pdFAIL;

  _button_mutex = xSemaphoreCreateMutex();

  if( _button_mutex != NULL )
  {
    err = xTaskCreatePinnedToCore( button_press_task, "buttonPress",
                                   BUTTON_TASK_STACK_SIZE, NULL, 1,
                                   &_button_task_handle, 1 );
    if( err != pdPASS || _button_task_handle == NULL )
    {
      ESP_LOGE( _TAG, "Failed to create button task" );
      vSemaphoreDelete( _button_mutex );
      _button_mutex = NULL;
    }
  }
  else
  {
    ESP_LOGE( _TAG, "Failed to create button mutex" );
  }

  return core2foraws_common_error( !err );
}