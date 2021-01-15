#include "freertos/FreeRTOS.h"
#include "i2c_device.h"
#include "mpu6886.h"

static I2CDevice_t mpu6886_device;
static gyro_scale_t gyro_scale = MPU6886_GFS_2000DPS;
static acc_scale_t acc_scale = MPU6886_AFS_8G;
static float acc_res, gyro_res;

static void MPU6886_I2CInit() {
    mpu6886_device = i2c_malloc_device(I2C_NUM_1, 21, 22, 400000, MPU6886_ADDRESS);
}

static void MPU6886_I2CReadBytes(uint8_t start_Addr, uint8_t number_Bytes, uint8_t *read_Buffer) {
    i2c_read_bytes(mpu6886_device, start_Addr, read_Buffer, number_Bytes);
}

static void MPU6886_I2CWriteBytes(uint8_t start_Addr, uint8_t number_Bytes, uint8_t *write_Buffer) {
    i2c_write_bytes(mpu6886_device, start_Addr, write_Buffer, number_Bytes);
}

int MPU6886_Init(void) {
    unsigned char tempdata[1];
    unsigned char regdata;
    MPU6886_I2CInit();

    MPU6886_I2CReadBytes(MPU6886_WHOAMI, 1, tempdata);
    if (tempdata[0] != 0x19) {
        return -1;
    }
    vTaskDelay(1);

    regdata = 0x00;
    MPU6886_I2CWriteBytes(MPU6886_PWR_MGMT_1, 1, &regdata);
    vTaskDelay(10);

    regdata = (0x01 << 7);
    MPU6886_I2CWriteBytes(MPU6886_PWR_MGMT_1, 1, &regdata);
    vTaskDelay(10);

    regdata = (0x01 << 0);
    MPU6886_I2CWriteBytes(MPU6886_PWR_MGMT_1, 1, &regdata);
    vTaskDelay(10);

    regdata = 0x10;
    MPU6886_I2CWriteBytes(MPU6886_ACCEL_CONFIG, 1, &regdata);
    vTaskDelay(1);

    regdata = 0x18;
    MPU6886_I2CWriteBytes(MPU6886_GYRO_CONFIG, 1, &regdata);
    vTaskDelay(1);

    regdata = 0x01;
    MPU6886_I2CWriteBytes(MPU6886_CONFIG, 1, &regdata);
    vTaskDelay(1);

    regdata = 0x05;
    MPU6886_I2CWriteBytes(MPU6886_SMPLRT_DIV, 1, &regdata);
    vTaskDelay(1);

    regdata = 0x00;
    MPU6886_I2CWriteBytes(MPU6886_INT_ENABLE, 1, &regdata);
    vTaskDelay(1);

    regdata = 0x00;
    MPU6886_I2CWriteBytes(MPU6886_ACCEL_CONFIG2, 1, &regdata);
    vTaskDelay(1);

    regdata = 0x00;
    MPU6886_I2CWriteBytes(MPU6886_USER_CTRL, 1, &regdata);
    vTaskDelay(1);

    regdata = 0x00;
    MPU6886_I2CWriteBytes(MPU6886_FIFO_EN, 1, &regdata);
    vTaskDelay(1);

    regdata = 0x22;
    MPU6886_I2CWriteBytes(MPU6886_INT_PIN_CFG, 1, &regdata);
    vTaskDelay(1);

    regdata = 0x01;
    MPU6886_I2CWriteBytes(MPU6886_INT_ENABLE, 1, &regdata);
    vTaskDelay(100);

    gyro_res = MPU6886_GetGyroRes(gyro_scale);
    acc_res = MPU6886_GetAccRes(acc_scale);
    return 0;
}

void MPU6886_GetAccelAdc(int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t buf[6];
    MPU6886_I2CReadBytes(MPU6886_ACCEL_XOUT_H, 6, buf);

    *ax = ((int16_t)buf[0] << 8) | buf[1];
    *ay = ((int16_t)buf[2] << 8) | buf[3];
    *az = ((int16_t)buf[4] << 8) | buf[5];
}

void MPU6886_GetGyroAdc(int16_t *gx, int16_t *gy, int16_t *gz) {
    uint8_t buf[6];
    MPU6886_I2CReadBytes(MPU6886_GYRO_XOUT_H, 6, buf);

    *gx = ((uint16_t)buf[0] << 8) | buf[1];
    *gy = ((uint16_t)buf[2] << 8) | buf[3];
    *gz = ((uint16_t)buf[4] << 8) | buf[5];
}

void MPU6886_GetTempAdc(int16_t *t) {
    uint8_t buf[2];
    MPU6886_I2CReadBytes(MPU6886_TEMP_OUT_H, 2, buf);
    *t = ((uint16_t)buf[0] << 8) | buf[1];
}

float MPU6886_GetGyroRes(gyro_scale_t scale) {
    switch (scale) {
        case MPU6886_GFS_250DPS:
            return 250.0 / 32768.0;
        case MPU6886_GFS_500DPS:
            return 500.0 / 32768.0;
        case MPU6886_GFS_1000DPS:
            return 1000.0 / 32768.0;
        case MPU6886_GFS_2000DPS:
        default:
            return 2000.0 / 32768.0;
    }
}

float MPU6886_GetAccRes(acc_scale_t scale) {
    switch (scale) {
        // Possible accelerometer scales (and their register bit settings) are:
        // 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11).
        // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
        case MPU6886_AFS_2G:
            return 2.0 / 32768.0;
        case MPU6886_AFS_4G:
            return 4.0 / 32768.0;
        case MPU6886_AFS_8G:
            return 8.0 / 32768.0;
        case MPU6886_AFS_16G:
        default:
            return 16.0 / 32768.0;
    }
}

void MPU6886_SetGyroFSR(gyro_scale_t scale) {
    unsigned char regdata;
    regdata = (scale << 3);
    MPU6886_I2CWriteBytes(MPU6886_GYRO_CONFIG, 1, &regdata);
    gyro_scale = scale;
    gyro_res = MPU6886_GetGyroRes(scale);
}

void MPU6886_SetAccelFSR(acc_scale_t scale) {
    unsigned char regdata;
    regdata = (scale << 3);
    MPU6886_I2CWriteBytes(MPU6886_ACCEL_CONFIG, 1, &regdata);
    vTaskDelay(10);
    acc_scale = scale;
    acc_res = MPU6886_GetAccRes(scale);
}

void MPU6886_GetAccelData(float *ax, float *ay, float *az) {
    int16_t accX = 0;
    int16_t accY = 0;
    int16_t accZ = 0;
    MPU6886_GetAccelAdc(&accX, &accY, &accZ);

    *ax = (float)accX * acc_res;
    *ay = (float)accY * acc_res;
    *az = (float)accZ * acc_res;
}

void MPU6886_GetGyroData(float *gx, float *gy, float *gz) {
    int16_t gyroX = 0;
    int16_t gyroY = 0;
    int16_t gyroZ = 0;
    MPU6886_GetGyroAdc(&gyroX, &gyroY, &gyroZ);

    *gx = (float)gyroX * gyro_res;
    *gy = (float)gyroY * gyro_res;
    *gz = (float)gyroZ * gyro_res;
}

void MPU6886_GetTempData(float *t) {

    int16_t temp = 0;
    MPU6886_GetTempAdc(&temp);
    *t = (float)temp / 326.8 + 25.0;
}
