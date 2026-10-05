/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_motion.c
 * @brief Core2 for AWS IoT Kit motion sensor hardware driver APIs
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "mpu6886.h"
#include "bmi270.h"
#include "core2foraws_common.h"
#include "core2foraws_board.h"
#include "core2foraws_motion.h"

static const char *_TAG = "CORE2FORAWS_MOTION";
static core2foraws_board_imu_t _imu = BOARD_IMU_NONE;

esp_err_t core2foraws_motion_init( void )
{
    ESP_LOGI( _TAG, "\tInitializing" );

    if ( _imu != BOARD_IMU_NONE )
    {
        return ESP_OK;
    }

    core2foraws_board_info_t board;
    esp_err_t err = core2foraws_board_info_get( &board );
    if ( err != ESP_OK )
    {
        return err;
    }

    switch ( board.imu )
    {
        case BOARD_IMU_MPU6886:
            err = mpu6886_init( COMMON_I2C_INTERNAL );
            break;
        case BOARD_IMU_BMI270:
            err = bmi270_init( COMMON_I2C_INTERNAL );
            break;
        case BOARD_IMU_NONE:
        default:
            err = ESP_ERR_NOT_FOUND;
            break;
    }

    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "\t%s init failed: 0x%x", board.imu_name, err );
    }
    else
    {
        ESP_LOGI( _TAG, "\t%s ready", board.imu_name );
        _imu = board.imu;
    }
    return err;
}

esp_err_t core2foraws_motion_temperature_get( float *temperature )
{
    if( temperature == NULL ) return ESP_ERR_INVALID_ARG;
    esp_err_t err;
    switch( _imu )
    {
        case BOARD_IMU_MPU6886: err = mpu6886_temp_data_get( temperature ); break;
        case BOARD_IMU_BMI270:  err = bmi270_temp_data_get( temperature ); break;
        default:                return ESP_ERR_INVALID_STATE;
    }
    if ( err == ESP_OK )
    {
        ESP_LOGV( _TAG, "temperature=%.2f C", *temperature );
    }
    return err;
}

esp_err_t core2foraws_motion_accel_get( float *x, float *y, float *z )
{
    if( x == NULL || y == NULL || z == NULL ) return ESP_ERR_INVALID_ARG;
    esp_err_t err;
    switch( _imu )
    {
        case BOARD_IMU_MPU6886: err = mpu6886_accel_data_get( x, y, z ); break;
        case BOARD_IMU_BMI270:  err = bmi270_accel_data_get( x, y, z ); break;
        default:                return ESP_ERR_INVALID_STATE;
    }
    if ( err == ESP_OK )
    {
        ESP_LOGV( _TAG, "accel x=%.3f y=%.3f z=%.3f g", *x, *y, *z );
    }
    return err;
}

esp_err_t core2foraws_motion_gyro_get( float *roll, float *pitch, float *yaw )
{
    if( roll == NULL || pitch == NULL || yaw == NULL )
        return ESP_ERR_INVALID_ARG;
    esp_err_t err;
    switch( _imu )
    {
        case BOARD_IMU_MPU6886: err = mpu6886_gyro_data_get( roll, pitch, yaw ); break;
        case BOARD_IMU_BMI270:  err = bmi270_gyro_data_get( roll, pitch, yaw ); break;
        default:                return ESP_ERR_INVALID_STATE;
    }
    if ( err == ESP_OK )
    {
        ESP_LOGV( _TAG, "gyro roll=%.3f pitch=%.3f yaw=%.3f dps", *roll, *pitch, *yaw );
    }
    return err;
}

esp_err_t core2foraws_motion_accel_gyro_get( float *x, float *y, float *z,
                                             float *roll, float *pitch,
                                             float *yaw )
{
    if( x == NULL || y == NULL || z == NULL ||
        roll == NULL || pitch == NULL || yaw == NULL )
        return ESP_ERR_INVALID_ARG;
    switch( _imu )
    {
        case BOARD_IMU_MPU6886:
            return mpu6886_accel_gyro_data_get( x, y, z, roll, pitch, yaw );
        case BOARD_IMU_BMI270:
            return bmi270_accel_gyro_data_get( x, y, z, roll, pitch, yaw );
        default:
            return ESP_ERR_INVALID_STATE;
    }
}

esp_err_t core2foraws_motion_accel_range_set( motion_accel_range_t range )
{
    if ( range < MOTION_ACCEL_RANGE_2G || range > MOTION_ACCEL_RANGE_16G )
    {
        return ESP_ERR_INVALID_ARG;
    }

    switch( _imu )
    {
        case BOARD_IMU_MPU6886: return mpu6886_fsr_accel_set( ( acc_scale_t )range );
        /* ACC_RANGE uses the same 2/4/8/16 G order. */
        case BOARD_IMU_BMI270:  return bmi270_acc_range_set( ( bmi270_acc_range_t )range );
        default:                return ESP_ERR_INVALID_STATE;
    }
}

esp_err_t core2foraws_motion_gyro_range_set( motion_gyro_range_t range )
{
    if ( range < MOTION_GYRO_RANGE_250DPS || range > MOTION_GYRO_RANGE_2000DPS )
    {
        return ESP_ERR_INVALID_ARG;
    }

    switch( _imu )
    {
        case BOARD_IMU_MPU6886: return mpu6886_fsr_gyro_set( ( gyro_scale_t )range );
        /* GYR_RANGE counts down from ±2000 °/s. */
        case BOARD_IMU_BMI270:
            return bmi270_gyr_range_set( ( bmi270_gyr_range_t )( MOTION_GYRO_RANGE_2000DPS - range ) );
        default:                return ESP_ERR_INVALID_STATE;
    }
}