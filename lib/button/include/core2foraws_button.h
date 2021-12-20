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
 * @file core2foraws_button.h
 * @brief Core2 for AWS IoT EduKit virtual button hardware driver APIs
 */

#ifndef _CORE2FORAWS_BUTTON_H_
#define _CORE2FORAWS_BUTTON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <driver/i2c.h>
#include <esp_err.h>

#include "lvgl.h"

/** @brief Enumerated list of available buttons
 *
 * These are the available virtual buttons mapped to the imprinted 
 * circles on the touch screen.
 *
 * **Example:**
 *
 * Print "Left button was tapped" after the on-screen left button is 
 * quickly tapped.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <esp_log.h>
 *  #include "core2foraws.h"
 *  
 *  static const char *TAG = "MAIN_BUTTON_DEMO";
 *  void app_main ( void )
 *  {
 *      ESP_LOGI( TAG, "\tStarting..." );
 *      core2foraws_init();
 * 
 *      bool button_press = false;
 *      core2foraws_button_tapped( BUTTON_LEFT, &button_press );
 *      if ( button_press )
 *          ESP_LOGI( TAG, "\tLeft button was tapped" );
 *  }
 * @endcode
*/
/* @[declare_core2foraws_button_btns] */
enum core2foraws_button_btns
{
    BUTTON_LEFT = 0,
    BUTTON_MIDDLE,
    BUTTON_RIGHT
};
/* @[declare_core2foraws_button_btns] */

/**
 * @brief List of possible virtual button press events.
 */
/* @[declare_button_press_event_t] */
typedef enum {
    PRESS = (1 << 0),       /**< @brief Virtual button was pressed. */
    RELEASE = (1 << 1),     /**< @brief Virtual button was released. */
    LONGPRESS = (1 << 2),   /**< @brief Virtual button was long pressed. */
} press_event_t;
/* @[declare_button_press_event_t] */

/**
 * @brief Initializes the virtual buttons using the ft6336u touch 
 * controller.
 *
 * @note The core2foraws_init() calls this function when the hardware 
 * feature is enabled.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_button_init] */
esp_err_t core2foraws_button_init( void );
/* @[declare_core2foraws_button_init] */

/**
 * @brief See if the specified button was tapped. 
 * 
 * This function updates the state to true (1) if the button has been
 * touched and then released.
 * 
 * **Example:**
 *
 * Print "Left button was tapped" after the on-screen left button is 
 * quickly tapped.
 * 
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <esp_log.h>
 *  #include "core2foraws.h"
 *  
 *  static const char *TAG = "MAIN_BUTTON_DEMO";
 *  void app_main ( void )
 *  {
 *      ESP_LOGI( TAG, "\tStarting..." );
 *      core2foraws_init();
 * 
 *      bool button_press = false;
 *      core2foraws_button_tapped( BUTTON_LEFT, &button_press );
 *      if ( button_press )
 *          ESP_LOGI( TAG, "\tLeft button was tapped" );
 *  }
 * @endcode
 * 
 * @param[in] button The button to check if it was pressed. 
 * @param[out] state The current state of the button if it was pressed 
 * or not.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_button_tapped] */
esp_err_t core2foraws_button_tapped( enum core2foraws_button_btns button, bool *state );
/* @[declare_core2foraws_button_tapped] */

/**
 * @brief See if the specified button was tapped. 
 * 
 * This function updates the state to true (1) if the button has been
 * touched and then released.
 * 
 * **Example:**
 *
 * Print "Middle button is currently being pressed" when the on-screen 
 * middle button is pressed.
 * 
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <esp_log.h>
 *  #include "core2foraws.h"
 *  
 *  static const char *TAG = "MAIN_BUTTON_DEMO";
 *  void app_main ( void )
 *  {
 *      ESP_LOGI( TAG, "\tStarting..." );
 *      core2foraws_init();
 * 
 *      bool button_press = false;
 *      core2foraws_button_pressing( BUTTON_MIDDLE, &button_press );
 *      if ( button_press )
 *          ESP_LOGI( TAG, "\tMiddle button is currently being pressed" );
 *  }
 * @endcode
 * 
 * @param[in] button The button to check if it's currently being pressed. 
 * @param[out] state The current state of the button if it is being pressed 
 * or not.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_button_pressing] */
esp_err_t core2foraws_button_pressing( enum core2foraws_button_btns button, bool *state );
/* @[declare_core2foraws_button_pressing] */

/**
 * @brief See if the specified button was held. 
 * 
 * This function updates the state to true (1) if the button has been
 * held for longer than CONFIG_LV_INDEV_DEF_LONG_PRESS_TIME (400ms 
 * default).
 * 
 * **Example:**
 *
 * Print "Left button was held for more than (N)ms" after the on-screen 
 * left button is held.
 * 
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <esp_log.h>
 *  #include "core2foraws.h"
 *  
 *  static const char *TAG = "MAIN_BUTTON_DEMO";
 *  void app_main ( void )
 *  {
 *      ESP_LOGI( TAG, "\tStarting..." );
 *      core2foraws_init();
 * 
 *      bool button_press = false;
 *      core2foraws_button_held( BUTTON_LEFT, &button_press );
 *      if ( button_press )
 *          ESP_LOGI( TAG, "\tLeft button was held for more than %dms", CONFIG_LV_INDEV_DEF_LONG_PRESS_TIME );
 *  }
 * @endcode
 * 
 * @param[in] button The button to check if it has been held. 
 * @param[out] state The current state of the button if it has been held 
 * or not.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_button_held] */
esp_err_t core2foraws_button_held( enum core2foraws_button_btns button, bool *state );
/* @[declare_core2foraws_button_held] */

/**
 * @brief See if the specified button was pressed button has been released. 
 * 
 * This function updates the state to true (1) if the button has been
 * released after being pressed.
 * 
 * **Example:**
 *
 * Print "Right button was released" after the on-screen right button is 
 * released after being pressed.
 * 
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <esp_log.h>
 *  #include "core2foraws.h"
 *  
 *  static const char *TAG = "MAIN_BUTTON_DEMO";
 *  void app_main ( void )
 *  {
 *      ESP_LOGI( TAG, "\tStarting..." );
 *      core2foraws_init();
 * 
 *      bool button_press = false;
 *      core2foraws_button_released( BUTTON_RIGHT, &button_press );
 *      if ( button_press )
 *          ESP_LOGI( TAG, "\tRight button was released" );
 *  }
 * @endcode
 * 
 * @param[in] button The button to check if it's currently been released. 
 * @param[out] state The current state of the button if it was released 
 * or not.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_button_released] */
esp_err_t core2foraws_button_released( enum core2foraws_button_btns button, bool *state );
/* @[declare_core2foraws_button_released] */

#ifdef __cplusplus
}
#endif
#endif
