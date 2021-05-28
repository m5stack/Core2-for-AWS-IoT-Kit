/**
 * @file bm8563.h
 * @brief Functions for the BM8563 Real-Time Clock (RTC).
 */

#pragma once
#include "esp_log.h"

/**
 * @brief The member defintions of the BM8563 RTC date.
 */
/* @[declare_bm8563_rtc_date] */
typedef struct _rtc_data_t {
    /*@{*/
    uint16_t year;  /**< @brief Date year. */
    uint8_t month;  /**< @brief Date month. */
    uint8_t day;    /**< @brief Date day. */
    uint8_t hour;   /**< @brief Time hour. */
    uint8_t minute; /**< @brief Time minute. */
    uint8_t second; /**< @brief Time second. */
    /*@}*/
} rtc_date_t;
/* @[declare_bm8563_rtc_date] */

/**
 * @brief Initializes the BM8563 real-time clock over i2c.
 * 
 * @note The Core2ForAWS_Init() calls this function
 * when the hardware feature is enabled.
 */
/* @[declare_bm8563_init] */
void BM8563_Init();
/* @[declare_bm8563_init] */

/**
 * @brief Sets the date and time on the BM8563.
 * 
 * **Example:**
 * 
 * Set the date and time to September 30, 2020 13:40:10.
 * @code{c}
 *  rtc_date_t date;
 *  date.year = 2020;
 *  date.month = 9;
 *  date.day = 30;
 *  
 *  date.hour = 13;
 *  date.minute = 40;
 *  date.second = 10;
 *  BM8563_SetTime(&date);
 * @endcode
 * 
 * @param[in] data Desired date and time.
 */
/* @[declare_bm8563_settime] */
void BM8563_SetTime(rtc_date_t* data);
/* @[declare_bm8563_settime] */

/**
 * @brief Retrieves the date and time on the BM8563.
 * 
 * **Example:**
 * 
 * Get the current date and time and print it out.
 * @code{c}
 *  rtc_date_t date;
 *  
 *  BM8563_GetTime(&date);
 *  printf("Date: %d-%02d-%02d Time: %02d:%02d:%02d",
 *          date.year, date.month, date.day, date.hour, date.minute, date.second)
 * @endcode
 *  
 * @param[out] data The current date and time.
 */
/* @[declare_bm8563_gettime] */
void BM8563_GetTime(rtc_date_t* data);
/* @[declare_bm8563_gettime] */

/**
 * @brief Sets the date and time "alarm" IRQ with the BM8563.
 * 
 * @param[in] minute The minute to trigger the IRQ.
 * @param[in] hour The hour to trigger the IRQ.
 * @param[in] day The day to trigger the IRQ.
 * @param[in] week The week to trigger the IRQ.
 */
/* @[declare_bm8563_setalarmirq] */
void BM8563_SetAlarmIRQ(int8_t minute, int8_t hour, int8_t day, int8_t week);
/* @[declare_bm8563_setalarmirq] */

/**
 * @brief Sets the "timer" IRQ on the BM8563.
 * 
 * @param[in] value Desired countdown time to trigger the IRQ. 
 * The maximum countdown time is 255 * 60 seconds.
 */
/* @[declare_bm8563_settimerirq] */
int16_t BM8563_SetTimerIRQ(int16_t value);
/* @[declare_bm8563_settimerirq] */

/**
 * @brief Retrieve the time left in the set timer.
 */
/* @[declare_bm8563_gettimertime] */
int16_t BM8563_GetTimerTime();
/* @[declare_bm8563_gettimertime] */

/**
 * @brief Retrieve the IRQ status from timer/alarm.
 */
/* @[declare_bm8563_getirq] */
uint8_t BM8563_GetIRQ();
/* @[declare_bm8563_getirq] */

/**
 * @brief Clear the IRQ status from timer/alarm.
 */
/* @[declare_bm8563_clearirq] */
void BM8563_ClearIRQ();
/* @[declare_bm8563_clearirq] */
