/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../../lib/motion/mpu6886.c"

static uint8_t registers[256];
static unsigned int lock_depth;
static bool require_atomic;
static esp_err_t lock_error, transfer_error;
static float *completed_axis;
static float expected_axis;
static int64_t fake_time_us;
static unsigned int reset_polls_until_clear;
static bool reset_never_clears;

int64_t esp_timer_get_time(void) { return fake_time_us; }

esp_err_t core2foraws_i2c_lock(core2foraws_i2c_port_t port)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL);
    if (lock_error != ESP_OK) return lock_error;
    lock_depth++;
    return ESP_OK;
}
esp_err_t core2foraws_i2c_unlock(core2foraws_i2c_port_t port)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && lock_depth > 0);
    if (require_atomic && transfer_error == ESP_OK)
    {
        assert((unsigned int)acc_scale == (registers[MPU6886_ACCEL_CONFIG] >> 3));
        assert((unsigned int)gyro_scale == (registers[MPU6886_GYRO_CONFIG] >> 3));
        if (completed_axis != NULL) assert(fabsf(*completed_axis - expected_axis) < 0.0001f);
    }
    lock_depth--;
    return ESP_OK;
}
esp_err_t core2foraws_i2c_device_add(core2foraws_i2c_port_t port, uint16_t address, uint32_t speed, i2c_master_dev_handle_t *handle)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && address == 0x68 && speed == 400000);
    *handle = (i2c_master_dev_handle_t)registers;
    return ESP_OK;
}
esp_err_t core2foraws_i2c_read(core2foraws_i2c_port_t port, i2c_master_dev_handle_t handle, uint32_t reg, uint8_t *data, uint16_t size)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && handle != NULL && reg + size <= sizeof(registers));
    if (require_atomic) assert(lock_depth == 1);
    if (transfer_error != ESP_OK) return transfer_error;
    memcpy(data, registers + reg, size);
    if (reg == MPU6886_PWR_MGMT_1 && reset_polls_until_clear > 0 &&
        --reset_polls_until_clear == 0)
        registers[MPU6886_PWR_MGMT_1] &= 0x7f;
    return ESP_OK;
}
esp_err_t core2foraws_i2c_write(core2foraws_i2c_port_t port, i2c_master_dev_handle_t handle, uint32_t reg, const uint8_t *data, uint16_t size)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && handle != NULL && reg + size <= sizeof(registers));
    if (require_atomic) assert(lock_depth == 1);
    if (transfer_error != ESP_OK) return transfer_error;
    memcpy(registers + reg, data, size);
    if (reg == MPU6886_PWR_MGMT_1 && (data[0] & 0x80))
        reset_polls_until_clear = reset_never_clears ? 0 : 2;
    return ESP_OK;
}
void vTaskDelay(TickType_t ticks)
{
    assert(ticks > 0 && lock_depth == 0);
    fake_time_us += (int64_t)ticks * 10000;
}

int main(void)
{
    registers[MPU6886_WHOAMI] = 0x19;
    assert(mpu6886_init(CORE2FORAWS_I2C_INTERNAL) == ESP_OK);
    assert(registers[MPU6886_PWR_MGMT_1] == 0x01 && reset_polls_until_clear == 0);
    /* Init leaves the gyro start-up wait to the first read. */
    assert(fake_time_us < 100000);
    float first_x, first_y, first_z;
    assert(mpu6886_gyro_data_get(&first_x, &first_y, &first_z) == ESP_OK);
    assert(fake_time_us >= _gyro_ready_us);
    const uint8_t samples[] = { 0x40, 0x00, 0xc0, 0x00, 0x00, 0x00 };
    memcpy(registers + MPU6886_ACCEL_XOUT_H, samples, sizeof(samples));
    memcpy(registers + MPU6886_GYRO_XOUT_H, samples, sizeof(samples));
    require_atomic = true;
    for (unsigned int range = 0; range < 4; ++range)
    {
        assert(mpu6886_fsr_accel_set((acc_scale_t)range) == ESP_OK);
        assert(mpu6886_fsr_gyro_set((gyro_scale_t)range) == ESP_OK);
        float axis_x = 0, axis_y = 0, axis_z = 0;
        completed_axis = &axis_x;
        expected_axis = (float)(1U << range);
        assert(mpu6886_accel_data_get(&axis_x, &axis_y, &axis_z) == ESP_OK);
        assert(axis_y == -expected_axis && axis_z == 0);
        expected_axis = 125.0f * (1U << range);
        assert(mpu6886_gyro_data_get(&axis_x, &axis_y, &axis_z) == ESP_OK);
        assert(axis_y == -expected_axis && axis_z == 0);
        completed_axis = NULL;
        float ax, ay, az, gx, gy, gz;
        assert(mpu6886_accel_gyro_data_get(&ax, &ay, &az, &gx, &gy, &gz) == ESP_OK);
        assert(ax == (float)(1U << range) && ay == -ax && az == 0);
        assert(gx == expected_axis && gy == -expected_axis && gz == 0);
    }
    float old_acc_res = acc_res, old_gyro_res = gyro_res;
    transfer_error = ESP_ERR_TIMEOUT;
    assert(mpu6886_fsr_accel_set(MPU6886_AFS_2G) == ESP_ERR_TIMEOUT);
    assert(mpu6886_fsr_gyro_set(MPU6886_GFS_250DPS) == ESP_ERR_TIMEOUT);
    assert(acc_res == old_acc_res && gyro_res == old_gyro_res && lock_depth == 0);
    float axis_x = 123, axis_y = 456, axis_z = 789;
    assert(mpu6886_accel_data_get(&axis_x, &axis_y, &axis_z) == ESP_ERR_TIMEOUT);
    assert(axis_x == 123 && axis_y == 456 && axis_z == 789 && lock_depth == 0);
    transfer_error = ESP_OK;
    lock_error = ESP_ERR_TIMEOUT;
    assert(mpu6886_gyro_data_get(&axis_x, &axis_y, &axis_z) == ESP_ERR_TIMEOUT);
    assert(mpu6886_fsr_accel_set(MPU6886_AFS_2G) == ESP_ERR_TIMEOUT);
    lock_error = ESP_OK;
    assert(mpu6886_fsr_accel_set((acc_scale_t)-1) == ESP_ERR_INVALID_ARG);
    assert(mpu6886_fsr_gyro_set((gyro_scale_t)4) == ESP_ERR_INVALID_ARG);
    assert(mpu6886_accel_data_get(NULL, &axis_y, &axis_z) == ESP_ERR_INVALID_ARG);
    assert(mpu6886_accel_gyro_data_get(&axis_x, &axis_y, &axis_z, NULL, &axis_y, &axis_z) == ESP_ERR_INVALID_ARG);
    assert(lock_depth == 0);

    /* A reset bit that never clears fails init instead of proceeding. */
    require_atomic = false;
    reset_never_clears = true;
    int64_t start = fake_time_us;
    assert(mpu6886_init(CORE2FORAWS_I2C_INTERNAL) == ESP_ERR_TIMEOUT);
    assert(fake_time_us - start >= 100000 && lock_depth == 0);
    puts("Motion register/cache atomicity, scaled-read locking, reset polling, start-up wait and burst-read tests passed");
    return 0;
}