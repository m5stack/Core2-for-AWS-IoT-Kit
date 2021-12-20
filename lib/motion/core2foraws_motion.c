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
 * @file core2foraws_motion.c
 * @brief Core2 for AWS IoT EduKit motion sensor hardware driver APIs
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "mpu6886.h"
#include "i2c_manager.h"
#include "core2foraws_common.h"
#include "core2foraws_motion.h"

static const char *_s_TAG = "CORE2FORAWS_MOTION";

esp_err_t core2foraws_motion_init( void )
{
    ESP_LOGI( _s_TAG, "\tInitializing" );
    i2c_port_t port = COMMON_I2C_INTERNAL;
    return mpu6886_init( &port );
}

esp_err_t core2foraws_motion_temperature_get( float *temperature )
{
    return mpu6886_temp_data_get( temperature );
}

esp_err_t core2foraws_motion_accel_get( float *x, float *y, float *z )
{
    return mpu6886_accel_data_get( x, y, z );
}

esp_err_t core2foraws_motion_gyro_get( float *roll, float *pitch, float *yaw )
{
    return mpu6886_gyro_data_get( roll, yaw, pitch );
}