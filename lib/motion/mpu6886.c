/*
 * MPU6886 6-axis IMU (accelerometer + gyroscope) driver for the Core2 for AWS IoT Kit BSP.
 *
 * Written from scratch based on the InvenSense MPU-6886 Product Specification (rev. 1.2).
 * No third-party source code was referenced or copied.
 * Copyright (C) 2022 Rashed Talukder.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file mpu6886.c
 * @brief MPU6886 I2C driver implementation for the Core2 for AWS IoT Kit BSP.
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_timer.h>

#include "core2foraws_common.h"
#include "core2foraws_i2c.h"
#include "mpu6886.h"

#define MPU6886_I2C_SPEED_HZ         400000
#define MPU6886_DEVICE_RESET_BIT     0x80
#define MPU6886_RESET_TIMEOUT_MS     100
/* Datasheet section 11.1 maximums, measured from leaving sleep */
#define MPU6886_ACCEL_STARTUP_US     ( 20 * 1000 )
#define MPU6886_GYRO_STARTUP_US      ( 100 * 1000 )

/* ------------------------------------------------------------------ */
/* Module state                                                       */
/* ------------------------------------------------------------------ */
static core2foraws_i2c_port_t _i2c_port;
static i2c_master_dev_handle_t _mpu6886_dev;
static gyro_scale_t gyro_scale = MPU6886_GFS_2000DPS;
static acc_scale_t  acc_scale  = MPU6886_AFS_8G;
static float acc_res, gyro_res;
static int64_t _accel_ready_us;
static int64_t _gyro_ready_us;

static const char *TAG = "MPU6886";

/* Blocks until esp_timer reaches ready_us, without holding the bus. */
static void _wait_until( int64_t ready_us )
{
    int64_t remaining_us = ready_us - esp_timer_get_time();
    if( remaining_us > 0 )
    {
        vTaskDelay( CORE2FORAWS_DELAY_MS_TO_TICKS( ( remaining_us + 999 ) / 1000 ) );
    }
}

/* ------------------------------------------------------------------ */
/* I2C helpers                                                        */
/* ------------------------------------------------------------------ */

static esp_err_t mpu6886_i2c_init( core2foraws_i2c_port_t port )
{
    if( _mpu6886_dev != NULL )
    {
        return ESP_OK;
    }
    _i2c_port = port;
    return core2foraws_i2c_device_add( port, MPU6886_ADDRESS,
                                       MPU6886_I2C_SPEED_HZ, &_mpu6886_dev );
}

static esp_err_t read_reg( uint8_t reg, uint8_t num_bytes,
                           uint8_t *read_buffer )
{
    esp_err_t res = core2foraws_i2c_read( _i2c_port, _mpu6886_dev,
                                          reg, read_buffer, num_bytes );
    if ( res != ESP_OK )
    {
        ESP_LOGE( TAG, "\tCould not read from register 0x%02x", reg );
    }
    return res;
}

static esp_err_t write_reg( uint8_t reg, const uint8_t value )
{
    esp_err_t res = core2foraws_i2c_write( _i2c_port, _mpu6886_dev,
                                           reg, &value, 1 );
    if ( res != ESP_OK )
    {
        ESP_LOGE( TAG, "\tCould not write 0x%02x to register 0x%02x",
                  value, reg );
    }
    return res;
}

/* ------------------------------------------------------------------ */
/* Initialization                                                     */
/* ------------------------------------------------------------------ */

