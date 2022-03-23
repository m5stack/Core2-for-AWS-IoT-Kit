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
 * @file core2foraws_rtc.h
 * @brief Core2 for AWS IoT EduKit Real-Time Clock (RTC) hardware driver APIs
 */

#ifndef _CORE2FORAWS_RTC_H_
#define _CORE2FORAWS_RTC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <esp_err.h>

#include "bm8563.h"

/**
 * @brief Used to not set alarm for individual time struct property.
 */
/* @[declare_core2foraws_rtc_rtc_alarm_none] */
#define RTC_ALARM_NONE      BM8563_ALARM_NONE
/* @[declare_core2foraws_rtc_rtc_alarm_none] */

/**
 * @brief Value if alarm is not set on individual time struct 
 * property.
 */
/* @[declare_core2foraws_rtc_rtc_alarm_disable] */
#define RTC_ALARM_DISABLE   BM8563_ALARM_DISABLE
/* @[declare_core2foraws_rtc_rtc_alarm_disable] */

/**
 * @brief Initializes the Real-Time Clock (RTC) driver over I2C.
 *
 * @note The core2foraws_init() calls this function when the 
 * hardware feature is enabled.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_rtc_init] */
esp_err_t core2foraws_rtc_init( void );
/* @[declare_core2foraws_rtc_init] */

/**
 * @brief Gets the date and time from the Real-Time Clock (RTC).
 * 
 * **Example:**
 * 
 * Get the current date and time and print it out.
 * @code{c}
 *  #include <stdint.h>
 *  #include <time.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_RTC_DEMO";
 * 
 *  void app_main( void )
 *  {
 *      struct tm datetime;
 *      
 *      core2foraws_init();
 *      core2foraws_rtc_time_get( &datetime );
 *      ESP_LOGI( TAG, "\tDate: %d-%02d-%02d Time: %02d:%02d:%02d",
 *          datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second );
 *  }
 
 * @endcode
 *  
 * @param[out] time The date-time read from the RTC.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Input parameter error
 */
/* @[declare_core2foraws_rtc_time_get] */
esp_err_t core2foraws_rtc_time_get( struct tm *time );
/* @[declare_core2foraws_rtc_time_get] */

/**
 * @brief Sets the date-time to the Real-Time Clock (RTC).
 * 
 * **Example:**
 * 
 * Get the current date-time from the RTC, add 1 minute, 
 * set the RTC to the new date-time, print out the new time.
 * @code{c}
 *  #include <stdint.h>
 *  #include <time.h>
 *  #include <esp_err.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_RTC_DEMO";
 * 
 *  void app_main( void )
 *  {
 *      struct tm datetime;
 *      esp_err_t err = ESP_FAIL;
 *      
 *      core2foraws_init();
 *      core2foraws_rtc_time_get( &datetime );
 *      
 *      datetime.second += 1;
 *  
 *      err = core2foraws_rtc_time_set( datetime );
 *      if ( err == ESP_OK )
 *      {
 *          char buffer[ 128 ];
 *          strftime( buffer, 128 ,"%c (day %j)" , &datetime );
	        ESP_LOGI( TAG, "\tCurrent time: %s\n", buffer );
 *      }
 *  }
 
 * @endcode
 *  
 * @param[in] time The date-time to set on the RTC.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Input parameter error
 */
/* @[declare_core2foraws_rtc_time_set] */
esp_err_t core2foraws_rtc_time_set( const struct tm time );
/* @[declare_core2foraws_rtc_time_set] */

