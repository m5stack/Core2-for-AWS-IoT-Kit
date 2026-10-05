/*
 * BMI270 6-axis IMU (accelerometer + gyroscope) driver for the Core2 for AWS IoT Kit BSP.
 *
 * Written from the Bosch Sensortec BMI270 data sheet (BST-BMI270-DS000-05,
 * revision 1.3). Only the configuration file in bmi270_config.c comes from
 * Bosch; it is BSD-3-Clause licensed.
 * Copyright (C) 2022 Rashed Talukder.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * Hardware connections (M5GO Bottom2 v1.3 AWS schematic, U1):
 *   - Internal I²C bus: SDA=GPIO21, SCL=GPIO22; the device is clocked at
 *     400 kHz (data sheet fast mode).
 *   - SDO pulled to GND via R5 (4.7 kΩ) → 7-bit I²C address 0x68.
 *   - INT1, INT2, and the OIS/AUX pins are not routed.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file bmi270.h
 * @brief BMI270 inertial measurement unit (IMU) driver for the
 *        Core2 for AWS IoT Kit v1.3 BSP.
 */

#ifndef __BMI270_H__
#define __BMI270_H__

#include <stdint.h>
#include <esp_err.h>

#include "core2foraws_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* I2C address and register map (data sheet section 5.2)              */
/* ------------------------------------------------------------------ */
#define BMI270_ADDRESS                  0x68
#define BMI270_CHIP_ID                  0x00
#define BMI270_CHIP_ID_VALUE            0x24
#define BMI270_DATA_ACC_X_LSB           0x0C
#define BMI270_INTERNAL_STATUS          0x21
#define BMI270_TEMPERATURE_0            0x22
#define BMI270_ACC_CONF                 0x40
#define BMI270_ACC_RANGE                0x41
#define BMI270_GYR_CONF                 0x42
#define BMI270_GYR_RANGE                0x43
#define BMI270_INIT_CTRL                0x59
#define BMI270_INIT_ADDR_0              0x5B
#define BMI270_INIT_DATA                0x5E
#define BMI270_PWR_CONF                 0x7C
#define BMI270_PWR_CTRL                 0x7D
#define BMI270_CMD                      0x7E

#define BMI270_CMD_SOFTRESET            0xB6
#define BMI270_INTERNAL_STATUS_MSG_MASK 0x0F
#define BMI270_INTERNAL_STATUS_INIT_OK  0x01

/* ACC_X_LSB (0x0C) through GYR_Z_MSB (0x17): accel then gyro, little-endian */
#define BMI270_DATA_NUM_BYTES           12

/** Size of the Bosch configuration file uploaded to INIT_DATA. */
#define BMI270_CONFIG_FILE_SIZE         8192

/** Bosch BMI270 configuration file (bmi270_config.c, BSD-3-Clause). */
extern const uint8_t bmi270_config_file[ BMI270_CONFIG_FILE_SIZE ];

/* ------------------------------------------------------------------ */
/* Range encodings                                                    */
/* ------------------------------------------------------------------ */

/**
 * @brief Accelerometer full-scale range (ACC_RANGE.acc_range).
 */
typedef enum {
    BMI270_ACC_RANGE_2G = 0,
    BMI270_ACC_RANGE_4G,
    BMI270_ACC_RANGE_8G,
    BMI270_ACC_RANGE_16G
} bmi270_acc_range_t;

/**
 * @brief Gyroscope full-scale range (GYR_RANGE.gyr_range).
 */
typedef enum {
    BMI270_GYR_RANGE_2000DPS = 0,
    BMI270_GYR_RANGE_1000DPS,
    BMI270_GYR_RANGE_500DPS,
    BMI270_GYR_RANGE_250DPS,
    BMI270_GYR_RANGE_125DPS
} bmi270_gyr_range_t;

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Initialize the BMI270 on the specified I2C bus.
 *
 * Verifies CHIP_ID. If INTERNAL_STATUS already reports init_ok (the sensor
 * stayed powered across an ESP32 reset), the configuration file is kept;
 * otherwise the device is soft-reset and the 8 KB configuration file is
 * uploaded. Then configures the accelerometer to ±8 G and the gyroscope to
 * ±2000 °/s at 200 Hz and enables both plus the temperature sensor. It does
 * not wait out the sensor start-up time; the first read does that instead.
 *
 * @param[in] port  I2C bus port the BMI270 is connected to.
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND for a wrong CHIP_ID,
 * ESP_ERR_TIMEOUT if the configuration file is not accepted, or an I2C error.
 */
esp_err_t bmi270_init( core2foraws_i2c_port_t port );

/**
 * @brief Set the accelerometer full-scale range.
 *
 * Register and cached conversion factor update atomically against scaled reads.
 * A failed transfer leaves the cached range unchanged.
 *
 * @param[in] range  Desired full-scale range.
 * @return ESP_OK on success.
 */
esp_err_t bmi270_acc_range_set( bmi270_acc_range_t range );

/**
 * @brief Set the gyroscope full-scale range.
 *
 * Register and cached conversion factor update atomically against scaled reads.
 * A failed transfer leaves the cached range unchanged.
 *
 * @param[in] range  Desired full-scale range.
 * @return ESP_OK on success.
 */
esp_err_t bmi270_gyr_range_set( bmi270_gyr_range_t range );

/**
 * @brief Read scaled accelerometer data in Gs.
 *
 * @param[out] ax  X-axis acceleration in Gs.
 * @param[out] ay  Y-axis acceleration in Gs.
 * @param[out] az  Z-axis acceleration in Gs.
 * @return ESP_OK on success.
 */
esp_err_t bmi270_accel_data_get( float *ax, float *ay, float *az );

/**
 * @brief Read scaled gyroscope data in degrees per second.
 *
 * @param[out] gx  X-axis angular rate (deg/s).
 * @param[out] gy  Y-axis angular rate (deg/s).
 * @param[out] gz  Z-axis angular rate (deg/s).
 * @return ESP_OK on success.
 */
esp_err_t bmi270_gyro_data_get( float *gx, float *gy, float *gz );

/**
 * @brief Read scaled accelerometer and gyroscope data from one sample.
 *
 * Reads all twelve data registers in a single burst; the data sheet
 * guarantees burst reads are not split across sensor updates.
 *
 * @param[out] ax  X-axis acceleration in Gs.
 * @param[out] ay  Y-axis acceleration in Gs.
 * @param[out] az  Z-axis acceleration in Gs.
 * @param[out] gx  X-axis angular rate (deg/s).
 * @param[out] gy  Y-axis angular rate (deg/s).
 * @param[out] gz  Z-axis angular rate (deg/s).
 * @return ESP_OK on success.
 */
esp_err_t bmi270_accel_gyro_data_get( float *ax, float *ay, float *az,
                                      float *gx, float *gy, float *gz );

/**
 * @brief Read the die temperature in degrees Celsius.
 *
 * @param[out] t  Temperature in °C.
 * @return ESP_OK on success, or ESP_ERR_INVALID_STATE before the first
 * temperature conversion completes.
 */
esp_err_t bmi270_temp_data_get( float *t );

#ifdef __cplusplus
}
#endif

#endif /* __BMI270_H__ */