esp_err_t mpu6886_init( core2foraws_i2c_port_t port )
{
    uint8_t device_id;
    esp_err_t err = mpu6886_i2c_init( port );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* Verify the device is an MPU6886 by reading WHO_AM_I register */
    err = read_reg( MPU6886_WHOAMI, 1, &device_id );
    if ( err != ESP_OK )
    {
        return err;
    }
    if ( device_id != 0x19 )
    {
        ESP_LOGE( TAG, "\tUnexpected WHO_AM_I: 0x%02x (expected 0x19)",
                  device_id );
        return ESP_ERR_NOT_FOUND;
    }

    /* Reset the device; DEVICE_RESET auto-clears on completion (datasheet
     * section 13.2). */
    err = write_reg( MPU6886_PWR_MGMT_1, MPU6886_DEVICE_RESET_BIT );
    if ( err != ESP_OK )
    {
        return err;
    }

    const int64_t reset_deadline_us =
        esp_timer_get_time() + MPU6886_RESET_TIMEOUT_MS * 1000;
    uint8_t pwr_mgmt_1 = MPU6886_DEVICE_RESET_BIT;
    do
    {
        vTaskDelay( CORE2FORAWS_DELAY_MS_TO_TICKS( 1 ) );
        /* Called directly so polling during reset does not log errors. */
        err = core2foraws_i2c_read( _i2c_port, _mpu6886_dev,
                                    MPU6886_PWR_MGMT_1, &pwr_mgmt_1, 1 );
    } while( ( err != ESP_OK || ( pwr_mgmt_1 & MPU6886_DEVICE_RESET_BIT ) ) &&
             esp_timer_get_time() < reset_deadline_us );
    if ( err != ESP_OK || ( pwr_mgmt_1 & MPU6886_DEVICE_RESET_BIT ) )
    {
        ESP_LOGE( TAG, "\tDevice reset did not complete within %d ms",
                  MPU6886_RESET_TIMEOUT_MS );
        return err != ESP_OK ? err : ESP_ERR_TIMEOUT;
    }

    /* Select the best available clock source (auto-select). This clears
     * SLEEP, which starts the accelerometer and gyroscope start-up timers. */
    err = write_reg( MPU6886_PWR_MGMT_1, 0x01 );
    if ( err != ESP_OK )
    {
        return err;
    }
    const int64_t wake_us = esp_timer_get_time();
    _accel_ready_us = wake_us + MPU6886_ACCEL_STARTUP_US;
    _gyro_ready_us = wake_us + MPU6886_GYRO_STARTUP_US;

    /* ACCEL_INTEL_CTRL (0x69): WoM (Wake-on-Motion) intelligence control.
     * Per the MPU-6886 application note, bit 1 must be written after every
     * device reset.  Setting it ensures accelerometer output values are not
     * clamped during internal WoM comparisons, giving full-range readings
     * at all times.  The INT pin is not connected in this hardware design,
     * but this write is still required for correct ADC output. */
    err = write_reg( MPU6886_ACCEL_INTEL_CTRL, 0x02 );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* Set accelerometer full-scale range: ±8 G */
    err = write_reg( MPU6886_ACCEL_CONFIG, ( MPU6886_AFS_8G << 3 ) );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* Set gyroscope full-scale range: ±2000 DPS */
    err = write_reg( MPU6886_GYRO_CONFIG, ( MPU6886_GFS_2000DPS << 3 ) );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* CONFIG (0x1A) DLPF_CFG=1: enables the digital low-pass filter.
     * Effect: gyroscope 3-dB bandwidth = 184 Hz, delay = 2.9 ms;
     *         internal gyro/accel sample rate (Fs) becomes 1 kHz.
     * (See MPU-6886 datasheet Table 1, DLPF_CFG register description.) */
    err = write_reg( MPU6886_CONFIG, 0x01 );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* Sample rate = 1 kHz / (1 + 5) ≈ 167 Hz */
    err = write_reg( MPU6886_SMPLRT_DIV, 0x05 );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* Disable all interrupts initially */
    err = write_reg( MPU6886_INT_ENABLE, 0x00 );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* Accelerometer DLPF: use default bandwidth */
    err = write_reg( MPU6886_ACCEL_CONFIG2, 0x00 );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* Disable FIFO and I2C master mode */
    err = write_reg( MPU6886_USER_CTRL, 0x00 );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* Disable FIFO for all sensor types */
    err = write_reg( MPU6886_FIFO_EN, 0x00 );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* Configure interrupt pin: latch until status register is read */
    err = write_reg( MPU6886_INT_PIN_CFG, 0x20 );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* Enable data-ready interrupt source.
     * Note: the INT pin is not wired in this hardware design (schema.yml),
     * so no physical interrupt fires.  The INT_STATUS register can still
     * be polled if needed; this setting is kept for completeness. */
    err = write_reg( MPU6886_INT_ENABLE, 0x01 );
    if ( err != ESP_OK )
    {
        return err;
    }

    /* Cache the resolution values for the configured scales */
    mpu6886_gyro_res_get( gyro_scale, &gyro_res );
    mpu6886_accel_res_get( acc_scale, &acc_res );

    return ESP_OK;
}

/* ------------------------------------------------------------------ */
/* Raw ADC reads                                                      */
/* ------------------------------------------------------------------ */