/**
 * @brief Gets the alarm date-time from Real-Time Clock (RTC).
 * 
 * The alarm only has 4 parameters from the time struct:
 * * tm_hour — The hour the alarm will trigger. Range of 0 to 23. @ref 
 * RTC_ALARM_DISABLE (255) if not set.
 * * tm_min — The minute the alarm will trigger. Range of 0 to 59. @ref 
 * RTC_ALARM_DISABLE (255) if not set.
 * * tm_mday — The day of the month the alarm will trigger. Range of 0 to 31. @ref 
 * RTC_ALARM_DISABLE (255) if not set.
 * * tm_wday — The day of the week the alarm will trigger. Range of 0 to 6. @ref 
 * RTC_ALARM_DISABLE (255) if not set.
 * 
 * **Example:**
 * 
 * Get the current alarm date-time from the RTC, set alarm to
 * today, 1 minute from the current time, if successful, print out 
 * the date-time the alarm was set to. 
 * @code{c}
 *  #include <stdint.h>
 *  #include <time.h>
 *  #include <esp_err.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_RTC_DEMO";
 * 
 *  void app_main( void )
 *  {
 *      struct tm datetime;
 *      struct tm alarm_time = { 0 };
 *      esp_err_t err = ESP_FAIL;
 *      
 *      core2foraws_init();
 *      core2foraws_rtc_time_get( &datetime );
 *      
 *      
 *      if ( datetime.tm_min == 59 )
 *      {
 *          alarm_time.tm_hour = datetime.tm_hour + 1;
 *          alarm_time.tm_min = 0;
 *          if ( alarm_time.tm_hour == 25 )
 *              alarm_time.tm_hour = 0;
 *      }
 *      else
 *      {
 *          alarm_time.tm_hour = datetime.tm_hour;
 *          alarm_time.tm_min = datetime.tm_min + 1;
 *      }
 *      alarm_time.tm_mday = RTC_ALARM_NONE;
 *      alarm_time.tm_wday = RTC_ALARM_NONE;
 * 
 *      datetime.tm_min += 1;
 *  
 *      core2foraws_rtc_alarm_set( alarm_time );
 *      alarm_time = { 0 };
 * 
 *      err = core2foraws_rtc_alarm_get( &alarm_time );
 *      if ( err == ESP_OK )
 * 		    ESP_LOGI( TAG, "\tAlarm set for hour %d, minute %d, on the %d day of week, %d day of month\n", 
 *              alarm_time.tm_hour, alarm_time.tm_min, alarm_time.tm_wday, alarm_time.tm_mday );
 *  }
 * @endcode
 *  
 * @param[out] alarm_time Pointer to the date-time the alarm is set to.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Input parameter error
 */
/* @[declare_core2foraws_rtc_alarm_get] */
esp_err_t core2foraws_rtc_alarm_get( struct tm *alarm_time );
/* @[declare_core2foraws_rtc_alarm_get] */

/**
 * @brief Sets the alarm date-time on the Real-Time Clock (RTC).
 * 
 * The alarm only takes 4 parameters from the time struct:
 * * tm_hour — The hour the alarm will trigger. Range of 0 to 23. @ref 
 * RTC_ALARM_DISABLE (255) if not set.
 * * tm_min — The minute the alarm will trigger. Range of 0 to 59. @ref 
 * RTC_ALARM_DISABLE (255) if not set.
 * * tm_mday — The day of the month the alarm will trigger. Range of 0 to 31. @ref 
 * RTC_ALARM_DISABLE (255) if not set.
 * * tm_wday — The day of the week the alarm will trigger. Range of 0 to 6. @ref 
 * RTC_ALARM_DISABLE (255) if not set.
 * 
 * **Example:**
 * 
 * Get the current alarm date-time from the RTC, set alarm to
 * today, 1 minute from the current time, if successful, print out 
 * the date-time the alarm was set to. 
 * @code{c}
 *  #include <stdint.h>
 *  #include <time.h>
 *  #include <esp_err.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_RTC_DEMO";
 * 
 *  void app_main( void )
 *  {
 *      struct tm datetime;
 *      struct tm alarm_time = { 0 };
 *      esp_err_t err = ESP_FAIL;
 *      
 *      core2foraws_init();
 *      core2foraws_rtc_time_get( &datetime );
 *      
 *      
 *      if ( datetime.tm_min == 59 )
 *      {
 *          alarm_time.tm_hour = datetime.tm_hour + 1;
 *          alarm_time.tm_min = 0;
 *          if ( alarm_time.tm_hour == 25 )
 *              alarm_time.tm_hour = 0;
 *      }
 *      else
 *      {
 *          alarm_time.tm_hour = datetime.tm_hour;
 *          alarm_time.tm_min = datetime.tm_min + 1;
 *      }
 *      alarm_time.tm_mday = RTC_ALARM_NONE;
 *      alarm_time.tm_wday = RTC_ALARM_NONE;
 * 
 *      datetime.tm_min += 1;
 *  
 *      core2foraws_rtc_alarm_set( alarm_time );
 *      alarm_time = { 0 };
 * 
 *      err = core2foraws_rtc_alarm_get( &alarm_time );
 *      if ( err == ESP_OK )
 * 		    ESP_LOGI( TAG, "\tAlarm set for hour %d, minute %d, on the %d day of week, %d day of month\n", 
 *              alarm_time.tm_hour, alarm_time.tm_min, alarm_time.tm_wday, alarm_time.tm_mday );
 *  }
 * @endcode
 *  
 * @param[in] alarm_time The date-time the alarm is set to.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Input parameter error
 */
