/*
 * Core2 for AWS IoT Kit BSP v2.0.0
 * Copyright (C) 2025 Rashed Talukder.  All Rights Reserved.
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

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core2foraws_common.h"
#include "core2foraws_rtc.h"
#include "i2c_manager.h"

static const char *_TAG = "CORE2FORAWS_RTC";

// BM8563 I2C Address
#define BM8563_I2C_ADDR 0x51

// BM8563 Register Addresses
#define BM8563_REG_CTRL_STATUS1 0x00
#define BM8563_REG_CTRL_STATUS2 0x01
#define BM8563_REG_SECONDS      0x02
#define BM8563_REG_MINUTES      0x03
#define BM8563_REG_HOURS        0x04
#define BM8563_REG_DAYS         0x05
#define BM8563_REG_WEEKDAYS     0x06
#define BM8563_REG_MONTHS       0x07
#define BM8563_REG_YEARS        0x08
#define BM8563_REG_ALARM_MIN    0x09
#define BM8563_REG_ALARM_HOUR   0x0A
#define BM8563_REG_ALARM_DAY    0x0B
#define BM8563_REG_ALARM_WEEK   0x0C
#define BM8563_REG_CLKOUT_FREQ  0x0D
#define BM8563_REG_TIMER_CTRL   0x0E
#define BM8563_REG_TIMER_COUNT  0x0F

// Control/Status Register Bits
#define BM8563_CTRL1_STOP 0x20
#define BM8563_CTRL2_AF   0x08 // Alarm Flag
#define BM8563_CTRL2_TF   0x04 // Timer Flag
#define BM8563_CTRL2_AIE  0x02 // Alarm Interrupt Enable
#define BM8563_CTRL2_TIE  0x01 // Timer Interrupt Enable

// Time Register Bits
#define BM8563_SECONDS_VL    0x80 // Voltage Low
#define BM8563_MONTH_CENTURY 0x80 // Century bit (0=20xx, 1=19xx)

// Alarm and Timer Constants
#define BM8563_ALARM_NONE    0x80
#define BM8563_TIMER_TE      0x80 // Timer Enable
#define BM8563_TIMER_TD_MASK 0x03 // Timer Frequency Mask

// Timer Frequencies
#define BM8563_TIMER_FREQ_4096HZ 0x00 // 4.096 kHz
#define BM8563_TIMER_FREQ_64HZ   0x01 // 64 Hz
#define BM8563_TIMER_FREQ_1HZ    0x02 // 1 Hz
#define BM8563_TIMER_FREQ_1_60HZ 0x03 // 1/60 Hz (1 minute)

// Static function prototypes
static uint8_t _dec_to_bcd( uint8_t dec );
static uint8_t _bcd_to_dec( uint8_t bcd );
static esp_err_t _bm8563_read_reg( uint8_t reg, uint8_t *data, size_t len );
static esp_err_t _bm8563_write_reg( uint8_t reg, const uint8_t *data,
                                    size_t len );
static esp_err_t _bm8563_read_reg_internal( uint8_t reg, uint8_t *data,
                                            size_t len );
static esp_err_t _bm8563_write_reg_internal( uint8_t reg, const uint8_t *data,
                                             size_t len );
static void _tm_to_bm8563( const struct tm *tm_time, uint8_t *bm_regs );
static void _bm8563_to_tm( const uint8_t *bm_regs, struct tm *tm_time );
static esp_err_t _setup_timezone( void );
static void _restore_timezone( void );

static bool _rtc_initialized = false;
static char *_saved_tz = NULL;

static uint8_t _dec_to_bcd( uint8_t dec )
{
  return ( ( dec / 10 ) << 4 ) | ( dec % 10 );
}

static uint8_t _bcd_to_dec( uint8_t bcd )
{
  return ( ( bcd >> 4 ) * 10 ) + ( bcd & 0x0F );
}

static esp_err_t _bm8563_read_reg( uint8_t reg, uint8_t *data, size_t len )
{
  if( !_rtc_initialized )
  {
    ESP_LOGE( _TAG, "RTC not initialized" );
    return ESP_ERR_INVALID_STATE;
  }
  return i2c_manager_read( COMMON_I2C_INTERNAL, BM8563_I2C_ADDR, reg, data,
                           len );
}

static esp_err_t _bm8563_write_reg( uint8_t reg, const uint8_t *data,
                                    size_t len )
{
  if( !_rtc_initialized )
  {
    ESP_LOGE( _TAG, "RTC not initialized" );
    return ESP_ERR_INVALID_STATE;
  }
  return i2c_manager_write( COMMON_I2C_INTERNAL, BM8563_I2C_ADDR, reg, data,
                            len );
}

static esp_err_t _bm8563_read_reg_internal( uint8_t reg, uint8_t *data,
                                            size_t len )
{
  return i2c_manager_read( COMMON_I2C_INTERNAL, BM8563_I2C_ADDR, reg, data,
                           len );
}

static esp_err_t _bm8563_write_reg_internal( uint8_t reg, const uint8_t *data,
                                             size_t len )
{
  return i2c_manager_write( COMMON_I2C_INTERNAL, BM8563_I2C_ADDR, reg, data,
                            len );
}

static void _tm_to_bm8563( const struct tm *tm_time, uint8_t *bm_regs )
{
  bm_regs[ 0 ] = _dec_to_bcd( tm_time->tm_sec );
  bm_regs[ 1 ] = _dec_to_bcd( tm_time->tm_min );
  bm_regs[ 2 ] = _dec_to_bcd( tm_time->tm_hour );
  bm_regs[ 3 ] = _dec_to_bcd( tm_time->tm_mday );
  bm_regs[ 4 ] = tm_time->tm_wday & 0x07;

  uint8_t month = _dec_to_bcd( tm_time->tm_mon + 1 );

  // Century bit: 0 = 20xx, 1 = 19xx (tm_year is years since 1900)
  if( tm_time->tm_year >= 100 )
  {
    month &= ~BM8563_MONTH_CENTURY; // 20xx
  }
  else
  {
    month |= BM8563_MONTH_CENTURY; // 19xx
  }
  bm_regs[ 5 ] = month;
  bm_regs[ 6 ] = _dec_to_bcd( tm_time->tm_year % 100 );
}

static void _bm8563_to_tm( const uint8_t *bm_regs, struct tm *tm_time )
{
  memset( tm_time, 0, sizeof( struct tm ) );

  tm_time->tm_sec = _bcd_to_dec( bm_regs[ 0 ] & 0x7F );
  tm_time->tm_min = _bcd_to_dec( bm_regs[ 1 ] & 0x7F );
  tm_time->tm_hour = _bcd_to_dec( bm_regs[ 2 ] & 0x3F );
  tm_time->tm_mday = _bcd_to_dec( bm_regs[ 3 ] & 0x3F );
  tm_time->tm_wday = bm_regs[ 4 ] & 0x07;

  uint8_t month_reg = bm_regs[ 5 ];
  tm_time->tm_mon = _bcd_to_dec( month_reg & 0x1F ) - 1;

  int year_2digit = _bcd_to_dec( bm_regs[ 6 ] );

  // Century handling: 0 = 20xx, 1 = 19xx according to datasheet
  if( !( month_reg & BM8563_MONTH_CENTURY ) )
  {
    tm_time->tm_year = year_2digit + 100; // Years since 1900 for 20xx
  }
  else
  {
    tm_time->tm_year = year_2digit; // Years since 1900 for 19xx
  }

  // Let mktime() determine DST. This is important for correct conversion
  // from the RTC's UTC time to local time by the caller.
  tm_time->tm_isdst = -1;
}

static esp_err_t _setup_timezone( void )
{
  // Save current TZ if exists
  char *current_tz = getenv( "TZ" );
  if( current_tz )
  {
    if( _saved_tz )
    {
      free( _saved_tz );
    }
    _saved_tz = strdup( current_tz );
  }

  // Set to UTC for conversion
  setenv( "TZ", "UTC0", 1 );
  tzset();

  return ESP_OK;
}

static void _restore_timezone( void )
{
  if( _saved_tz )
  {
    setenv( "TZ", _saved_tz, 1 );
    free( _saved_tz );
    _saved_tz = NULL;
  }
  else
  {
    // Set to configured timezone
    setenv( "TZ", CONFIG_TIME_ZONE, 1 );
  }
  tzset();
}

esp_err_t core2foraws_rtc_init( void )
{
  ESP_LOGI( _TAG, "Initializing BM8563 RTC" );

  uint8_t ctrl_reg = 0;
  esp_err_t ret =
      _bm8563_read_reg_internal( BM8563_REG_CTRL_STATUS1, &ctrl_reg, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to read BM8563 control register: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  // Start RTC clock and disable test modes
  ctrl_reg &= ~BM8563_CTRL1_STOP;
  ret = _bm8563_write_reg_internal( BM8563_REG_CTRL_STATUS1, &ctrl_reg, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to write BM8563 control register: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  // Clear all pending status flags
  ctrl_reg = 0x00;
  ret = _bm8563_write_reg_internal( BM8563_REG_CTRL_STATUS2, &ctrl_reg, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to clear BM8563 status flags: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  // Set the configured timezone
  setenv( "TZ", CONFIG_TIME_ZONE, 1 );
  tzset();

  _rtc_initialized = true;
  ESP_LOGI( _TAG, "BM8563 RTC initialized successfully with timezone: %s",
            CONFIG_TIME_ZONE );
  return ESP_OK;
}

esp_err_t core2foraws_rtc_time_get( struct tm *time )
{
  if( time == NULL )
  {
    ESP_LOGE( _TAG, "Time pointer is NULL" );
    return ESP_ERR_INVALID_ARG;
  }

  // Get UTC time from RTC
  esp_err_t ret = core2foraws_rtc_utc_time_get( time );
  if( ret != ESP_OK )
  {
    return ret;
  }

  // Convert UTC to local time
  time_t utc_time = mktime( time );
  struct tm *local_time = localtime( &utc_time );
  if( local_time == NULL )
  {
    ESP_LOGE( _TAG, "Failed to convert UTC to local time" );
    return ESP_FAIL;
  }

  // Copy local time to output
  memcpy( time, local_time, sizeof( struct tm ) );

  return ESP_OK;
}

esp_err_t core2foraws_rtc_time_set( const struct tm time )
{
  // Convert local time to UTC
  struct tm local_copy = time;
  time_t local_time = mktime( &local_copy );

  struct tm *utc_time = gmtime( &local_time );
  if( utc_time == NULL )
  {
    ESP_LOGE( _TAG, "Failed to convert local time to UTC" );
    return ESP_FAIL;
  }

  // Set UTC time to RTC
  return core2foraws_rtc_utc_time_set( *utc_time );
}

esp_err_t core2foraws_rtc_utc_time_get( struct tm *time )
{
  if( time == NULL )
  {
    ESP_LOGE( _TAG, "Time pointer is NULL" );
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t time_regs[ 7 ];
  esp_err_t ret = _bm8563_read_reg( BM8563_REG_SECONDS, time_regs, 7 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to read time registers: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  // Check for power-down condition (Voltage Low flag)
  if( time_regs[ 0 ] & BM8563_SECONDS_VL )
  {
    ESP_LOGW( _TAG, "RTC power-down detected, time may be invalid" );
    // Clear the flag after reading
    time_regs[ 0 ] &= ~BM8563_SECONDS_VL;
    _bm8563_write_reg( BM8563_REG_SECONDS, &time_regs[ 0 ], 1 );
  }

  // Convert hardware registers to UTC time struct
  _setup_timezone();
  _bm8563_to_tm( time_regs, time );
  _restore_timezone();

  return ESP_OK;
}

esp_err_t core2foraws_rtc_utc_time_set( const struct tm time )
{
  int year = time.tm_year + 1900;
  if( year < 1900 || year > 2099 )
  {
    ESP_LOGE( _TAG, "Year %d out of supported range (1900-2099)", year );
    return ESP_ERR_INVALID_ARG;
  }

  // Input time is expected to be UTC
  uint8_t time_regs[ 7 ];
  _tm_to_bm8563( &time, time_regs );

  esp_err_t ret = _bm8563_write_reg( BM8563_REG_SECONDS, time_regs, 7 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to write time registers: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  ESP_LOGI( _TAG, "RTC time set successfully" );
  return ESP_OK;
}

esp_err_t core2foraws_rtc_alarm_get( struct tm *alarm_time )
{
  if( alarm_time == NULL )
  {
    ESP_LOGE( _TAG, "Invalid parameter: alarm_time is NULL" );
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t alarm_regs[ 4 ];
  esp_err_t ret = _bm8563_read_reg( BM8563_REG_ALARM_MIN, alarm_regs, 4 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to read alarm registers: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  memset( alarm_time, 0, sizeof( struct tm ) );

  // Convert alarm registers to tm structure
  if( alarm_regs[ 0 ] & BM8563_ALARM_NONE )
  {
    alarm_time->tm_min = RTC_ALARM_DISABLE;
  }
  else
  {
    alarm_time->tm_min = _bcd_to_dec( alarm_regs[ 0 ] & 0x7F );
  }

  if( alarm_regs[ 1 ] & BM8563_ALARM_NONE )
  {
    alarm_time->tm_hour = RTC_ALARM_DISABLE;
  }
  else
  {
    alarm_time->tm_hour = _bcd_to_dec( alarm_regs[ 1 ] & 0x3F );
  }

  if( alarm_regs[ 2 ] & BM8563_ALARM_NONE )
  {
    alarm_time->tm_mday = RTC_ALARM_DISABLE;
  }
  else
  {
    alarm_time->tm_mday = _bcd_to_dec( alarm_regs[ 2 ] & 0x3F );
  }

  if( alarm_regs[ 3 ] & BM8563_ALARM_NONE )
  {
    alarm_time->tm_wday = RTC_ALARM_DISABLE;
  }
  else
  {
    alarm_time->tm_wday = alarm_regs[ 3 ] & 0x07;
  }

  return ESP_OK;
}

esp_err_t core2foraws_rtc_alarm_set( struct tm alarm_time )
{
  uint8_t alarm_regs[ 4 ];

  // Convert tm structure to alarm registers
  if( alarm_time.tm_min == RTC_ALARM_DISABLE ||
      alarm_time.tm_min == RTC_ALARM_NONE )
  {
    alarm_regs[ 0 ] = BM8563_ALARM_NONE;
  }
  else
  {
    alarm_regs[ 0 ] = _dec_to_bcd( alarm_time.tm_min ) & 0x7F;
  }

  if( alarm_time.tm_hour == RTC_ALARM_DISABLE ||
      alarm_time.tm_hour == RTC_ALARM_NONE )
  {
    alarm_regs[ 1 ] = BM8563_ALARM_NONE;
  }
  else
  {
    alarm_regs[ 1 ] = _dec_to_bcd( alarm_time.tm_hour ) & 0x3F;
  }

  if( alarm_time.tm_mday == RTC_ALARM_DISABLE ||
      alarm_time.tm_mday == RTC_ALARM_NONE )
  {
    alarm_regs[ 2 ] = BM8563_ALARM_NONE;
  }
  else
  {
    alarm_regs[ 2 ] = _dec_to_bcd( alarm_time.tm_mday ) & 0x3F;
  }

  if( alarm_time.tm_wday == RTC_ALARM_DISABLE ||
      alarm_time.tm_wday == RTC_ALARM_NONE )
  {
    alarm_regs[ 3 ] = BM8563_ALARM_NONE;
  }
  else
  {
    alarm_regs[ 3 ] = alarm_time.tm_wday & 0x07;
  }

  esp_err_t ret = _bm8563_write_reg( BM8563_REG_ALARM_MIN, alarm_regs, 4 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to set alarm: %s", esp_err_to_name( ret ) );
    return ret;
  }

  // Enable alarm interrupt
  uint8_t ctrl2;
  ret = _bm8563_read_reg( BM8563_REG_CTRL_STATUS2, &ctrl2, 1 );
  if( ret != ESP_OK )
  {
    return ret;
  }

  ctrl2 |= BM8563_CTRL2_AIE;
  ret = _bm8563_write_reg( BM8563_REG_CTRL_STATUS2, &ctrl2, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to enable alarm interrupt: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  ESP_LOGI( _TAG, "Alarm set successfully" );
  return ESP_OK;
}

esp_err_t core2foraws_rtc_alarm_status( bool *triggered, bool clear_flag )
{
  if( triggered == NULL )
  {
    ESP_LOGE( _TAG, "Invalid parameter: triggered is NULL" );
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t ctrl2;
  esp_err_t ret = _bm8563_read_reg( BM8563_REG_CTRL_STATUS2, &ctrl2, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to read control register 2: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  *triggered = ( ctrl2 & BM8563_CTRL2_AF ) ? true : false;

  // Clear alarm flag if requested and flag is set
  if( clear_flag && *triggered )
  {
    ctrl2 &= ~BM8563_CTRL2_AF;
    ret = _bm8563_write_reg( BM8563_REG_CTRL_STATUS2, &ctrl2, 1 );
    if( ret != ESP_OK )
    {
      ESP_LOGE( _TAG, "Failed to clear alarm flag: %s",
                esp_err_to_name( ret ) );
      return ret;
    }
  }

  return ESP_OK;
}

esp_err_t core2foraws_rtc_timer_status( bool *triggered, bool clear_flag )
{
  if( triggered == NULL )
  {
    ESP_LOGE( _TAG, "Invalid parameter: triggered is NULL" );
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t ctrl2;
  esp_err_t ret = _bm8563_read_reg( BM8563_REG_CTRL_STATUS2, &ctrl2, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to read control register 2: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  *triggered = ( ctrl2 & BM8563_CTRL2_TF ) ? true : false;

  // Clear timer flag if requested and flag is set
  if( clear_flag && *triggered )
  {
    ctrl2 &= ~BM8563_CTRL2_TF;
    ret = _bm8563_write_reg( BM8563_REG_CTRL_STATUS2, &ctrl2, 1 );
    if( ret != ESP_OK )
    {
      ESP_LOGE( _TAG, "Failed to clear timer flag: %s",
                esp_err_to_name( ret ) );
      return ret;
    }
  }

  return ESP_OK;
}

esp_err_t core2foraws_rtc_get_status( uint8_t *flags, uint8_t clear_mask )
{
  if( flags == NULL )
  {
    ESP_LOGE( _TAG, "Invalid parameter: flags is NULL" );
    return ESP_ERR_INVALID_ARG;
  }

  *flags = 0;
  uint8_t ctrl2, seconds_reg;

  esp_err_t ret = _bm8563_read_reg( BM8563_REG_CTRL_STATUS2, &ctrl2, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to read control register 2: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  ret = _bm8563_read_reg( BM8563_REG_SECONDS, &seconds_reg, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to read seconds register: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  // Set status flags
  if( ctrl2 & BM8563_CTRL2_AF )
    *flags |= RTC_STATUS_ALARM;
  if( ctrl2 & BM8563_CTRL2_TF )
    *flags |= RTC_STATUS_TIMER;
  if( seconds_reg & BM8563_SECONDS_VL )
    *flags |= RTC_STATUS_POWER_LOSS;

  // Clear requested flags
  if( clear_mask != 0 )
  {
    uint8_t ctrl2_new = ctrl2;
    uint8_t seconds_new = seconds_reg;
    bool need_ctrl2_update = false;
    bool need_seconds_update = false;

    if( ( clear_mask & RTC_STATUS_ALARM ) && ( ctrl2 & BM8563_CTRL2_AF ) )
    {
      ctrl2_new &= ~BM8563_CTRL2_AF;
      need_ctrl2_update = true;
    }

    if( ( clear_mask & RTC_STATUS_TIMER ) && ( ctrl2 & BM8563_CTRL2_TF ) )
    {
      ctrl2_new &= ~BM8563_CTRL2_TF;
      need_ctrl2_update = true;
    }

    if( ( clear_mask & RTC_STATUS_POWER_LOSS ) &&
        ( seconds_reg & BM8563_SECONDS_VL ) )
    {
      seconds_new &= ~BM8563_SECONDS_VL;
      need_seconds_update = true;
    }

    if( need_ctrl2_update )
    {
      ret = _bm8563_write_reg( BM8563_REG_CTRL_STATUS2, &ctrl2_new, 1 );
      if( ret != ESP_OK )
      {
        ESP_LOGE( _TAG,
                  "Failed to clear status flags in control register 2: %s",
                  esp_err_to_name( ret ) );
        return ret;
      }
    }

    if( need_seconds_update )
    {
      ret = _bm8563_write_reg( BM8563_REG_SECONDS, &seconds_new, 1 );
      if( ret != ESP_OK )
      {
        ESP_LOGE( _TAG, "Failed to clear power loss flag: %s",
                  esp_err_to_name( ret ) );
        return ret;
      }
    }
  }

  return ESP_OK;
}

esp_err_t core2foraws_rtc_timer_get( uint32_t *seconds )
{
  if( seconds == NULL )
  {
    ESP_LOGE( _TAG, "Invalid parameter: seconds is NULL" );
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t timer_ctrl, timer_count;
  esp_err_t ret = _bm8563_read_reg( BM8563_REG_TIMER_CTRL, &timer_ctrl, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to read timer control: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  ret = _bm8563_read_reg( BM8563_REG_TIMER_COUNT, &timer_count, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to read timer count: %s", esp_err_to_name( ret ) );
    return ret;
  }

  // Calculate seconds based on timer frequency
  uint8_t freq = timer_ctrl & BM8563_TIMER_TD_MASK;
  switch( freq )
  {
  case BM8563_TIMER_FREQ_4096HZ:
    *seconds = timer_count / 4096;
    break;
  case BM8563_TIMER_FREQ_64HZ:
    *seconds = timer_count / 64;
    break;
  case BM8563_TIMER_FREQ_1HZ:
    *seconds = timer_count;
    break;
  case BM8563_TIMER_FREQ_1_60HZ:
    *seconds = timer_count * 60;
    break;
  default:
    *seconds = 0;
    break;
  }

  return ESP_OK;
}

esp_err_t core2foraws_rtc_timer_set( uint32_t seconds )
{
  // Max duration is 255 minutes (15300 seconds)
  if( seconds == 0 || seconds > 15300 )
  {
    ESP_LOGE( _TAG,
              "Invalid timer value: %u seconds. Must be between 1 and 15300.",
              seconds );
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t timer_ctrl, timer_count;
  uint8_t freq;

  // Choose appropriate frequency to maximize range and precision
  if( seconds <= 255 )
  {
    freq = BM8563_TIMER_FREQ_1HZ; // 1-second resolution
    timer_count = (uint8_t)seconds;
  }
  else // seconds <= 15300
  {
    freq = BM8563_TIMER_FREQ_1_60HZ; // 1-minute resolution
    timer_count = (uint8_t)( seconds / 60 );
  }

  // Disable timer before setting new values
  timer_ctrl = freq; // TE bit is 0
  esp_err_t ret = _bm8563_write_reg( BM8563_REG_TIMER_CTRL, &timer_ctrl, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to disable timer: %s", esp_err_to_name( ret ) );
    return ret;
  }

  // Set timer countdown value
  ret = _bm8563_write_reg( BM8563_REG_TIMER_COUNT, &timer_count, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to set timer count: %s", esp_err_to_name( ret ) );
    return ret;
  }

  // Enable timer
  timer_ctrl = BM8563_TIMER_TE | freq;
  ret = _bm8563_write_reg( BM8563_REG_TIMER_CTRL, &timer_ctrl, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to enable timer: %s", esp_err_to_name( ret ) );
    return ret;
  }

  // Enable timer interrupt flag
  uint8_t ctrl2;
  ret = _bm8563_read_reg( BM8563_REG_CTRL_STATUS2, &ctrl2, 1 );
  if( ret != ESP_OK )
  {
    return ret;
  }

  ctrl2 |= BM8563_CTRL2_TIE;
  ret = _bm8563_write_reg( BM8563_REG_CTRL_STATUS2, &ctrl2, 1 );
  if( ret != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to enable timer interrupt: %s",
              esp_err_to_name( ret ) );
    return ret;
  }

  ESP_LOGI( _TAG, "Timer set for %u seconds", seconds );
  return ESP_OK;
}