esp_err_t mpu6886_adc_accel_get( int16_t *ax, int16_t *ay, int16_t *az )
{
    uint8_t buf[ MPU6886_ADC_ACCEL_NUM_BYTES ];

    esp_err_t err = read_reg( MPU6886_ACCEL_XOUT_H,
                              MPU6886_ADC_ACCEL_NUM_BYTES, buf );
    if ( err != ESP_OK )
    {
        return err;
    }

    *ax = ( ( int16_t )buf[ 0 ] << 8 ) | buf[ 1 ];
    *ay = ( ( int16_t )buf[ 2 ] << 8 ) | buf[ 3 ];
    *az = ( ( int16_t )buf[ 4 ] << 8 ) | buf[ 5 ];

    return ESP_OK;
}

esp_err_t mpu6886_adc_gyro_get( int16_t *gx, int16_t *gy, int16_t *gz )
{
    uint8_t buf[ MPU6886_ADC_GYRO_NUM_BYTES ];

    esp_err_t err = read_reg( MPU6886_GYRO_XOUT_H,
                              MPU6886_ADC_GYRO_NUM_BYTES, buf );
    if ( err != ESP_OK )
    {
        return err;
    }

    *gx = ( ( int16_t )buf[ 0 ] << 8 ) | buf[ 1 ];
    *gy = ( ( int16_t )buf[ 2 ] << 8 ) | buf[ 3 ];
    *gz = ( ( int16_t )buf[ 4 ] << 8 ) | buf[ 5 ];

    return ESP_OK;
}

esp_err_t mpu6886_adc_temp_get( int16_t *t )
{
    uint8_t buf[ MPU6886_ADC_TEMP_NUM_BYTES ];

    esp_err_t err = read_reg( MPU6886_TEMP_OUT_H,
                              MPU6886_ADC_TEMP_NUM_BYTES, buf );
    if ( err != ESP_OK )
    {
        return err;
    }

    *t = ( ( int16_t )buf[ 0 ] << 8 ) | buf[ 1 ];

    return ESP_OK;
}

/* ------------------------------------------------------------------ */
/* Resolution helpers                                                 */
/* ------------------------------------------------------------------ */

esp_err_t mpu6886_gyro_res_get( gyro_scale_t scale, float *resolution )
{
    switch ( scale )
    {
        case MPU6886_GFS_250DPS:
            *resolution = 250.0f / 32768.0f;
            break;
        case MPU6886_GFS_500DPS:
            *resolution = 500.0f / 32768.0f;
            break;
        case MPU6886_GFS_1000DPS:
            *resolution = 1000.0f / 32768.0f;
            break;
        case MPU6886_GFS_2000DPS:
        default:
            *resolution = 2000.0f / 32768.0f;
            break;
    }

    return ESP_OK;
}

esp_err_t mpu6886_accel_res_get( acc_scale_t scale, float *resolution )
{
    switch ( scale )
    {
        case MPU6886_AFS_2G:
            *resolution = 2.0f / 32768.0f;
            break;
        case MPU6886_AFS_4G:
            *resolution = 4.0f / 32768.0f;
            break;
        case MPU6886_AFS_8G:
            *resolution = 8.0f / 32768.0f;
            break;
        case MPU6886_AFS_16G:
        default:
            *resolution = 16.0f / 32768.0f;
            break;
    }

    return ESP_OK;
}

/* ------------------------------------------------------------------ */
/* Full-scale range setters                                           */
/* ------------------------------------------------------------------ */

esp_err_t mpu6886_fsr_gyro_set( gyro_scale_t scale )
{
    if( scale < MPU6886_GFS_250DPS || scale > MPU6886_GFS_2000DPS )
        return ESP_ERR_INVALID_ARG;
    esp_err_t err = core2foraws_i2c_lock( _i2c_port );
    if( err != ESP_OK ) return err;
    uint8_t regdata = ( scale << 3 );

    err = write_reg( MPU6886_GYRO_CONFIG, regdata );
    if ( err == ESP_OK )
    {
        gyro_scale = scale;
        mpu6886_gyro_res_get( scale, &gyro_res );
    }

    esp_err_t unlock_err = core2foraws_i2c_unlock( _i2c_port );
    return err != ESP_OK ? err : unlock_err;
}