/* @[declare_core2foraws_rtc_alarm_set] */
esp_err_t core2foraws_rtc_alarm_set( struct tm alarm_time );
/* @[declare_core2foraws_rtc_alarm_set] */

/**
 * @brief Gets the alarm triggered state from Real-Time Clock (RTC).
 * 
 * **Example:**
 * 
 * Get the current alarm date-time from the RTC, set alarm to
 * today, 1 minute from the current time, if successful, print out 
 * the date-time the alarm was set to. Then create a FreeRTOS task
 * that continually checks if the alarm has been triggered, if it
 * has, print it out and clear the alarm.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <time.h>
 *  #include <esp_err.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_RTC_DEMO";
 * 
 *  void rtc_demo_task( void *pvParameters )
 *  {
 *      bool alarm_status = false;
 *      for ( ;; )
 *      {
 *          core2foraws_rtc_alarm_triggered( &alarm_status );
 *          if ( alarm_status == true )
 *          {
 *              ESP_LOGI( TAG, "\tAlarm has been triggered!" );
 *              core2foraws_rtc_alarm_clear();
 *          }
 *      }
 *  }
 * 
 *  void app_main( void )
 *  {
 *      struct tm datetime;
 *      struct tm alarm_time = { 0 };
 *      esp_err_t err = ESP_FAIL;
 *      
 *      core2foraws_init();
 *      core2foraws_rtc_time_get( &datetime );
 *      
 *      
 *      if ( datetime.tm_min == 59 )
 *      {
 *          alarm_time.tm_hour = datetime.tm_hour + 1;
 *          alarm_time.tm_min = 0;
 *          if ( alarm_time.tm_hour == 25 )
 *              alarm_time.tm_hour = 0;
 *      }
 *      else
 *      {
 *          alarm_time.tm_hour = datetime.tm_hour;
 *          alarm_time.tm_min = datetime.tm_min + 1;
 *      }
 *      alarm_time.tm_mday = RTC_ALARM_NONE;
 *      alarm_time.tm_wday = RTC_ALARM_NONE;
 * 
 *      datetime.tm_min += 1;
 *  
 *      core2foraws_rtc_alarm_set( alarm_time );
 *      alarm_time = { 0 };
 * 
 *      err = core2foraws_rtc_alarm_get( &alarm_time );
 *      if ( err == ESP_OK )
 * 		    ESP_LOGI( TAG, "\tAlarm set for hour %d, minute %d, on the %d day of week, %d day of month\n", 
 *              alarm_time.tm_hour, alarm_time.tm_min, alarm_time.tm_wday, alarm_time.tm_mday );
 * 
 *      xTaskCreatePinnedToCore( rtc_demo_task, "rtcTask", configMINIMAL_STACK_SIZE * 3, NULL, 0, ( TaskHandle_t * ) NULL, 1 );
 *  }
 * @endcode
 *  
 * @param[out] state The state of the alarm. 0 is not triggered, 1 if it is.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Input parameter error
 */
