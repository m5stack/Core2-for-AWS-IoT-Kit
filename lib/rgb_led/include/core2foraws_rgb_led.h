
/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_rgb_led.h
 * @brief Core2 for AWS IoT Kit RGB LEDs hardware driver APIs
 */

#ifndef _CORE2FORAWS_RGB_LED_H_
#define _CORE2FORAWS_RGB_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <esp_err.h>

/**
 * @brief The number of RGB LEDs.
 */
/* @[declare_core2foraws_rgb_led_nums] */
#define RGB_LED_NUMS 10U
/* @[declare_core2foraws_rgb_led_nums] */

/**
 * @brief The enumerated options for LED bar sides.
 */
/* @[declare_core2foraws_rgb_led_side_type_t] */
typedef enum
{
    RGB_LED_SIDE_LEFT = 0,
    RGB_LED_SIDE_RIGHT
} rgb_led_side_type_t;
/* @[declare_core2foraws_rgb_led_side_type_t] */

/**
 * @brief Initializes the RGB LED driver.
 *
 * Returns ESP_ERR_INVALID_STATE when a prior failed initialization or teardown
 * still owns resources; retry core2foraws_rgb_led_deinit() before initialization.
 *
 * @note The core2foraws_init() calls this function when the 
 * hardware feature is enabled.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK            : Success
 *  - ESP_ERR_NO_MEM    : Out of memory
 */
/* @[declare_core2foraws_rgb_led_init] */
esp_err_t core2foraws_rgb_led_init( void );
/* @[declare_core2foraws_rgb_led_init] */

/**
 * @brief Sets the color of a single RGB LED in the LED bars.
 *
 * @note Requires following with core2foraws_rgb_led_write() to push 
 * the set color to the LED.
 *
 * **Example:**
 *
 * Set the color of each of the LEDs to white one at a time every
 * second, clear the LEDs after it's set, and loop again.
 * @code{c}
 *  #include <stdint.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  
 *  #include "core2foraws.h"
 *
 *  void rgb_demo_task( void *pvParameters )
 *  {
 *      while( 1 )
 *      {
 *          for ( uint8_t i = 0; i < 10; i++ )
 *          {
 *              core2foraws_rgb_led_single_color_set( i, 0xffffff );
 *              core2foraws_rgb_led_write();
 *              vTaskDelay( pdMS_TO_TICKS( 1000 ) );
 *          }
 *          core2foraws_rgb_led_clear();
 *          core2foraws_rgb_led_write();
 *      }
 * 
 *      core2foraws_rgb_led_deinit();
 *      vTaskDelete( NULL );
 *  }
 *  
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      xTaskCreatePinnedToCore( rgb_demo_task, "rgbLEDTask", configMINIMAL_STACK_SIZE * 3, NULL, 0, ( TaskHandle_t * ) NULL, 1 );
 *  }
 * @endcode
 *
 * @param[in] led_num The LED to set. Accepts a value from 0 to 9.
 * @param[in] color Hexadecial color value for the LED. Accepts 
 * hexadecimal (web colors). 0x000000 is black and 0xffffff is white.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_NOT_SUPPORTED : Invalid LED number or color
 */
/* @[declare_core2foraws_rgb_led_single_color_set] */
esp_err_t core2foraws_rgb_led_single_color_set( uint8_t led_num, uint32_t color );
/* @[declare_core2foraws_rgb_led_single_color_set] */

/**
 * @brief Sets the specified side of LEDs to the hexadecimal color.
 * 
 * Set the side using @ref rgb_led_side_type_t to the desired color.
 * Will change all 5 RGB LEDs of that side to the hexadecimal color.
 *
 * @note Requires following with core2foraws_rgb_led_write() to push 
 * the set color to the LED.
 *
 * **Example:**
 *
 * Set the color of the left LED bar to white for one second, clear 
 * the LEDs so it appears off for one second, and loop again.
 * @code{c}
 *  #include <stdint.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  
 *  #include "core2foraws.h"
 *
 *  void rgb_demo_task( void *pvParameters )
 *  {
 *      while( 1 )
 *      {
 *          core2foraws_rgb_led_side_color_set( RGB_LED_SIDE_LEFT, 0xffffff );
 *          core2foraws_rgb_led_write();
 *          vTaskDelay( pdMS_TO_TICKS( 1000 ) );
 * 
 *          core2foraws_rgb_led_clear();
 *          core2foraws_rgb_led_write();
 *          vTaskDelay( pdMS_TO_TICKS( 1000 ) );
 *      }
 * 
 *      core2foraws_rgb_led_deinit();
 *      vTaskDelete( NULL );
 *  }
 *  
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      xTaskCreatePinnedToCore( rgb_demo_task, "rgbLEDTask", configMINIMAL_STACK_SIZE * 3, NULL, 0, ( TaskHandle_t * ) NULL, 1 );
 *  }
 * @endcode
 *
 * @param[in] side The LED bar side to set.
 * @param[in] color Hexadecial color value for the LED. Accepts 
 * hexadecimal (web colors). 0x000000 is black and 0xffffff is white.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG   : Invalid value in parameter(s)
 *  - ESP_ERR_NOT_SUPPORTED : Invalid LED number or color
 */
/* @[declare_core2foraws_rgb_led_side_color_set] */
esp_err_t core2foraws_rgb_led_side_color_set( rgb_led_side_type_t side, uint32_t color );
/* @[declare_core2foraws_rgb_led_side_color_set] */

