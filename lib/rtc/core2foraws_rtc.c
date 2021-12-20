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
 * @file core2foraws_rtc.c
 * @brief Core2 for AWS IoT EduKit Real-Time Clock (RTC) hardware driver APIs
 */

#include <esp_log.h>

#include "i2c_manager.h"
#include "core2foraws_common.h"
#include "core2foraws_rtc.h"

#define RTC_I2C	i2c_hal( COMMON_I2C_INTERNAL )

static const char *_s_TAG = "CORE2FORAWS_RTC";

esp_err_t core2foraws_rtc_init( void )
{
    ESP_LOGI( _s_TAG, "\tInitializing" );
    bm8563_t* bm8563_i2c_device = ( bm8563_t * )RTC_I2C;
    bm8563_err_t err = bm8563_init( bm8563_i2c_device );
    
    return core2foraws_common_error( err );
}

esp_err_t core2foraws_rtc_time_get( struct tm *time )
{
	bm8563_t* bm8563_i2c_device = ( bm8563_t * )RTC_I2C;
    bm8563_err_t err = bm8563_read( bm8563_i2c_device, time );

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_rtc_time_set( const struct tm time )
{
    bm8563_t* bm8563_i2c_device = ( bm8563_t * )RTC_I2C;
    bm8563_err_t err = bm8563_write( bm8563_i2c_device, &time );

    return core2foraws_common_error(err);
}

esp_err_t core2foraws_rtc_alarm_get( struct tm *alarm_time )
{
    bm8563_t* bm8563_i2c_device = ( bm8563_t * )RTC_I2C;
    bm8563_err_t err = bm8563_ioctl( bm8563_i2c_device, BM8563_ALARM_READ, alarm_time );

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_rtc_alarm_set( struct tm alarm_time )
{
    bm8563_t* bm8563_i2c_device = ( bm8563_t * )RTC_I2C;
    bm8563_err_t err = bm8563_ioctl( bm8563_i2c_device, BM8563_ALARM_SET, &alarm_time );

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_rtc_alarm_triggered( bool *state )
{
    uint8_t tmp;
    bm8563_t* bm8563_i2c_device = ( bm8563_t * )RTC_I2C;
    bm8563_err_t err = bm8563_ioctl( bm8563_i2c_device, BM8563_CONTROL_STATUS2_READ, &tmp );
    *state = tmp & BM8563_AF;
    
    return core2foraws_common_error( err );
}

esp_err_t core2foraws_rtc_alarm_clear( void )
{
    uint8_t tmp = 0;
    bm8563_t* bm8563_i2c_device = ( bm8563_t * )RTC_I2C;
    bm8563_err_t err = bm8563_ioctl( bm8563_i2c_device, BM8563_CONTROL_STATUS2_WRITE, &tmp );

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_rtc_timer_get( uint32_t *seconds )
{
    bm8563_t* bm8563_i2c_device = ( bm8563_t *) RTC_I2C;
    bm8563_err_t err = bm8563_ioctl( bm8563_i2c_device, BM8563_TIMER_READ, ( void * ) seconds );

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_rtc_timer_set( uint32_t seconds )
{
    bm8563_t* bm8563_i2c_device = ( bm8563_t * )RTC_I2C;
    bm8563_err_t err = bm8563_ioctl( bm8563_i2c_device, BM8563_TIMER_WRITE, &seconds );
    uint8_t timer_control = BM8563_TIMER_ENABLE | BM8563_TIMER_1HZ;

    if ( err == BM8563_OK )
    {
        err = bm8563_ioctl( bm8563_i2c_device, BM8563_TIMER_CONTROL_WRITE, &timer_control );
    }

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_rtc_timer_triggered( bool *state )
{
    bm8563_t *bm8563_i2c_device = ( bm8563_t * )RTC_I2C;
    uint8_t timer_control = BM8563_TIMER_ENABLE | BM8563_TIMER_1HZ;
    bm8563_err_t err = bm8563_ioctl( bm8563_i2c_device, BM8563_CONTROL_STATUS2_READ, &timer_control );
    *state = timer_control & BM8563_TF;

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_rtc_timer_clear( void )
{
    bm8563_t* bm8563_i2c_device = ( bm8563_t * )RTC_I2C;
    uint8_t tmp = 0x00;
    bm8563_err_t err = bm8563_ioctl( bm8563_i2c_device, BM8563_TIMER_CONTROL_WRITE, &tmp );
    if (err == BM8563_OK){
        bm8563_ioctl( bm8563_i2c_device, BM8563_TIMER_WRITE, &tmp );
        err = bm8563_ioctl( bm8563_i2c_device, BM8563_CONTROL_STATUS2_WRITE, &tmp );
    }

    return core2foraws_common_error( err );
}