/* @[declare_core2foraws_rtc_alarm_triggered] */
esp_err_t core2foraws_rtc_alarm_triggered( bool *state );
/* @[declare_core2foraws_rtc_alarm_triggered] */

/**
 * @brief Clears the alarm triggered state from Real-Time Clock (RTC).
 * 
 * @note This function sets the alarm triggered value to false. It does
 * not clear a reoccuring alarm, just the current triggered state.
 * 
 * **Example:**
 * 
 * Get the current alarm date-time from the RTC, set alarm to
 * today, 1 minute from the current time, if successful, print out 
 * the date-time the alarm was set to. Then create a FreeRTOS task
 * that continually checks if the alarm has been triggered, if it
 * has, print it out and clear the alarm.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <time.h>
 *  #include <esp_err.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_RTC_DEMO";
 * 
 *  void rtc_demo_task( void *pvParameters )
 *  {
 *      bool alarm_status = false;
 *      for ( ;; )
 *      {
 *          core2foraws_rtc_alarm_triggered( &alarm_status );
 *          if ( alarm_status == true )
 *          {
 *              ESP_LOGI( TAG, "\tAlarm has been triggered!" );
 *              core2foraws_rtc_alarm_clear();
 *          }
 *      }
 *  }
 * 
 *  void app_main( void )
 *  {
 *      struct tm datetime;
 *      struct tm alarm_time = { 0 };
 *      esp_err_t err = ESP_FAIL;
 *      
 *      core2foraws_init();
 *      core2foraws_rtc_time_get( &datetime );
 *      
 *      
 *      if ( datetime.tm_min == 59 )
 *      {
 *          alarm_time.tm_hour = datetime.tm_hour + 1;
 *          alarm_time.tm_min = 0;
 *          if ( alarm_time.tm_hour == 25 )
 *              alarm_time.tm_hour = 0;
 *      }
 *      else
 *      {
 *          alarm_time.tm_hour = datetime.tm_hour;
 *          alarm_time.tm_min = datetime.tm_min + 1;
 *      }
 *      alarm_time.tm_mday = RTC_ALARM_NONE;
 *      alarm_time.tm_wday = RTC_ALARM_NONE;
 * 
 *      datetime.tm_min += 1;
 *  
 *      core2foraws_rtc_alarm_set( alarm_time );
 *      alarm_time = { 0 };
 * 
 *      err = core2foraws_rtc_alarm_get( &alarm_time );
 *      if ( err == ESP_OK )
 * 		    ESP_LOGI( TAG, "\tAlarm set for hour %d, minute %d, on the %d day of week, %d day of month\n", 
 *              alarm_time.tm_hour, alarm_time.tm_min, alarm_time.tm_wday, alarm_time.tm_mday );
 * 
 *      xTaskCreatePinnedToCore( rtc_demo_task, "rtcTask", configMINIMAL_STACK_SIZE * 3, NULL, 0, ( TaskHandle_t * ) NULL, 1 );
 *  }
 * @endcode
 *  
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_rtc_alarm_clear] */
esp_err_t core2foraws_rtc_alarm_clear( void );
/* @[declare_core2foraws_rtc_alarm_clear] */

