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
 * @file core2foraws_motion.h
 * @brief Core2 for AWS IoT EduKit motion sensor hardware driver APIs
 */

#ifndef _CORE2FORAWS_MOTION_H_
#define _CORE2FORAWS_MOTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <driver/i2c.h>
#include <esp_err.h>

/**
 * @brief Initializes the inertial measurement unit (IMU) motion 
 * sensor driver over I2C.
 * 
 * @note The core2foraws_init() calls this function when the 
 * hardware feature is enabled.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_motion_init] */
esp_err_t core2foraws_motion_init( void );
/* @[declare_core2foraws_motion_init] */

/**
 * @brief Retrieves the _internal_ temperature measurement from the 
 * 16-bit ADC on the inertial measurement unit (IMU) motion 
 * sensor.
 * 
 * **Example:**
 * 
 * This example gets the IMU internal temperature and then prints it 
 * out to serial output.
 * @code{c}
 *  #include <stdint.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_MOTION_EXAMPLE";
 * 
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 * 
 *      float temp_c;
 * 		core2foraws_motion_temperature_get( &temp_c );
 * 		ESP_LOGI( TAG, "\tMPU6886 Temperature: %.2f°C", temp_c );
 *  }
 * @endcode
 * 
 * @param[out] temperature Pointer to the temperature of the MPU6886 
 * passed through the 16-bit ADC.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_motion_temperature_get] */
esp_err_t core2foraws_motion_temperature_get( float *temperature );
/* @[declare_core2foraws_motion_temperature_get] */

/**
 * @brief Retrieves the acceleration measurements from the inertial 
 * measurement unit (IMU) motion sensor's accelerometer.
 * 
 * **Example:**
 * 
 * This example gets the accelerometer values for the X, Y, and Z
 * directions, then prints them out to serial output.
 * @code{c}
 *  #include <stdint.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_MOTION_EXAMPLE";
 * 
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 * 
 *      float accel_x, accel_y, accel_z;
 *      core2foraws_motion_accel_get (&accel_x, &accel_y, &accel_z );
 *      ESP_LOGI( TAG, "\tAccel x: %.2f, y: %.2f, z: %.2f", accel_x, accel_y, accel_z );
 *  }
 * @endcode
 * 
 * @param[out] x Pointer to the 16-bit accelerometer measurement in 
 * the X direction.
 * @param[out] y Pointer to the 16-bit accelerometer measurement in 
 * the Y direction.
 * @param[out] z Pointer to the 16-bit accelerometer measurement in 
 * the Z direction.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_motion_accel_get] */
esp_err_t core2foraws_motion_accel_get( float *x, float *y, float *z );
/* @[declare_core2foraws_motion_accel_get] */

/**
 * @brief Retrieves the rotational measurements from the inertial 
 * measurement unit (IMU) motion sensor's gyroscope.
 * 
 * **Example:**
 * 
 * This example gets the gyroscope values for the roll, yaw, and 
 * pitch rotations, then prints them out to serial output.
 * @code{c}
 *  #include <esp_log.h>
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_MOTION_EXAMPLE";
 * 
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 * 
 *      float g_roll, g_pitch, g_yaw;
 *      core2foraws_motion_gyro_get (&g_roll, &g_pitch, &g_yaw );
 *      ESP_LOGI( TAG, "\tGyro roll:%.2f, pitch:%.2f, yaw:%.2f", g_row, g_pitch, g_yaw );
 *  }
 * @endcode
 * 
 * @param[out] roll Pointer to the 16-bit gyroscope roll measurement.
 * @param[out] pitch Pointer to the 16-bit gyroscope pitch 
 * measurement.
 * @param[out] yaw Pointer to the 16-bit gyroscope yaw measurement.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_motion_gyro_get] */
esp_err_t core2foraws_motion_gyro_get( float *roll, float *pitch, float *yaw );
/* @[declare_core2foraws_motion_gyro_get] */

#ifdef __cplusplus
}
#endif
#endif
