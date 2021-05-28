/**
 * @file mpu6886.h
 * @brief Functions for the MPU6886 inertial measurement unit (IMU).
 */

#pragma once

#include "stdint.h"

#define MPU6886_ADDRESS           0x68 
#define MPU6886_WHOAMI            0x75
#define MPU6886_ACCEL_INTEL_CTRL  0x69
#define MPU6886_SMPLRT_DIV        0x19
#define MPU6886_INT_PIN_CFG       0x37
#define MPU6886_INT_ENABLE        0x38
#define MPU6886_ACCEL_XOUT_H      0x3B
#define MPU6886_ACCEL_XOUT_L      0x3C
#define MPU6886_ACCEL_YOUT_H      0x3D
#define MPU6886_ACCEL_YOUT_L      0x3E
#define MPU6886_ACCEL_ZOUT_H      0x3F
#define MPU6886_ACCEL_ZOUT_L      0x40

#define MPU6886_TEMP_OUT_H        0x41
#define MPU6886_TEMP_OUT_L        0x42

#define MPU6886_GYRO_XOUT_H       0x43
#define MPU6886_GYRO_XOUT_L       0x44
#define MPU6886_GYRO_YOUT_H       0x45
#define MPU6886_GYRO_YOUT_L       0x46
#define MPU6886_GYRO_ZOUT_H       0x47
#define MPU6886_GYRO_ZOUT_L       0x48

#define MPU6886_USER_CTRL         0x6A
#define MPU6886_PWR_MGMT_1        0x6B
#define MPU6886_PWR_MGMT_2        0x6C
#define MPU6886_CONFIG            0x1A
#define MPU6886_GYRO_CONFIG       0x1B
#define MPU6886_ACCEL_CONFIG      0x1C
#define MPU6886_ACCEL_CONFIG2     0x1D
#define MPU6886_FIFO_EN           0x23

/**
 * @brief List of possible accelerometer scalars in Gs.
 */
/* @[declare_mpu6886_acc_scale_t] */
typedef enum {
    MPU6886_AFS_2G = 0,
    MPU6886_AFS_4G,
    MPU6886_AFS_8G,
    MPU6886_AFS_16G
} acc_scale_t;
/* @[declare_mpu6886_acc_scale_t] */

/**
 * @brief List of possible gyroscope scalars in degrees per second.
 */
/* @[declare_mpu6886_gyro_scale_t] */
typedef enum {
    MPU6886_GFS_250DPS = 0,
    MPU6886_GFS_500DPS,
    MPU6886_GFS_1000DPS,
    MPU6886_GFS_2000DPS
} gyro_scale_t;
/* @[declare_mpu6886_gyro_scale_t] */

/**
 * @brief Initializes the MPU6886 over I2C.
 * 
 * @note The Core2ForAWS_Init() calls this function
 * when the hardware feature is enabled.
 *
 * @return 0 if successful, -1 otherwise.
 */
/* @[declare_mpu6886_init] */
int MPU6886_Init(void);
/* @[declare_mpu6886_init] */

/**
 * @brief Retrieves the accelerometer measurements from the 
 * 16-bit ADC on the MPU6886.
 * 
 * @param[out] ax The 16-bit accelerometer measurement in the X direction.
 * @param[out] ay The 16-bit accelerometer measurement in the Y direction.
 * @param[out] az The 16-bit accelerometer measurement in the Z direction.
 */
/* @[declare_mpu6886_getacceladc] */
void MPU6886_GetAccelAdc(int16_t *ax, int16_t *ay, int16_t *az);
/* @[declare_mpu6886_getacceladc] */

/**
 * @brief Retrieves the gyroscope measurements from the 16-bit ADC on 
 * the MPU6886.
 * 
 * @param[out] gx The 16-bit accelerometer measurement in the X direction.
 * @param[out] gy The 16-bit accelerometer measurement in the Y direction.
 * @param[out] gz The 16-bit accelerometer measurement in the Z direction.
 */
/* @[declare_mpu6886_getgyroadc] */
void MPU6886_GetGyroAdc(int16_t *gx, int16_t *gy, int16_t *gz);
/* @[declare_mpu6886_getgyroadc] */