/**
 * @brief Gets the number of seconds left on the Real-Time Clock 
 * (RTC) timer.
 * 
 * **Example:**
 * 
 * In a FreeRTOS task, set a 5 second timer, check periodically how 
 * much time is left on the timer, if the timer has been triggered,
 * print a message to serial output, clear the timer, and suspend 
 * the FreeRTOS task so it's not using MCU cycles.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_err.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_RTC_DEMO";
 * 
 *  void rtc_demo_task( void *pvParameters )
 *  {
 *      core2foraws_rtc_timer_set( 5 );
 * 
 *      for ( ;; )
 *      {
 *          uint32_t timer_seconds;
 *          bool timer_trigger = false;
 *          err = core2foraws_rtc_timer_get( &timer_seconds );
 *          if ( err == ESP_OK )
 *          {
 *              ESP_LOGI( TAG,"\t%d seconds left on timer", timer_seconds );
 *              core2foraws_rtc_timer_triggered( &timer_trigger );
 *              if ( timer_trigger )
 *              {
 *                  ESP_LOGI( TAG, "\tTimer expired!" );
 *                  err = core2foraws_rtc_timer_clear();
 *                  ESP_LOGI( TAG, "\tCleared timer 0x%x", err );
 *                  
 *                  vTaskSuspend( NULL );
 *              }
 * 
 *              vTaskDelay( pdMS_TO_TICKS( 500 ) );
 *          }
 *      }
 *  }
 * 
 *  void app_main( void )
 *  {      
 *      core2foraws_init();
 *      
 *      xTaskCreatePinnedToCore( rtc_demo_task, "rtcTask", configMINIMAL_STACK_SIZE * 3, NULL, 0, ( TaskHandle_t * ) NULL, 1 );
 *  }
 * @endcode
 *  
 * @param[out] seconds Pointer to the number of seconds on the current timer.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Input parameter error
 */
/* @[declare_core2foraws_rtc_timer_get] */
esp_err_t core2foraws_rtc_timer_get( uint32_t *seconds );
/* @[declare_core2foraws_rtc_timer_get] */

/**
 * @brief Sets the timer on the Real-Time Clock (RTC) to specified 
 * number of seconds.
 * 
 * **Example:**
 * 
 * In a FreeRTOS task, set a 5 second timer, check periodically how 
 * much time is left on the timer, if the timer has been triggered,
 * print a message to serial output, clear the timer, and suspend 
 * the FreeRTOS task so it's not using MCU cycles.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_err.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_RTC_DEMO";
 * 
 *  void rtc_demo_task( void *pvParameters )
 *  {
 *      core2foraws_rtc_timer_set( 5 );
 * 
 *      for ( ;; )
 *      {
 *          uint32_t timer_seconds;
 *          bool timer_trigger = false;
 *          err = core2foraws_rtc_timer_get( &timer_seconds );
 *          if ( err == ESP_OK )
 *          {
 *              ESP_LOGI( TAG,"\t%d seconds left on timer", timer_seconds );
 *              core2foraws_rtc_timer_triggered( &timer_trigger );
 *              if ( timer_trigger )
 *              {
 *                  ESP_LOGI( TAG, "\tTimer expired!" );
 *                  err = core2foraws_rtc_timer_clear();
 *                  ESP_LOGI( TAG, "\tCleared timer 0x%x", err );
 *                  
 *                  vTaskSuspend( NULL );
 *              }
 * 
 *              vTaskDelay( pdMS_TO_TICKS( 500 ) );
 *          }
 *      }
 *  }
 * 
 *  void app_main( void )
 *  {      
 *      core2foraws_init();
 *      
 *      xTaskCreatePinnedToCore( rtc_demo_task, "rtcTask", configMINIMAL_STACK_SIZE * 3, NULL, 0, ( TaskHandle_t * ) NULL, 1 );
 *  }
 * @endcode
 *  
 * @param[in] seconds The number of seconds to set the current timer.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Input parameter error
 */
/* @[declare_core2foraws_rtc_timer_set] */
esp_err_t core2foraws_rtc_timer_set( uint32_t seconds );
/* @[declare_core2foraws_rtc_timer_set] */

