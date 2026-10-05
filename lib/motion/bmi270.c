/*
 * BMI270 6-axis IMU (accelerometer + gyroscope) driver for the Core2 for AWS IoT Kit BSP.
 *
 * Written from the Bosch Sensortec BMI270 data sheet (BST-BMI270-DS000-05,
 * revision 1.3). No third-party driver source was referenced or copied.
 * Copyright (C) 2022 Rashed Talukder.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file bmi270.c
 * @brief BMI270 I2C driver implementation for the Core2 for AWS IoT Kit BSP.
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_timer.h>

#include "core2foraws_common.h"
#include "core2foraws_i2c.h"
#include "bmi270.h"

#define BMI270_I2C_SPEED_HZ          400000
/* Registers are accessible 450 us after POR or soft reset (section 4.4). */
#define BMI270_BOOT_MS               1
/* PWR_CONF.adv_power_save=0 takes effect after 450 us (section 4.4). */
#define BMI270_APS_OFF_MS            1
/* INTERNAL_STATUS reports init_ok at most 20 ms after INIT_CTRL=1. */
#define BMI270_INIT_TIMEOUT_MS       20
/* Data sheet section 4.4 footnote 4 allows chunked INIT_DATA writes. */
#define BMI270_CONFIG_CHUNK_BYTES    256
/* Section 1, table 1/2 start-up times from suspend to normal mode. */
#define BMI270_ACCEL_STARTUP_US      ( 2 * 1000 )
#define BMI270_GYRO_STARTUP_US       ( 45 * 1000 )

/* acc_filter_perf=1, acc_bwp=norm_avg4, acc_odr=200 Hz */
#define BMI270_ACC_CONF_VALUE        0xA9
/* gyr_filter_perf=1, gyr_noise_perf=1, gyr_bwp=normal, gyr_odr=200 Hz */
#define BMI270_GYR_CONF_VALUE        0xE9
/* temp_en | acc_en | gyr_en */
#define BMI270_PWR_CTRL_VALUE        0x0E

#define BMI270_TEMPERATURE_INVALID   ( ( int16_t )0x8000 )

static core2foraws_i2c_port_t _i2c_port;
static i2c_master_dev_handle_t _bmi270_dev;
static float _acc_res;
static float _gyr_res;
static int64_t _accel_ready_us;
static int64_t _gyro_ready_us;

static const char *TAG = "BMI270";

static void _wait_until( int64_t ready_us )
{
    int64_t remaining_us = ready_us - esp_timer_get_time();
    if( remaining_us > 0 )
    {
        vTaskDelay( CORE2FORAWS_DELAY_MS_TO_TICKS( ( remaining_us + 999 ) / 1000 ) );
    }
}

static esp_err_t read_reg( uint8_t reg, uint8_t num_bytes, uint8_t *buffer )
{
    esp_err_t err = core2foraws_i2c_read( _i2c_port, _bmi270_dev, reg,
                                          buffer, num_bytes );
    if( err != ESP_OK )
    {
        ESP_LOGE( TAG, "\tCould not read from register 0x%02x", reg );
    }
    return err;
}

static esp_err_t write_regs( uint8_t reg, const uint8_t *buffer,
                             uint16_t num_bytes )
{
    esp_err_t err = core2foraws_i2c_write( _i2c_port, _bmi270_dev, reg,
                                           buffer, num_bytes );
    if( err != ESP_OK )
    {
        ESP_LOGE( TAG, "\tCould not write %u bytes to register 0x%02x",
                  num_bytes, reg );
    }
    return err;
}

static esp_err_t write_reg( uint8_t reg, uint8_t value )
{
    return write_regs( reg, &value, 1 );
}

static float _acc_res_get( bmi270_acc_range_t range )
{
    return ( float )( 2 << range ) / 32768.0f;
}

static float _gyr_res_get( bmi270_gyr_range_t range )
{
    return ( float )( 2000 >> range ) / 32768.0f;
}

static esp_err_t _internal_status_get( uint8_t *message )
{
    uint8_t status = 0;
    esp_err_t err = read_reg( BMI270_INTERNAL_STATUS, 1, &status );
    if( err == ESP_OK )
    {
        *message = status & BMI270_INTERNAL_STATUS_MSG_MASK;
    }
    return err;
}