/**
 * @brief Set all the LEDs to a specified brightness.
 *
 * @note Requires following with core2foraws_rgb_led_write() to push 
 * the set color to the LED.
 *
 * **Example:**
 *
 * Set the color of the both LED bars to white, then run a loop to 
 * increase and then decrease the LED brightness, giving a pulse 
 * effect.
 * @code{c}
 *  #include <stdint.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  
 *  #include "core2foraws.h"
 *
 *  void rgb_demo_task( void *pvParameters )
 *  {
 *      core2foraws_rgb_led_side_color_set( RGB_LED_SIDE_LEFT, 0xffffff );
 *      core2foraws_rgb_led_side_color_set( RGB_LED_SIDE_RIGHT, 0xffffff );
 * 
 *      while( 1 )
 *      {
 * 
 *          int8_t brightness = 0;
 *          uint8_t max = 100;
 *          
 *          for( brightness = 0; brightness <= max; brightness += 10 )
 *          {
 *              core2foraws_rgb_led_brightness_set( brightness );
 *              core2foraws_rgb_led_write();
 *              vTaskDelay( pdMS_TO_TICKS( 100 ) );
 *          }
 * 
 *          for( brightness = max; brightness >= 0; brightness -= 10 )
 *          {
 *              core2foraws_rgb_led_brightness_set( brightness );
 *              core2foraws_rgb_led_write();
 *              vTaskDelay( pdMS_TO_TICKS( 100 ) );
 *          }
 *      }
 * 
 *      core2foraws_rgb_led_deinit();
 *      vTaskDelete( NULL );
 *  }
 *  
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      xTaskCreatePinnedToCore( rgb_demo_task, "rgbLEDTask", configMINIMAL_STACK_SIZE * 3, NULL, 0, ( TaskHandle_t * ) NULL, 1 );
 *  }
 * @endcode
 *
 * @param[in] brightness The brightness level to set the LED bars. 
 * Accepts percentage value from 0 to 100, with 100 being full 
 * bright.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK              : Success
 *  - ESP_ERR_INVALID_ARG : Brightness is greater than 100
 */
/* @[declare_core2foraws_rgb_led_brightness_set] */
esp_err_t core2foraws_rgb_led_brightness_set( uint8_t brightness );
/* @[declare_core2foraws_rgb_led_brightness_set] */

/**
 * @brief Updates the LEDs in the LED bars with any new
 * values set using @ref core2foraws_rgb_led_single_color_set,
 * @ref core2foraws_rgb_led_side_color_set, or @ref 
 * core2foraws_rgb_led_brightness_set.
 *
 * This function must be executed after setting LED bar values. 
 * This saves execution time by writing to the LEDs a single time
 * after making multiple changes instead of updating with
 * every change.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_FAIL              : Failed to update
 */
/* @[declare_core2foraws_rgb_led_write] */
esp_err_t core2foraws_rgb_led_write( void );
/* @[declare_core2foraws_rgb_led_write] */

/**
 * @brief Turns off the LEDs in the LED bars and removes any set 
 * color.
 *
 * @note You must use `core2foraws_rgb_led_write()` after
 * this function for the setting to take effect.
 *
 * **Example:**
 *
 * Set the color of each of the LEDs to white one at a time every
 * second, clear the LEDs after it's set, and loop again.
 * @code{c}
 *  #include <stdint.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  
 *  #include "core2foraws.h"
 *
 *  void rgb_demo_task( void *pvParameters )
 *  {
 *      while( 1 )
 *      {
 *          for ( uint8_t i = 0; i < 10; i++ )
 *          {
 *              core2foraws_rgb_led_single_color_set( i, 0xffffff );
 *              core2foraws_rgb_led_write();
 *              vTaskDelay( pdMS_TO_TICKS( 1000 ) );
 *          }
 *          core2foraws_rgb_led_clear();
 *          core2foraws_rgb_led_write();
 *      }
 *      
 *      core2foraws_rgb_led_deinit();
 *      vTaskDelete( NULL );
 *  }
 *  
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      xTaskCreatePinnedToCore( rgb_demo_task, "rgbLEDTask", configMINIMAL_STACK_SIZE * 3, NULL, 0, ( TaskHandle_t * ) NULL, 1 );
 *  }
 * @endcode
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG   : Invalid value in parameter(s)
 *  - ESP_ERR_NOT_SUPPORTED : Driver issue
 */
/* @[declare_core2foraws_rgb_led_clear] */
esp_err_t core2foraws_rgb_led_clear( void );
/* @[declare_core2foraws_rgb_led_clear] */

/**
 * @brief Removes the RGB LED driver and frees the memory used.
 *
 * Wait/disable/delete errors retain unreleased resources and return the original
 * error. Retry this function to finish cleanup; init and write are rejected
 * until it succeeds. Already completed cleanup steps are not repeated.
 * An active transmission retains its buffer until completion. This function
 * does not transmit black pixels; clear and write before deinit to turn LEDs off.
 *
 * **Example:**
 *
 * After initializing the device drivers using the @ref 
 * core2foraws_init convenience function, remove the RGB LED driver.
 * @code{c}
 *  #include "core2foraws.h"
 *  
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      
 *      core2foraws_rgb_led_deinit();
 *  }
 * @endcode
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 */
/* @[declare_core2foraws_rgb_led_deinit] */
esp_err_t core2foraws_rgb_led_deinit( void );
/* @[declare_core2foraws_rgb_led_deinit] */

#ifdef __cplusplus
}
#endif
#endif
