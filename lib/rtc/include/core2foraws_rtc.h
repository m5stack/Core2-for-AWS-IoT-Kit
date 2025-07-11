/*
 * Core2 for AWS IoT Kit BSP v2.0.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file core2foraws_rtc.h
 * @brief Core2 for AWS IoT Kit Real-Time Clock (RTC) hardware driver APIs
 */

#ifndef _CORE2FORAWS_RTC_H_
#define _CORE2FORAWS_RTC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// BM8563 constants needed for the header definitions
#define BM8563_ALARM_NONE    0x80
#define BM8563_ALARM_DISABLE 0xFF

/**
 * @brief Used to not set alarm for individual time struct property.
 */
/* @[declare_core2foraws_rtc_rtc_alarm_none] */
#define RTC_ALARM_NONE BM8563_ALARM_NONE
/* @[declare_core2foraws_rtc_rtc_alarm_none] */

/**
 * @brief Value if alarm is not set on individual time struct
 * property.
 */
/* @[declare_core2foraws_rtc_rtc_alarm_disable] */
#define RTC_ALARM_DISABLE BM8563_ALARM_DISABLE
  /* @[declare_core2foraws_rtc_rtc_alarm_disable] */

  /**
   * @brief RTC status flags for unified status checking
   */
  typedef enum
  {
    RTC_STATUS_ALARM = 0x01,     /**< Alarm has been triggered */
    RTC_STATUS_TIMER = 0x02,     /**< Timer has been triggered */
    RTC_STATUS_POWER_LOSS = 0x04 /**< Power loss detected */
  }
  /* @[declare_core2foraws_rtc_rtc_status_flags_t] */
  rtc_status_flags_t;
  /* @[declare_core2foraws_rtc_rtc_status_flags_t] */

  /**
   * @brief Initializes the Real-Time Clock (RTC) driver over I2C.
   *
   * @note The core2foraws_init() calls this function when the
   * hardware feature is enabled.
   *
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_ARG	: Driver parameter error
   */
  /* @[declare_core2foraws_rtc_init] */
  esp_err_t core2foraws_rtc_init( void );
  /* @[declare_core2foraws_rtc_init] */

  /**
   * @brief Gets the local date and time from the Real-Time Clock (RTC).
   *
   * The RTC stores time in UTC. This function converts it to local time
   * using the timezone configured in CONFIG_TIME_ZONE.
   *
   * **Example:**
   *
   * Get the current local time and print it.
   * @code{c}
   *  #include "core2foraws.h"
   *  #include <esp_log.h>
   *
   *  static const char *TAG = "RTC_DEMO";
   *
   *  void app_main(void) {
   *      struct tm local_time;
   *      char time_str[64];
   *
   *      core2foraws_init();
   *
   *      esp_err_t ret = core2foraws_rtc_time_get(&local_time);
   *      if (ret == ESP_OK) {
   *          strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S %Z",
   * &local_time); ESP_LOGI(TAG, "Current local time: %s", time_str); } else {
   *          ESP_LOGE(TAG, "Failed to get time: %s", esp_err_to_name(ret));
   *      }
   *  }
   * @endcode
   *
   * @param[out] time The local date-time converted from RTC UTC time.
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_ARG	: Input parameter error
   */
  /* @[declare_core2foraws_rtc_time_get] */
  esp_err_t core2foraws_rtc_time_get( struct tm *time );
  /* @[declare_core2foraws_rtc_time_get] */

  /**
   * @brief Sets the local date-time to the Real-Time Clock (RTC).
   *
   * This function converts local time to UTC before storing in the RTC
   * using the timezone configured in CONFIG_TIME_ZONE.
   *
   * **Example:**
   *
   * Set the RTC to a specific local time.
   * @code{c}
   *  #include "core2foraws.h"
   *  #include <esp_log.h>
   *  #include <time.h>
   *
   *  static const char *TAG = "RTC_DEMO";
   *
   *  void app_main(void) {
   *      struct tm set_time = {0};
   *
   *      core2foraws_init();
   *
   *      // Set to July 11, 2025, 10:45:00 AM local time
   *      set_time.tm_year = 2025 - 1900;  // Years since 1900
   *      set_time.tm_mon = 7 - 1;         // Month 0-11 (July = 6)
   *      set_time.tm_mday = 11;           // Day of month
   *      set_time.tm_hour = 10;           // Hour 0-23
   *      set_time.tm_min = 45;            // Minute 0-59
   *      set_time.tm_sec = 0;             // Second 0-59
   *      set_time.tm_isdst = -1;          // Let system determine DST
   *
   *      mktime(&set_time);  // Normalize the time structure
   *
   *      esp_err_t ret = core2foraws_rtc_time_set(set_time);
   *      if (ret == ESP_OK) {
   *          ESP_LOGI(TAG, "RTC time set successfully");
   *      } else {
   *          ESP_LOGE(TAG, "Failed to set time: %s", esp_err_to_name(ret));
   *      }
   *  }
   * @endcode
   *
   * @param[in] time The local date-time to set on the RTC.
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_ARG	: Input parameter error
   */
  /* @[declare_core2foraws_rtc_time_set] */
  esp_err_t core2foraws_rtc_time_set( const struct tm time );
  /* @[declare_core2foraws_rtc_time_set] */

  /**
   * @brief Gets the UTC date and time from the Real-Time Clock (RTC).
   *
   * This function returns the raw UTC time stored in the RTC without
   * any timezone conversion.
   *
   * **Example:**
   *
   * Get UTC time for logging or network protocols.
   * @code{c}
   *  #include "core2foraws.h"
   *  #include <esp_log.h>
   *
   *  static const char *TAG = "RTC_DEMO";
   *
   *  void app_main(void) {
   *      struct tm utc_time;
   *
   *      core2foraws_init();
   *
   *      esp_err_t ret = core2foraws_rtc_utc_time_get(&utc_time);
   *      if (ret == ESP_OK) {
   *          ESP_LOGI(TAG, "UTC: %04d-%02d-%02d %02d:%02d:%02d",
   *                   utc_time.tm_year + 1900, utc_time.tm_mon + 1,
   *                   utc_time.tm_mday, utc_time.tm_hour,
   *                   utc_time.tm_min, utc_time.tm_sec);
   *      } else {
   *          ESP_LOGE(TAG, "Failed to get UTC time: %s", esp_err_to_name(ret));
   *      }
   *  }
   * @endcode
   *
   * @param[out] time The UTC date-time read from the RTC.
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_ARG	: Input parameter error
   */
  /* @[declare_core2foraws_rtc_utc_time_get] */
  esp_err_t core2foraws_rtc_utc_time_get( struct tm *time );
  /* @[declare_core2foraws_rtc_utc_time_get] */

  /**
   * @brief Sets the UTC date-time to the Real-Time Clock (RTC).
   *
   * This function sets the RTC directly with UTC time without any
   * timezone conversion.
   *
   * **Example:**
   *
   * Set RTC from SNTP UTC time.
   * @code{c}
   *  #include "core2foraws.h"
   *  #include "esp_sntp.h"
   *  #include <esp_log.h>
   *
   *  static const char *TAG = "RTC_DEMO";
   *
   *  void sync_rtc_with_sntp(void) {
   *      time_t now;
   *      struct tm utc_time;
   *
   *      // Get current system time (assumed to be synced with SNTP)
   *      time(&now);
   *      gmtime_r(&now, &utc_time);
   *
   *      esp_err_t ret = core2foraws_rtc_utc_time_set(utc_time);
   *      if (ret == ESP_OK) {
   *          ESP_LOGI(TAG, "RTC synced with SNTP UTC time");
   *      } else {
   *          ESP_LOGE(TAG, "Failed to sync RTC: %s", esp_err_to_name(ret));
   *      }
   *  }
   * @endcode
   *
   * @param[in] time The UTC date-time to set on the RTC.
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_ARG	: Input parameter error
   */
  /* @[declare_core2foraws_rtc_utc_time_set] */
  esp_err_t core2foraws_rtc_utc_time_set( const struct tm time );
  /* @[declare_core2foraws_rtc_utc_time_set] */

  /**
   * @brief Gets the alarm date-time from Real-Time Clock (RTC).
   *
   * The alarm only has 4 parameters from the time struct:
   * * tm_hour — The hour the alarm will trigger. Range of 0 to 23. @ref
   * RTC_ALARM_DISABLE (255) if not set.
   * * tm_min — The minute the alarm will trigger. Range of 0 to 59. @ref
   * RTC_ALARM_DISABLE (255) if not set.
   * * tm_mday — The day of the month the alarm will trigger. Range of 0 to 31.
   * @ref RTC_ALARM_DISABLE (255) if not set.
   * * tm_wday — The day of the week the alarm will trigger. Range of 0 to 6.
   * @ref RTC_ALARM_DISABLE (255) if not set.
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
   * 		    ESP_LOGI( TAG, "\tAlarm set for hour %d, minute %d, on the
   * %d day of week, %d day of month\n", alarm_time.tm_hour, alarm_time.tm_min,
   * alarm_time.tm_wday, alarm_time.tm_mday );
   *  }
   * @endcode
   *
   * @param[out] alarm_time Pointer to the date-time the alarm is set to.
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
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
   * RTC_ALARM_DISABLE (255) to disable.
   * * tm_min — The minute the alarm will trigger. Range of 0 to 59. @ref
   * RTC_ALARM_DISABLE (255) to disable.
   * * tm_mday — The day of the month the alarm will trigger. Range of 1 to 31.
   * @ref RTC_ALARM_DISABLE (255) to disable.
   * * tm_wday — The day of the week the alarm will trigger. Range of 0 to 6.
   * @ref RTC_ALARM_DISABLE (255) to disable.
   *
   * **Example:**
   *
   * Set a daily alarm for 7:30 AM.
   * @code{c}
   *  #include "core2foraws.h"
   *  #include <esp_log.h>
   *
   *  static const char *TAG = "RTC_DEMO";
   *
   *  void app_main(void) {
   *      struct tm alarm_time = {0};
   *
   *      core2foraws_init();
   *
   *      // Set alarm for 7:30 AM every day
   *      alarm_time.tm_hour = 7;                    // 7 AM
   *      alarm_time.tm_min = 30;                    // 30 minutes
   *      alarm_time.tm_mday = RTC_ALARM_DISABLE;    // Any day of month
   *      alarm_time.tm_wday = RTC_ALARM_DISABLE;    // Any day of week
   *
   *      esp_err_t ret = core2foraws_rtc_alarm_set(alarm_time);
   *      if (ret == ESP_OK) {
   *          ESP_LOGI(TAG, "Daily alarm set for 7:30 AM");
   *      } else {
   *          ESP_LOGE(TAG, "Failed to set alarm: %s", esp_err_to_name(ret));
   *      }
   *  }
   * @endcode
   *
   * @param[in] alarm_time The date-time the alarm is set to.
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_ARG	: Input parameter error
   */
  /* @[declare_core2foraws_rtc_alarm_set] */
  esp_err_t core2foraws_rtc_alarm_set( struct tm alarm_time );
  /* @[declare_core2foraws_rtc_alarm_set] */

  /**
   * @brief Gets and optionally clears the alarm triggered state from Real-Time
   * Clock (RTC).
   *
   * **Example:**
   *
   * Check if alarm has been triggered in a periodic task.
   * @code{c}
   *  #include "core2foraws.h"
   *  #include <esp_log.h>
   *  #include <freertos/FreeRTOS.h>
   *  #include <freertos/task.h>
   *
   *  static const char *TAG = "RTC_DEMO";
   *
   *  void alarm_check_task(void *pvParameters) {
   *      bool alarm_triggered = false;
   *
   *      while (1) {
   *          // Check alarm status and clear if triggered
   *          esp_err_t ret = core2foraws_rtc_alarm_status(&alarm_triggered,
   * true); if (ret == ESP_OK && alarm_triggered) { ESP_LOGI(TAG, "Alarm
   * triggered! Time to wake up!");
   *              // Handle alarm event here
   *          }
   *
   *          vTaskDelay(pdMS_TO_TICKS(1000));  // Check every second
   *      }
   *  }
   * @endcode
   *
   * @param[out] triggered Pointer to store the alarm triggered state. true if
   * triggered, false otherwise.
   * @param[in] clear_flag If true, clears the alarm triggered state after
   * reading.
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_ARG	: Input parameter error
   */
  /* @[declare_core2foraws_rtc_alarm_status] */
  esp_err_t core2foraws_rtc_alarm_status( bool *triggered, bool clear_flag );
  /* @[declare_core2foraws_rtc_alarm_status] */

  /**
   * @brief Gets and optionally clears the timer triggered state from Real-Time
   * Clock (RTC).
   *
   * **Example:**
   *
   * Check if timer has been triggered and clear it if so.
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
   *          bool timer_triggered = false;
   *          // Check timer status and clear if triggered
   *          core2foraws_rtc_timer_status( &timer_triggered, true );
   *          if ( timer_triggered )
   *          {
   *              ESP_LOGI( TAG, "\tTimer expired and cleared!" );
   *              vTaskSuspend( NULL );
   *          }
   *          vTaskDelay( pdMS_TO_TICKS( 500 ) );
   *      }
   *  }
   * @endcode
   *
   * @param[out] triggered Pointer to store the timer triggered state. true if
   * triggered, false otherwise.
   * @param[in] clear_flag If true, clears the timer triggered state after
   * reading.
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_ARG	: Input parameter error
   */
  /* @[declare_core2foraws_rtc_timer_status] */
  esp_err_t core2foraws_rtc_timer_status( bool *triggered, bool clear_flag );
  /* @[declare_core2foraws_rtc_timer_status] */

  /**
   * @brief Gets unified RTC status flags and optionally clears specified flags.
   *
   * This function provides a unified way to check multiple RTC status
   * conditions in a single call and selectively clear flags.
   *
   * **Example:**
   *
   * Check all RTC status flags and handle accordingly.
   * @code{c}
   *  #include <stdint.h>
   *  #include <esp_err.h>
   *  #include <esp_log.h>
   *
   *  #include "core2foraws.h"
   *
   *  static const char *TAG = "MAIN_RTC_DEMO";
   *
   *  void app_main( void )
   *  {
   *      core2foraws_init();
   *
   *      uint8_t status_flags = 0;
   *      uint8_t clear_mask = RTC_STATUS_ALARM | RTC_STATUS_TIMER;
   *
   *      // Get all status flags and clear alarm/timer flags
   *      esp_err_t err = core2foraws_rtc_get_status( &status_flags,
   *            clear_mask );
   *      if ( err == ESP_OK )
   *      {
   *          if ( status_flags & RTC_STATUS_ALARM )
   *              ESP_LOGI( TAG, "Alarm was triggered" );
   *          if ( status_flags & RTC_STATUS_TIMER )
   *              ESP_LOGI( TAG, "Timer was triggered" );
   *          if ( status_flags & RTC_STATUS_POWER_LOSS )
   *              ESP_LOGI( TAG, "Power loss detected" );
   *      }
   *  }
   * @endcode
   *
   * @param[out] flags Pointer to store the combined status flags (bitwise OR of
   * rtc_status_flags_t).
   * @param[in] clear_mask Bitmask of flags to clear after reading (bitwise OR
   * of rtc_status_flags_t).
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_ARG	: Input parameter error
   */
  /* @[declare_core2foraws_rtc_get_status] */
  esp_err_t core2foraws_rtc_get_status( uint8_t *flags, uint8_t clear_mask );
  /* @[declare_core2foraws_rtc_get_status] */

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
   *          esp_err_t err = core2foraws_rtc_timer_get( &timer_seconds );
   *          if ( err == ESP_OK )
   *          {
   *              ESP_LOGI( TAG,"\t%d seconds left on timer", timer_seconds );
   *              core2foraws_rtc_timer_status( &timer_trigger, false );
   *              if ( timer_trigger )
   *              {
   *                  ESP_LOGI( TAG, "\tTimer expired!" );
   *                  core2foraws_rtc_timer_status( &timer_trigger, true );
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
   *      xTaskCreatePinnedToCore(
   * 		rtc_demo_task,
   * 		"rtcTask",
   * 		configMINIMAL_STACK_SIZE * 3,
   * 		NULL,
   * 		0,
   * 		( TaskHandle_t * ) NULL,
   * 		1
   *      );
   *  }
   * @endcode
   *
   * @param[out] seconds Pointer to the number of seconds on the current timer.
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
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
   * The timer supports values from 1 to 15300 seconds (255 minutes).
   * The RTC automatically selects the best frequency for the duration.
   *
   * **Example:**
   *
   * Set a 5-minute timer and wait for it to expire.
   * @code{c}
   *  #include "core2foraws.h"
   *  #include <esp_log.h>
   *  #include <freertos/FreeRTOS.h>
   *  #include <freertos/task.h>
   *
   *  static const char *TAG = "RTC_DEMO";
   *
   *  void timer_demo_task(void *pvParameters) {
   *      // Set a 5-minute (300 second) timer
   *      esp_err_t ret = core2foraws_rtc_timer_set(300);
   *      if (ret != ESP_OK) {
   *          ESP_LOGE(TAG, "Failed to set timer: %s", esp_err_to_name(ret));
   *          return;
   *      }
   *
   *      ESP_LOGI(TAG, "5-minute timer started");
   *
   *      while (1) {
   *          bool timer_triggered = false;
   *          ret = core2foraws_rtc_timer_status(&timer_triggered, false);
   *
   *          if (ret == ESP_OK && timer_triggered) {
   *              ESP_LOGI(TAG, "Timer expired!");
   *              // Clear the timer flag
   *              core2foraws_rtc_timer_status(&timer_triggered, true);
   *              break;
   *          }
   *
   *          vTaskDelay(pdMS_TO_TICKS(1000));  // Check every second
   *      }
   *  }
   * @endcode
   *
   * @param[in] seconds The number of seconds to set the timer (1-15300).
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_ARG	: Input parameter error (value out of range)
   */
  /* @[declare_core2foraws_rtc_timer_set] */
  esp_err_t core2foraws_rtc_timer_set( uint32_t seconds );
  /* @[declare_core2foraws_rtc_timer_set] */

#ifdef __cplusplus
}
#endif
#endif