/* Section 4.4: soft reset, APS off, INIT_CTRL=0, INIT_DATA, INIT_CTRL=1. */
static esp_err_t _config_file_load( void )
{
    /* The interface is down during boot, so the write may not be acked. */
    ( void )core2foraws_i2c_write( _i2c_port, _bmi270_dev, BMI270_CMD,
                                   ( const uint8_t[] ){ BMI270_CMD_SOFTRESET },
                                   1 );
    vTaskDelay( CORE2FORAWS_DELAY_MS_TO_TICKS( BMI270_BOOT_MS ) );

    esp_err_t err = write_reg( BMI270_PWR_CONF, 0x00 );
    if( err != ESP_OK ) return err;
    vTaskDelay( CORE2FORAWS_DELAY_MS_TO_TICKS( BMI270_APS_OFF_MS ) );

    err = write_reg( BMI270_INIT_CTRL, 0x00 );
    if( err != ESP_OK ) return err;

    for( uint16_t offset = 0; offset < BMI270_CONFIG_FILE_SIZE;
         offset += BMI270_CONFIG_CHUNK_BYTES )
    {
        /* INIT_ADDR counts 16-bit words: bits 3..0 in ADDR_0, 11..4 in ADDR_1. */
        const uint16_t word = offset / 2;
        const uint8_t addr[ 2 ] = { word & 0x0F, ( uint8_t )( word >> 4 ) };
        err = write_regs( BMI270_INIT_ADDR_0, addr, sizeof( addr ) );
        if( err != ESP_OK ) return err;

        err = write_regs( BMI270_INIT_DATA, &bmi270_config_file[ offset ],
                          BMI270_CONFIG_CHUNK_BYTES );
        if( err != ESP_OK ) return err;
    }

    err = write_reg( BMI270_INIT_CTRL, 0x01 );
    if( err != ESP_OK ) return err;

    const int64_t deadline_us =
        esp_timer_get_time() + BMI270_INIT_TIMEOUT_MS * 1000;
    uint8_t message = 0;
    do
    {
        vTaskDelay( CORE2FORAWS_DELAY_MS_TO_TICKS( 1 ) );
        err = _internal_status_get( &message );
    } while( err == ESP_OK && message != BMI270_INTERNAL_STATUS_INIT_OK &&
             esp_timer_get_time() < deadline_us );

    if( err != ESP_OK ) return err;
    if( message != BMI270_INTERNAL_STATUS_INIT_OK )
    {
        ESP_LOGE( TAG, "\tConfiguration file rejected: INTERNAL_STATUS message 0x%x",
                  message );
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

esp_err_t bmi270_init( core2foraws_i2c_port_t port )
{
    if( _bmi270_dev == NULL )
    {
        _i2c_port = port;
        esp_err_t err = core2foraws_i2c_device_add( port, BMI270_ADDRESS,
                                                    BMI270_I2C_SPEED_HZ,
                                                    &_bmi270_dev );
        if( err != ESP_OK ) return err;
    }

    uint8_t chip_id = 0;
    esp_err_t err = read_reg( BMI270_CHIP_ID, 1, &chip_id );
    if( err != ESP_OK ) return err;
    if( chip_id != BMI270_CHIP_ID_VALUE )
    {
        ESP_LOGE( TAG, "\tUnexpected CHIP_ID: 0x%02x (expected 0x%02x)",
                  chip_id, BMI270_CHIP_ID_VALUE );
        return ESP_ERR_NOT_FOUND;
    }

    /* The configuration is volatile and survives only while the sensor stays
     * powered, so an ESP32-only reset can keep it. */
    uint8_t message = 0;
    err = _internal_status_get( &message );
    if( err != ESP_OK ) return err;
    if( message == BMI270_INTERNAL_STATUS_INIT_OK )
    {
        ESP_LOGD( TAG, "\tConfiguration file already loaded" );
        err = write_reg( BMI270_PWR_CONF, 0x00 );
        if( err != ESP_OK ) return err;
        vTaskDelay( CORE2FORAWS_DELAY_MS_TO_TICKS( BMI270_APS_OFF_MS ) );
    }
    else
    {
        const int64_t start_us = esp_timer_get_time();
        err = _config_file_load();
        if( err != ESP_OK ) return err;
        ESP_LOGI( TAG, "\tConfiguration file loaded in %lld ms",
                  ( esp_timer_get_time() - start_us ) / 1000 );
    }

    const uint8_t sensor_conf[][ 2 ] = {
        { BMI270_ACC_CONF, BMI270_ACC_CONF_VALUE },
        { BMI270_ACC_RANGE, BMI270_ACC_RANGE_8G },
        { BMI270_GYR_CONF, BMI270_GYR_CONF_VALUE },
        { BMI270_GYR_RANGE, BMI270_GYR_RANGE_2000DPS },
    };
    for( size_t i = 0; i < sizeof( sensor_conf ) / sizeof( sensor_conf[ 0 ] ); i++ )
    {
        err = write_reg( sensor_conf[ i ][ 0 ], sensor_conf[ i ][ 1 ] );
        if( err != ESP_OK ) return err;
    }
    _acc_res = _acc_res_get( BMI270_ACC_RANGE_8G );
    _gyr_res = _gyr_res_get( BMI270_GYR_RANGE_2000DPS );

    err = write_reg( BMI270_PWR_CTRL, BMI270_PWR_CTRL_VALUE );
    if( err != ESP_OK ) return err;

    const int64_t enabled_us = esp_timer_get_time();
    _accel_ready_us = enabled_us + BMI270_ACCEL_STARTUP_US;
    _gyro_ready_us = enabled_us + BMI270_GYRO_STARTUP_US;
    return ESP_OK;
}

esp_err_t bmi270_acc_range_set( bmi270_acc_range_t range )
{
    if( range < BMI270_ACC_RANGE_2G || range > BMI270_ACC_RANGE_16G )
        return ESP_ERR_INVALID_ARG;
    esp_err_t err = core2foraws_i2c_lock( _i2c_port );
    if( err != ESP_OK ) return err;

    err = write_reg( BMI270_ACC_RANGE, ( uint8_t )range );
    if( err == ESP_OK )
    {
        _acc_res = _acc_res_get( range );
    }

    esp_err_t unlock_err = core2foraws_i2c_unlock( _i2c_port );
    return err != ESP_OK ? err : unlock_err;
}

esp_err_t bmi270_gyr_range_set( bmi270_gyr_range_t range )
{
    if( range < BMI270_GYR_RANGE_2000DPS || range > BMI270_GYR_RANGE_125DPS )
        return ESP_ERR_INVALID_ARG;
    esp_err_t err = core2foraws_i2c_lock( _i2c_port );
    if( err != ESP_OK ) return err;

    err = write_reg( BMI270_GYR_RANGE, ( uint8_t )range );
    if( err == ESP_OK )
    {
        _gyr_res = _gyr_res_get( range );
    }

    esp_err_t unlock_err = core2foraws_i2c_unlock( _i2c_port );
    return err != ESP_OK ? err : unlock_err;
}

static inline int16_t _le16( const uint8_t *bytes )
{
    return ( int16_t )( ( uint16_t )bytes[ 0 ] | ( ( uint16_t )bytes[ 1 ] << 8 ) );
}

/* Reads DATA_8..DATA_19 and scales them under one bus lock. */
static esp_err_t _data_get( float accel[ 3 ], float gyro[ 3 ] )
{
    _wait_until( gyro != NULL ? _gyro_ready_us : _accel_ready_us );
    esp_err_t err = core2foraws_i2c_lock( _i2c_port );
    if( err != ESP_OK ) return err;

    uint8_t buf[ BMI270_DATA_NUM_BYTES ];
    err = read_reg( BMI270_DATA_ACC_X_LSB,
                    gyro != NULL ? BMI270_DATA_NUM_BYTES : 6, buf );
    if( err == ESP_OK )
    {
        for( int axis = 0; axis < 3; axis++ )
        {
            if( accel != NULL )
                accel[ axis ] = ( float )_le16( &buf[ axis * 2 ] ) * _acc_res;
            if( gyro != NULL )
                gyro[ axis ] = ( float )_le16( &buf[ 6 + axis * 2 ] ) * _gyr_res;
        }
    }

    esp_err_t unlock_err = core2foraws_i2c_unlock( _i2c_port );
    return err != ESP_OK ? err : unlock_err;
}

esp_err_t bmi270_accel_data_get( float *ax, float *ay, float *az )
{
    if( ax == NULL || ay == NULL || az == NULL ) return ESP_ERR_INVALID_ARG;
    float accel[ 3 ];
    esp_err_t err = _data_get( accel, NULL );
    if( err == ESP_OK )
    {
        *ax = accel[ 0 ];
        *ay = accel[ 1 ];
        *az = accel[ 2 ];
    }
    return err;
}

esp_err_t bmi270_gyro_data_get( float *gx, float *gy, float *gz )
{
    if( gx == NULL || gy == NULL || gz == NULL ) return ESP_ERR_INVALID_ARG;
    float gyro[ 3 ];
    esp_err_t err = _data_get( NULL, gyro );
    if( err == ESP_OK )
    {
        *gx = gyro[ 0 ];
        *gy = gyro[ 1 ];
        *gz = gyro[ 2 ];
    }
    return err;
}

esp_err_t bmi270_accel_gyro_data_get( float *ax, float *ay, float *az,
                                      float *gx, float *gy, float *gz )
{
    if( ax == NULL || ay == NULL || az == NULL ||
        gx == NULL || gy == NULL || gz == NULL )
        return ESP_ERR_INVALID_ARG;
    float accel[ 3 ], gyro[ 3 ];
    esp_err_t err = _data_get( accel, gyro );
    if( err == ESP_OK )
    {
        *ax = accel[ 0 ];
        *ay = accel[ 1 ];
        *az = accel[ 2 ];
        *gx = gyro[ 0 ];
        *gy = gyro[ 1 ];
        *gz = gyro[ 2 ];
    }
    return err;
}

esp_err_t bmi270_temp_data_get( float *t )
{
    if( t == NULL ) return ESP_ERR_INVALID_ARG;
    uint8_t buf[ 2 ];
    esp_err_t err = read_reg( BMI270_TEMPERATURE_0, sizeof( buf ), buf );
    if( err != ESP_OK ) return err;

    int16_t raw = _le16( buf );
    if( raw == BMI270_TEMPERATURE_INVALID ) return ESP_ERR_INVALID_STATE;
    /* 0x0000 is 23 °C with 1/512 K per LSB (TEMPERATURE_0 register). */
    *t = ( float )raw / 512.0f + 23.0f;
    return ESP_OK;
}
