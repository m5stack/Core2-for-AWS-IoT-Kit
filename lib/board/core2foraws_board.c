/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_board.c
 * @brief Core2 for AWS IoT Kit hardware revision detection
 */

#include <stdatomic.h>
#include <stdbool.h>
#include <esp_log.h>

#include "core2foraws_i2c.h"
#include "core2foraws_board_priv.h"

/* Both bottoms place their IMU at 0x68 (SDO/AD0 pulled low). */
#define BOARD_IMU_ADDRESS        0x68
#define BOARD_IMU_I2C_SPEED_HZ   400000
#define MPU6886_WHO_AM_I         0x75
#define MPU6886_WHO_AM_I_VALUE   0x19
#define BMI270_CHIP_ID           0x00
#define BMI270_CHIP_ID_VALUE     0x24

static const char *_TAG = "CORE2FORAWS_BOARD";

/* Guarded by the internal I2C bus lock. */
static bool _imu_probed;
static core2foraws_board_imu_t _imu;

static atomic_int _lcd = BOARD_LCD_UNKNOWN;

void core2foraws_board_lcd_set( core2foraws_board_lcd_t lcd )
{
    atomic_store( &_lcd, lcd );
}

/* WHO_AM_I first: 0x00 on the MPU6886 is a factory trim byte that could
 * read 0x24, while 0x75 is reserved on the BMI270. */
static core2foraws_board_imu_t _imu_probe( i2c_master_dev_handle_t dev )
{
    uint8_t id = 0;
    if( core2foraws_i2c_read( CORE2FORAWS_I2C_INTERNAL, dev, MPU6886_WHO_AM_I,
                              &id, 1 ) != ESP_OK )
    {
        return BOARD_IMU_NONE;
    }
    if( id == MPU6886_WHO_AM_I_VALUE )
    {
        return BOARD_IMU_MPU6886;
    }

    if( core2foraws_i2c_read( CORE2FORAWS_I2C_INTERNAL, dev, BMI270_CHIP_ID,
                              &id, 1 ) == ESP_OK &&
        id == BMI270_CHIP_ID_VALUE )
    {
        return BOARD_IMU_BMI270;
    }
    return BOARD_IMU_NONE;
}

static esp_err_t _imu_get( core2foraws_board_imu_t *imu )
{
    esp_err_t err = core2foraws_i2c_init( CORE2FORAWS_I2C_INTERNAL );
    if( err != ESP_OK ) return err;

    err = core2foraws_i2c_lock( CORE2FORAWS_I2C_INTERNAL );
    if( err != ESP_OK ) return err;

    if( !_imu_probed )
    {
        i2c_master_dev_handle_t dev = NULL;
        err = core2foraws_i2c_device_add( CORE2FORAWS_I2C_INTERNAL,
                                          BOARD_IMU_ADDRESS,
                                          BOARD_IMU_I2C_SPEED_HZ, &dev );
        if( err == ESP_OK )
        {
            _imu = _imu_probe( dev );
            _imu_probed = true;
            err = core2foraws_i2c_device_remove( dev );
        }
    }
    *imu = _imu;

    esp_err_t unlock_err = core2foraws_i2c_unlock( CORE2FORAWS_I2C_INTERNAL );
    return err != ESP_OK ? err : unlock_err;
}

esp_err_t core2foraws_board_info_get( core2foraws_board_info_t *info )
{
    if( info == NULL ) return ESP_ERR_INVALID_ARG;

    core2foraws_board_imu_t imu = BOARD_IMU_NONE;
    esp_err_t err = _imu_get( &imu );
    if( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "IMU probe failed: 0x%x", err );
        return err;
    }

    info->imu = imu;
    switch( imu )
    {
        case BOARD_IMU_MPU6886:
            info->model = BOARD_MODEL_CORE2FORAWS;
            info->model_name = "Core2 for AWS";
            info->imu_name = "MPU6886";
            info->microphone = BOARD_MIC_SPM1423;
            info->microphone_name = "SPM1423";
            break;
        case BOARD_IMU_BMI270:
            info->model = BOARD_MODEL_CORE2FORAWS_V1_3;
            info->model_name = "Core2 for AWS v1.3";
            info->imu_name = "BMI270";
            info->microphone = BOARD_MIC_LMD4737T261;
            info->microphone_name = "LMD4737T261";
            break;
        case BOARD_IMU_NONE:
        default:
            info->model = BOARD_MODEL_UNKNOWN;
            info->model_name = "Unknown";
            info->imu_name = "None";
            info->microphone = BOARD_MIC_UNKNOWN;
            info->microphone_name = "Unknown";
            break;
    }

    info->lcd = ( core2foraws_board_lcd_t )atomic_load( &_lcd );
    switch( info->lcd )
    {
        case BOARD_LCD_ILI9342C: info->lcd_name = "ILI9342C"; break;
        case BOARD_LCD_ILI9342E: info->lcd_name = "ILI9342E"; break;
        case BOARD_LCD_UNKNOWN:
        default: info->lcd_name = "Unknown"; break;
    }
    return ESP_OK;
}