esp_err_t mpu6886_fsr_accel_set( acc_scale_t scale )
{
    if( scale < MPU6886_AFS_2G || scale > MPU6886_AFS_16G )
        return ESP_ERR_INVALID_ARG;
    esp_err_t err = core2foraws_i2c_lock( _i2c_port );
    if( err != ESP_OK ) return err;
    uint8_t regdata = ( scale << 3 );

    err = write_reg( MPU6886_ACCEL_CONFIG, regdata );
    if ( err == ESP_OK )
    {
        acc_scale = scale;
        mpu6886_accel_res_get( scale, &acc_res );
    }

    esp_err_t unlock_err = core2foraws_i2c_unlock( _i2c_port );
    return err != ESP_OK ? err : unlock_err;
}

/* ------------------------------------------------------------------ */
/* Scaled data reads                                                  */
/* ------------------------------------------------------------------ */

esp_err_t mpu6886_accel_data_get( float *ax, float *ay, float *az )
{
    if( ax == NULL || ay == NULL || az == NULL ) return ESP_ERR_INVALID_ARG;
    _wait_until( _accel_ready_us );
    esp_err_t err = core2foraws_i2c_lock( _i2c_port );
    if( err != ESP_OK ) return err;
    int16_t raw_x = 0, raw_y = 0, raw_z = 0;
    err = mpu6886_adc_accel_get( &raw_x, &raw_y, &raw_z );

    if ( err == ESP_OK )
    {
        *ax = ( float )raw_x * acc_res;
        *ay = ( float )raw_y * acc_res;
        *az = ( float )raw_z * acc_res;
    }

    esp_err_t unlock_err = core2foraws_i2c_unlock( _i2c_port );
    return err != ESP_OK ? err : unlock_err;
}

esp_err_t mpu6886_gyro_data_get( float *gx, float *gy, float *gz )
{
    if( gx == NULL || gy == NULL || gz == NULL ) return ESP_ERR_INVALID_ARG;
    _wait_until( _gyro_ready_us );
    esp_err_t err = core2foraws_i2c_lock( _i2c_port );
    if( err != ESP_OK ) return err;
    int16_t raw_x = 0, raw_y = 0, raw_z = 0;
    err = mpu6886_adc_gyro_get( &raw_x, &raw_y, &raw_z );

    if ( err == ESP_OK )
    {
        *gx = ( float )raw_x * gyro_res;
        *gy = ( float )raw_y * gyro_res;
        *gz = ( float )raw_z * gyro_res;
    }

    esp_err_t unlock_err = core2foraws_i2c_unlock( _i2c_port );
    return err != ESP_OK ? err : unlock_err;
}

esp_err_t mpu6886_accel_gyro_data_get( float *ax, float *ay, float *az,
                                       float *gx, float *gy, float *gz )
{
    if( ax == NULL || ay == NULL || az == NULL ||
        gx == NULL || gy == NULL || gz == NULL )
        return ESP_ERR_INVALID_ARG;
    _wait_until( _gyro_ready_us > _accel_ready_us ? _gyro_ready_us
                                                  : _accel_ready_us );
    esp_err_t err = core2foraws_i2c_lock( _i2c_port );
    if( err != ESP_OK ) return err;
    uint8_t buf[ MPU6886_ADC_ALL_NUM_BYTES ];
    err = read_reg( MPU6886_ACCEL_XOUT_H, MPU6886_ADC_ALL_NUM_BYTES, buf );

    if ( err == ESP_OK )
    {
        /* buf[ 6..7 ] holds TEMP_OUT between the accel and gyro blocks */
        *ax = ( float )( int16_t )( ( buf[ 0 ] << 8 ) | buf[ 1 ] ) * acc_res;
        *ay = ( float )( int16_t )( ( buf[ 2 ] << 8 ) | buf[ 3 ] ) * acc_res;
        *az = ( float )( int16_t )( ( buf[ 4 ] << 8 ) | buf[ 5 ] ) * acc_res;
        *gx = ( float )( int16_t )( ( buf[ 8 ] << 8 ) | buf[ 9 ] ) * gyro_res;
        *gy = ( float )( int16_t )( ( buf[ 10 ] << 8 ) | buf[ 11 ] ) * gyro_res;
        *gz = ( float )( int16_t )( ( buf[ 12 ] << 8 ) | buf[ 13 ] ) * gyro_res;
    }

    esp_err_t unlock_err = core2foraws_i2c_unlock( _i2c_port );
    return err != ESP_OK ? err : unlock_err;
}

esp_err_t mpu6886_temp_data_get( float *t )
{
    int16_t raw_temp = 0;
    esp_err_t err = mpu6886_adc_temp_get( &raw_temp );

    if ( err == ESP_OK )
    {
        *t = ( float )raw_temp / 326.8f + 25.0f;
    }

    return err;
}