/**
 * @brief Get the state of the timer on the Real-Time Clock (RTC).
 * 
 * **Example:**
 * 
 * In a FreeRTOS task, set a 5 second timer, check periodically how 
 * much time is left on the timer, if the timer has been triggered,
 * print a message to serial output, clear the timer, and suspend 
 * the FreeRTOS task so it's not using MCU cycles.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_err.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_RTC_DEMO";
 * 
 *  void rtc_demo_task( void *pvParameters )
 *  {
 *      core2foraws_rtc_timer_set( 5 );
 * 
 *      for ( ;; )
 *      {
 *          uint32_t timer_seconds;
 *          bool timer_trigger = false;
 *          err = core2foraws_rtc_timer_get( &timer_seconds );
 *          if ( err == ESP_OK )
 *          {
 *              ESP_LOGI( TAG,"\t%d seconds left on timer", timer_seconds );
 *              core2foraws_rtc_timer_triggered( &timer_trigger );
 *              if ( timer_trigger )
 *              {
 *                  ESP_LOGI( TAG, "\tTimer expired!" );
 *                  err = core2foraws_rtc_timer_clear();
 *                  ESP_LOGI( TAG, "\tCleared timer 0x%x", err );
 *                  
 *                  vTaskSuspend( NULL );
 *              }
 * 
 *              vTaskDelay( pdMS_TO_TICKS( 500 ) );
 *          }
 *      }
 *  }
 * 
 *  void app_main( void )
 *  {      
 *      core2foraws_init();
 *      
 *      xTaskCreatePinnedToCore( rtc_demo_task, "rtcTask", configMINIMAL_STACK_SIZE * 3, NULL, 0, ( TaskHandle_t * ) NULL, 1 );
 *  }
 * @endcode
 *  
 * @param[out] state The number of state of the timer. 0 if it hasn't been triggered, 1 if it has.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Input parameter error
 */
/* @[declare_core2foraws_rtc_timer_triggered] */
esp_err_t core2foraws_rtc_timer_triggered( bool *state );
/* @[declare_core2foraws_rtc_timer_triggered] */

/**
 * @brief Clear the timer on the Real-Time Clock (RTC).
 * 
 * **Example:**
 * 
 * In a FreeRTOS task, set a 5 second timer, check periodically how 
 * much time is left on the timer, if the timer has been triggered,
 * print a message to serial output, clear the timer, and suspend 
 * the FreeRTOS task so it's not using MCU cycles.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_err.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_RTC_DEMO";
 * 
 *  void rtc_demo_task( void *pvParameters )
 *  {
 *      core2foraws_rtc_timer_set( 5 );
 * 
 *      for ( ;; )
 *      {
 *          uint32_t timer_seconds;
 *          bool timer_trigger = false;
 *          err = core2foraws_rtc_timer_get( &timer_seconds );
 *          if ( err == ESP_OK )
 *          {
 *              ESP_LOGI( TAG,"\t%d seconds left on timer", timer_seconds );
 *              core2foraws_rtc_timer_triggered( &timer_trigger );
 *              if ( timer_trigger )
 *              {
 *                  ESP_LOGI( TAG, "\tTimer expired!" );
 *                  err = core2foraws_rtc_timer_clear();
 *                  ESP_LOGI( TAG, "\tCleared timer 0x%x", err );
 *                  
 *                  vTaskSuspend( NULL );
 *              }
 * 
 *              vTaskDelay( pdMS_TO_TICKS( 500 ) );
 *          }
 *      }
 *  }
 * 
 *  void app_main( void )
 *  {      
 *      core2foraws_init();
 *      
 *      xTaskCreatePinnedToCore( rtc_demo_task, "rtcTask", configMINIMAL_STACK_SIZE * 3, NULL, 0, ( TaskHandle_t * ) NULL, 1 );
 *  }
 * @endcode
 *  
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_rtc_timer_clear] */
esp_err_t core2foraws_rtc_timer_clear( void );
/* @[declare_core2foraws_rtc_timer_clear] */

#ifdef __cplusplus
}
#endif
#endif