/**
 * @brief Retrieves the internal temperature measurement from the 16-bit 
 * ADC on the MPU6886.
 * 
 * @param[out] t Temperature of the MPU6886 passed through the 16-bit ADC.
 */
/* @[declare_mpu6886_gettempadc] */
void MPU6886_GetTempAdc(int16_t *t);
/* @[declare_mpu6886_gettempadc] */

/**
 * @brief Retrieves the resolution of the gyroscope measurements on the 
 * MPU6886.
 * 
 * @param[in] scale The degrees per second scale of measurement.
 * 
 * @return The gyroscope resolution.
 */
/* @[declare_mpu6886_getgyrores] */
float MPU6886_GetGyroRes(gyro_scale_t scale);
/* @[declare_mpu6886_getgyrores] */

/**
 * @brief Retrieves the resolution of the accelerometer measurements on 
 * the MPU6886.
 * 
 * @param[in] scale The G's per second scale of measurement.
 * 
 * @return The accelerometer resolution.
 */
/* @[declare_mpu6886_getaccres] */
float MPU6886_GetAccRes(acc_scale_t scale);
/* @[declare_mpu6886_getaccres] */

/**
 * @brief Sets the full-scale range of the gyroscope on the MPU6886.
 * 
 * @param[in] scale The degrees per second scale of measurement.
 */
/* @[declare_mpu6886_setgyrofsr] */
void MPU6886_SetGyroFSR(gyro_scale_t scale);
/* @[declare_mpu6886_setgyrofsr] */

/**
 * @brief Sets the full-scale range of the accelerometer on the 
 * MPU6886.
 * 
 * @param[in] scale The G's per second scale of measurement.
 */
/* @[declare_mpu6886_setaccelfsr] */
void MPU6886_SetAccelFSR(acc_scale_t scale);
/* @[declare_mpu6886_setaccelfsr] */

/**
 * @brief Retrieves the accelerometer measurements from the MPU6886.
 * 
 * **Example:**
 * 
 * Get the accelerometer values for the X, Y, and Z direction.
 * @code{c}
 *  float accel_x = 0.00;
 *  float accel_y = 0.00;
 *  float accel_z = 0.00;
 *  MPU6886_GetAccelData(&accel_x, &accel_y, &accel_z);
 * @endcode
 * 
 * @param[out] ax The 16-bit accelerometer measurement in the X direction.
 * @param[out] ay The 16-bit accelerometer measurement in the Y direction.
 * @param[out] az The 16-bit accelerometer measurement in the Z direction.
 */
/* @[declare_mpu6886_getacceldata] */
void MPU6886_GetAccelData(float *ax, float *ay, float *az);
/* @[declare_mpu6886_getacceldata] */

/**
 * @brief Retrieves the gyroscope measurements from the MPU6886.
 * 
 * **Example:**
 * 
 * Get the gyroscope values for the Roll, Yaw, and Pitch direction.
 * @code{c}
 *  float g_roll = 0.00;
 *  float g_yaw = 0.00;
 *  float g_pitch = 0.00;
 *  MPU6886_GetGyroData(&g_roll, &g_yaw, &g_pitch);
 * @endcode
 * 
 * @param[out] roll The 16-bit gyroscope roll measurement.
 * @param[out] yaw The 16-bit gyroscope yaw measurement.
 * @param[out] pitch The 16-bit gyroscope pitch measurement.
 */
/* @[declare_mpu6886_getgyrodata] */
void MPU6886_GetGyroData(float *roll, float *yaw, float *pitch);
/* @[declare_mpu6886_getgyrodata] */

/**
 * @brief Retrieves the internal temperature measurement from the MPU6886.
 * 
 * **Example:**
 * 
 * Get the temperature reading of the MPU6886 temperature sensor.
 * @code{c}
 *  float temperature;
 *  MPU6886_GetTempData(&temperature);
 * @endcode
 * 
 * @param[out] t Temperature of the MPU6886.
 */
/* @[declare_mpu6886_gettempdata] */
void MPU6886_GetTempData(float *t);
/* @[declare_mpu6886_gettempdata] */
