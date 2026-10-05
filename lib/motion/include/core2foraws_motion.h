/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_motion.h
 * @brief Core2 for AWS IoT Kit motion sensor hardware driver APIs
 */

#ifndef _CORE2FORAWS_MOTION_H_
#define _CORE2FORAWS_MOTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <esp_err.h>

/**
 * @brief Accelerometer full-scale range options.
 *
 * A smaller range gives finer resolution; a larger range measures
 * stronger accelerations before saturating.
 */
/* @[declare_core2foraws_motion_accel_range_t] */
typedef enum
{
    MOTION_ACCEL_RANGE_2G = 0,  /**< @brief ±2 G full-scale range. */
    MOTION_ACCEL_RANGE_4G,      /**< @brief ±4 G full-scale range. */
    MOTION_ACCEL_RANGE_8G,      /**< @brief ±8 G full-scale range (default). */
    MOTION_ACCEL_RANGE_16G,     /**< @brief ±16 G full-scale range. */
} motion_accel_range_t;
/* @[declare_core2foraws_motion_accel_range_t] */

/**
 * @brief Gyroscope full-scale range options.
 *
 * A smaller range gives finer resolution; a larger range measures
 * faster rotations before saturating.
 */
/* @[declare_core2foraws_motion_gyro_range_t] */
typedef enum
{
    MOTION_GYRO_RANGE_250DPS = 0, /**< @brief ±250 °/s full-scale range. */
    MOTION_GYRO_RANGE_500DPS,     /**< @brief ±500 °/s full-scale range. */
    MOTION_GYRO_RANGE_1000DPS,    /**< @brief ±1000 °/s full-scale range. */
    MOTION_GYRO_RANGE_2000DPS,    /**< @brief ±2000 °/s full-scale range (default). */
} motion_gyro_range_t;
/* @[declare_core2foraws_motion_gyro_range_t] */

/**
 * @brief Initializes the inertial measurement unit (IMU) motion 
 * sensor driver over I2C.
 *
 * The IMU is identified with core2foraws_board_info_get(): the MPU6886 on
 * the Core2 for AWS, or the BMI270 on the Core2 for AWS v1.3. Both report
 * in the same board frame (X+ screen right, Y+ screen top, Z+ out of the
 * screen) at ±8 G and ±2000 °/s after initialization.
 *
 * The BMI270 needs an 8 KB configuration file after it loses power. If it
 * stayed powered across an ESP32 reset, the loaded file is reused;
 * otherwise it is uploaded over I2C, which takes about 0.26 s.
 * 
 * @note The core2foraws_init() calls this function when the 
 * hardware feature is enabled. Repeating this function after successful
 * initialization returns ESP_OK without adding another I2C device handle.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_NOT_FOUND     : No supported IMU answered
 *  - ESP_ERR_TIMEOUT       : The BMI270 did not accept its configuration file
 *  - Other                 : I2C error
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
 * 		ESP_LOGI( TAG, "\tIMU Temperature: %.2f°C", temp_c );
 *  }
 * @endcode
 * 
 * @param[out] temperature Pointer to the IMU die temperature in °C.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 *  - ESP_ERR_INVALID_STATE : IMU not initialized, or the BMI270 has not
 *                            finished its first temperature conversion
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
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
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
 *      ESP_LOGI( TAG, "\tGyro roll:%.2f, pitch:%.2f, yaw:%.2f", g_roll, g_pitch, g_yaw );
 *  }
 * @endcode
 * 
 * @param[out] roll Pointer to the 16-bit gyroscope roll measurement.
 * @param[out] pitch Pointer to the 16-bit gyroscope pitch 
 * measurement.
 * @param[out] yaw Pointer to the 16-bit gyroscope yaw measurement.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_motion_gyro_get] */
esp_err_t core2foraws_motion_gyro_get( float *roll, float *pitch, float *yaw );
/* @[declare_core2foraws_motion_gyro_get] */

/**
 * @brief Retrieves accelerometer and gyroscope measurements taken at the
 * same sampling instant.
 *
 * Calling core2foraws_motion_accel_get() and core2foraws_motion_gyro_get()
 * back to back issues two I2C transfers, so the IMU can update between
 * them. This function reads both in one burst, which is what sensor-fusion
 * filters expect.
 *
 * **Example:**
 *
 * @code{c}
 *  float ax, ay, az, roll, pitch, yaw;
 *  core2foraws_motion_accel_gyro_get( &ax, &ay, &az, &roll, &pitch, &yaw );
 * @endcode
 *
 * @param[out] x Acceleration in the X direction, in Gs.
 * @param[out] y Acceleration in the Y direction, in Gs.
 * @param[out] z Acceleration in the Z direction, in Gs.
 * @param[out] roll Gyroscope roll rate, in degrees per second.
 * @param[out] pitch Gyroscope pitch rate, in degrees per second.
 * @param[out] yaw Gyroscope yaw rate, in degrees per second.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_motion_accel_gyro_get] */
esp_err_t core2foraws_motion_accel_gyro_get( float *x, float *y, float *z,
                                             float *roll, float *pitch,
                                             float *yaw );
/* @[declare_core2foraws_motion_accel_gyro_get] */

/**
 * @brief Sets the accelerometer full-scale measurement range.
 *
 * Register and cached scale updates are serialized against scaled reads.
 * Failed register writes leave the cached conversion factor unchanged.
 *
 * A smaller range gives finer resolution but saturates at lower
 * accelerations; a larger range measures stronger accelerations with
 * coarser resolution. core2foraws_motion_init() selects ±8 G.
 *
 * Per the MPU-6886 datasheet, the first few samples immediately after a
 * range change may be unsettled and should be discarded.
 *
 * **Example:**
 *
 * Switch the accelerometer to its ±2 G range for finer resolution.
 * @code{c}
 *  #include "core2foraws.h"
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      core2foraws_motion_accel_range_set( MOTION_ACCEL_RANGE_2G );
 *  }
 * @endcode
 *
 * @param[in] range The desired accelerometer full-scale range.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_motion_accel_range_set] */
esp_err_t core2foraws_motion_accel_range_set( motion_accel_range_t range );
/* @[declare_core2foraws_motion_accel_range_set] */

/**
 * @brief Sets the gyroscope full-scale measurement range.
 *
 * Register and cached scale updates are serialized against scaled reads.
 * Failed register writes leave the cached conversion factor unchanged.
 *
 * A smaller range gives finer resolution but saturates at lower
 * rotational speeds; a larger range measures faster rotations with
 * coarser resolution. core2foraws_motion_init() selects ±2000 °/s.
 *
 * Per the MPU-6886 datasheet, the first few samples immediately after a
 * range change may be unsettled and should be discarded.
 *
 * **Example:**
 *
 * Switch the gyroscope to its ±250 °/s range for finer resolution.
 * @code{c}
 *  #include "core2foraws.h"
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      core2foraws_motion_gyro_range_set( MOTION_GYRO_RANGE_250DPS );
 *  }
 * @endcode
 *
 * @param[in] range The desired gyroscope full-scale range.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_motion_gyro_range_set] */
esp_err_t core2foraws_motion_gyro_range_set( motion_gyro_range_t range );
/* @[declare_core2foraws_motion_gyro_range_set] */

#ifdef __cplusplus
}
#endif
#endif
