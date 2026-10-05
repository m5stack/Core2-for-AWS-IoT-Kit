/*
 * MPU6886 6-axis IMU (accelerometer + gyroscope) driver for the Core2 for AWS IoT Kit BSP.
 *
 * Written from scratch based on the InvenSense MPU-6886 Product Specification (rev. 1.2).
 * No third-party source code was referenced or copied.
 * Copyright (C) 2022 Rashed Talukder.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * Hardware connections (from schema.yml for Core2 for AWS IoT Kit):
 *   - Internal I²C bus: SDA=GPIO21, SCL=GPIO22; the device is clocked at
 *     400 kHz (datasheet fast mode).
 *   - SA0/SDO pulled to GND via R5 (4.7 kΩ) → 7-bit I²C address 0x68.
 *   - The INT and CS signals are not routed in this hardware design.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file mpu6886.h
 * @brief MPU6886 inertial measurement unit (IMU) driver for the
 *        Core2 for AWS IoT Kit BSP.
 */

#ifndef __MPU6886_H__
#define __MPU6886_H__

#include <stdint.h>
#include <esp_err.h>

#include "core2foraws_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* I2C address and register map                                       */
/* ------------------------------------------------------------------ */
#define MPU6886_ADDRESS                 0x68
#define MPU6886_WHOAMI                  0x75
#define MPU6886_ACCEL_INTEL_CTRL        0x69
#define MPU6886_SMPLRT_DIV              0x19
#define MPU6886_INT_PIN_CFG             0x37
#define MPU6886_INT_ENABLE              0x38

#define MPU6886_ACCEL_XOUT_H            0x3B
#define MPU6886_ACCEL_XOUT_L            0x3C
#define MPU6886_ACCEL_YOUT_H            0x3D
#define MPU6886_ACCEL_YOUT_L            0x3E
#define MPU6886_ACCEL_ZOUT_H            0x3F
#define MPU6886_ACCEL_ZOUT_L            0x40

#define MPU6886_TEMP_OUT_H              0x41
#define MPU6886_TEMP_OUT_L              0x42

#define MPU6886_GYRO_XOUT_H             0x43
#define MPU6886_GYRO_XOUT_L             0x44
#define MPU6886_GYRO_YOUT_H             0x45
#define MPU6886_GYRO_YOUT_L             0x46
#define MPU6886_GYRO_ZOUT_H             0x47
#define MPU6886_GYRO_ZOUT_L             0x48

#define MPU6886_USER_CTRL               0x6A
#define MPU6886_PWR_MGMT_1              0x6B
#define MPU6886_PWR_MGMT_2              0x6C
#define MPU6886_CONFIG                  0x1A
#define MPU6886_GYRO_CONFIG             0x1B
#define MPU6886_ACCEL_CONFIG            0x1C
#define MPU6886_ACCEL_CONFIG2           0x1D
#define MPU6886_FIFO_EN                 0x23

/* Number of raw bytes per sensor burst read */
#define MPU6886_ADC_ACCEL_NUM_BYTES     6
#define MPU6886_ADC_GYRO_NUM_BYTES      6
#define MPU6886_ADC_TEMP_NUM_BYTES      2
/* ACCEL_XOUT_H (0x3B) through GYRO_ZOUT_L (0x48), including temperature */
#define MPU6886_ADC_ALL_NUM_BYTES       14

/* ------------------------------------------------------------------ */
/* Scale enumerations                                                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Accelerometer full-scale range options (in Gs).
 */
typedef enum {
    MPU6886_AFS_2G = 0,
    MPU6886_AFS_4G,
    MPU6886_AFS_8G,
    MPU6886_AFS_16G
} acc_scale_t;

/**
 * @brief Gyroscope full-scale range options (in degrees per second).
 */
typedef enum {
    MPU6886_GFS_250DPS = 0,
    MPU6886_GFS_500DPS,
    MPU6886_GFS_1000DPS,
    MPU6886_GFS_2000DPS
} gyro_scale_t;

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Initialize the MPU6886 on the specified I2C bus.
 *
 * Resets the device, verifies the WHO_AM_I register, configures the
 * accelerometer to ±8 G and the gyroscope to ±2000 DPS, and enables
 * the data-ready interrupt. It does not wait out the sensor start-up
 * time; the first accelerometer or gyroscope read does that instead.
 *
 * @param[in] port  I2C bus port the MPU6886 is connected to.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t mpu6886_init( core2foraws_i2c_port_t port );

/**
 * @brief Read raw 16-bit accelerometer values.
 *
 * @param[out] ax  Raw X-axis accelerometer value.
 * @param[out] ay  Raw Y-axis accelerometer value.
 * @param[out] az  Raw Z-axis accelerometer value.
 * @return ESP_OK on success.
 */
esp_err_t mpu6886_adc_accel_get( int16_t *ax, int16_t *ay, int16_t *az );

/**
 * @brief Read raw 16-bit gyroscope values.
 *
 * @param[out] gx  Raw X-axis gyroscope value.
 * @param[out] gy  Raw Y-axis gyroscope value.
 * @param[out] gz  Raw Z-axis gyroscope value.
 * @return ESP_OK on success.
 */
esp_err_t mpu6886_adc_gyro_get( int16_t *gx, int16_t *gy, int16_t *gz );

/**
 * @brief Read the raw 16-bit internal temperature value.
 *
 * @param[out] t  Raw temperature ADC value.
 * @return ESP_OK on success.
 */
esp_err_t mpu6886_adc_temp_get( int16_t *t );

/**
 * @brief Get the resolution (units-per-LSB) for a gyroscope scale.
 *
 * @param[in]  scale       Gyroscope full-scale range.
 * @param[out] resolution  Degrees-per-second per ADC count.
 * @return ESP_OK on success.
 */
esp_err_t mpu6886_gyro_res_get( gyro_scale_t scale, float *resolution );

/**
 * @brief Get the resolution (units-per-LSB) for an accelerometer scale.
 *
 * @param[in]  scale       Accelerometer full-scale range.
 * @param[out] resolution  Gs per ADC count.
 * @return ESP_OK on success.
 */
esp_err_t mpu6886_accel_res_get( acc_scale_t scale, float *resolution );

/**
 * @brief Set the gyroscope full-scale range.
 *
 * Register and cached conversion factor update atomically against scaled reads.
 * A failed transfer leaves the cached range unchanged. Invalid ranges are rejected.
 *
 * @param[in] scale  Desired full-scale range.
 * @return ESP_OK on success.
 */
esp_err_t mpu6886_fsr_gyro_set( gyro_scale_t scale );

/**
 * @brief Set the accelerometer full-scale range.
 *
 * Register and cached conversion factor update atomically against scaled reads.
 * A failed transfer leaves the cached range unchanged. Invalid ranges are rejected.
 *
 * @param[in] scale  Desired full-scale range.
 * @return ESP_OK on success.
 */
esp_err_t mpu6886_fsr_accel_set( acc_scale_t scale );

/**
 * @brief Read scaled accelerometer data in Gs.
 *
 * Holds bus ownership from the register read through conversion. Range changes
 * cannot interleave with conversion; sensor settling after a change still applies.
 *
 * @param[out] ax  X-axis acceleration in Gs.
 * @param[out] ay  Y-axis acceleration in Gs.
 * @param[out] az  Z-axis acceleration in Gs.
 * @return ESP_OK on success.
 */
esp_err_t mpu6886_accel_data_get( float *ax, float *ay, float *az );

/**
 * @brief Read scaled gyroscope data in degrees per second.
 *
 * Holds bus ownership from the register read through conversion. Range changes
 * cannot interleave with conversion; sensor settling after a change still applies.
 *
 * @param[out] gx  X-axis angular rate (deg/s).
 * @param[out] gy  Y-axis angular rate (deg/s).
 * @param[out] gz  Z-axis angular rate (deg/s).
 * @return ESP_OK on success.
 */
esp_err_t mpu6886_gyro_data_get( float *gx, float *gy, float *gz );

/**
 * @brief Read scaled accelerometer and gyroscope data from one sample.
 *
 * Reads all output registers in a single 14-byte burst, so the six values
 * come from the same sampling instant.
 *
 * @param[out] ax  X-axis acceleration in Gs.
 * @param[out] ay  Y-axis acceleration in Gs.
 * @param[out] az  Z-axis acceleration in Gs.
 * @param[out] gx  X-axis angular rate (deg/s).
 * @param[out] gy  Y-axis angular rate (deg/s).
 * @param[out] gz  Z-axis angular rate (deg/s).
 * @return ESP_OK on success.
 */
esp_err_t mpu6886_accel_gyro_data_get( float *ax, float *ay, float *az,
                                       float *gx, float *gy, float *gz );

/**
 * @brief Read the internal temperature in degrees Celsius.
 *
 * @param[out] t  Temperature in °C.
 * @return ESP_OK on success.
 */
esp_err_t mpu6886_temp_data_get( float *t );

#ifdef __cplusplus
}
#endif

#endif /* __MPU6886_H__ */
