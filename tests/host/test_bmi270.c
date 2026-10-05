/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../../lib/motion/bmi270.c"

static uint8_t registers[256];
static uint8_t loaded_config[BMI270_CONFIG_FILE_SIZE];
static unsigned int lock_depth;
static unsigned int softresets, config_bytes, init_ctrl_starts;
static bool config_rejected;
static esp_err_t transfer_error;
static int64_t fake_time_us;

int64_t esp_timer_get_time(void) { return fake_time_us; }

esp_err_t core2foraws_i2c_lock(core2foraws_i2c_port_t port)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL);
    lock_depth++;
    return ESP_OK;
}
esp_err_t core2foraws_i2c_unlock(core2foraws_i2c_port_t port)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && lock_depth > 0);
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
    if (transfer_error != ESP_OK) return transfer_error;
    memcpy(data, registers + reg, size);
    return ESP_OK;
}
esp_err_t core2foraws_i2c_write(core2foraws_i2c_port_t port, i2c_master_dev_handle_t handle, uint32_t reg, const uint8_t *data, uint16_t size)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && handle != NULL && size > 0);
    if (transfer_error != ESP_OK) return transfer_error;
    if (reg == BMI270_INIT_DATA)
    {
        /* INIT_DATA does not auto-increment; INIT_ADDR selects the word. */
        assert(registers[BMI270_INIT_CTRL] == 0x00 && registers[BMI270_PWR_CONF] == 0x00);
        size_t offset = ((size_t)registers[BMI270_INIT_ADDR_0 + 1] << 4 |
                         (registers[BMI270_INIT_ADDR_0] & 0x0F)) * 2;
        assert(offset + size <= sizeof(loaded_config));
        memcpy(loaded_config + offset, data, size);
        config_bytes += size;
        return ESP_OK;
    }
    assert(reg + size <= sizeof(registers));
    memcpy(registers + reg, data, size);
    if (reg == BMI270_CMD && data[0] == BMI270_CMD_SOFTRESET)
    {
        softresets++;
        registers[BMI270_INTERNAL_STATUS] = 0x00;
        registers[BMI270_PWR_CONF] = 0x03;
        registers[BMI270_PWR_CTRL] = 0x00;
        memset(loaded_config, 0, sizeof(loaded_config));
    }
    if (reg == BMI270_INIT_CTRL && data[0] == 0x01)
    {
        init_ctrl_starts++;
        bool match = memcmp(loaded_config, bmi270_config_file, sizeof(loaded_config)) == 0;
        registers[BMI270_INTERNAL_STATUS] = match && !config_rejected ? 0x01 : 0x02;
    }
    return ESP_OK;
}
void vTaskDelay(TickType_t ticks)
{
    assert(ticks > 0 && lock_depth == 0);
    fake_time_us += (int64_t)ticks * 10000;
}

static void power_on_reset(void)
{
    memset(registers, 0, sizeof(registers));
    registers[BMI270_CHIP_ID] = BMI270_CHIP_ID_VALUE;
    registers[BMI270_PWR_CONF] = 0x03;
    registers[BMI270_TEMPERATURE_0 + 1] = 0x80;
    softresets = config_bytes = init_ctrl_starts = 0;
}

int main(void)
{
    /* Cold start: the whole file goes through INIT_DATA once, then init_ok. */
    power_on_reset();
    assert(bmi270_init(CORE2FORAWS_I2C_INTERNAL) == ESP_OK);
    assert(softresets == 1 && init_ctrl_starts == 1);
    assert(config_bytes == BMI270_CONFIG_FILE_SIZE);
    assert(registers[BMI270_PWR_CONF] == 0x00 && registers[BMI270_PWR_CTRL] == 0x0E);
    assert(registers[BMI270_ACC_RANGE] == BMI270_ACC_RANGE_8G);
    assert(registers[BMI270_GYR_RANGE] == BMI270_GYR_RANGE_2000DPS);
    assert(lock_depth == 0);

    /* ESP32-only reset: init_ok survives, so the file is not loaded twice. */
    registers[BMI270_PWR_CONF] = 0x03;
    registers[BMI270_PWR_CTRL] = 0x00;
    softresets = config_bytes = init_ctrl_starts = 0;
    assert(bmi270_init(CORE2FORAWS_I2C_INTERNAL) == ESP_OK);
    assert(softresets == 0 && init_ctrl_starts == 0 && config_bytes == 0);
    assert(registers[BMI270_PWR_CONF] == 0x00 && registers[BMI270_PWR_CTRL] == 0x0E);

    /* A rejected file times out instead of running unconfigured. */
    power_on_reset();
    config_rejected = true;
    int64_t start = fake_time_us;
    assert(bmi270_init(CORE2FORAWS_I2C_INTERNAL) == ESP_ERR_TIMEOUT);
    assert(fake_time_us - start >= BMI270_INIT_TIMEOUT_MS * 1000 && init_ctrl_starts == 1);
    config_rejected = false;

    power_on_reset();
    registers[BMI270_CHIP_ID] = 0x19;
    assert(bmi270_init(CORE2FORAWS_I2C_INTERNAL) == ESP_ERR_NOT_FOUND);

    power_on_reset();
    assert(bmi270_init(CORE2FORAWS_I2C_INTERNAL) == ESP_OK);

    /* Gyro reads wait out the 45 ms start-up time. */
    float gx, gy, gz;
    assert(bmi270_gyro_data_get(&gx, &gy, &gz) == ESP_OK);
    assert(fake_time_us >= _gyro_ready_us);

    /* Little-endian data: +0.5 and -0.5 of full scale on X and Y. */
    const uint8_t samples[] = { 0x00, 0x40, 0x00, 0xC0, 0x00, 0x00,
                                0x00, 0x40, 0x00, 0xC0, 0x00, 0x00 };
    memcpy(registers + BMI270_DATA_ACC_X_LSB, samples, sizeof(samples));
    for (unsigned int range = 0; range < 4; ++range)
    {
        assert(bmi270_acc_range_set((bmi270_acc_range_t)range) == ESP_OK);
        assert(registers[BMI270_ACC_RANGE] == range);
        float ax, ay, az;
        assert(bmi270_accel_data_get(&ax, &ay, &az) == ESP_OK);
        assert(ax == (float)(1U << range) && ay == -ax && az == 0);
    }
    const float gyro_full_scale[] = { 2000, 1000, 500, 250, 125 };
    for (unsigned int range = 0; range < 5; ++range)
    {
        assert(bmi270_gyr_range_set((bmi270_gyr_range_t)range) == ESP_OK);
        assert(registers[BMI270_GYR_RANGE] == range);
        float ax, ay, az;
        assert(bmi270_accel_gyro_data_get(&ax, &ay, &az, &gx, &gy, &gz) == ESP_OK);
        assert(gx == gyro_full_scale[range] / 2 && gy == -gx && gz == 0);
        assert(ax == 8 && ay == -8 && az == 0);
    }

    /* Failed range writes keep the cached scale. */
    float old_acc_res = _acc_res, old_gyr_res = _gyr_res;
    transfer_error = ESP_ERR_TIMEOUT;
    assert(bmi270_acc_range_set(BMI270_ACC_RANGE_2G) == ESP_ERR_TIMEOUT);
    assert(bmi270_gyr_range_set(BMI270_GYR_RANGE_250DPS) == ESP_ERR_TIMEOUT);
    assert(_acc_res == old_acc_res && _gyr_res == old_gyr_res && lock_depth == 0);
    transfer_error = ESP_OK;
    assert(bmi270_acc_range_set((bmi270_acc_range_t)4) == ESP_ERR_INVALID_ARG);
    assert(bmi270_gyr_range_set((bmi270_gyr_range_t)5) == ESP_ERR_INVALID_ARG);

    /* 0x8000 marks a temperature conversion that has not completed. */
    float t = 0;
    assert(bmi270_temp_data_get(&t) == ESP_ERR_INVALID_STATE);
    registers[BMI270_TEMPERATURE_0] = 0x00;
    registers[BMI270_TEMPERATURE_0 + 1] = 0x02;
    assert(bmi270_temp_data_get(&t) == ESP_OK && fabsf(t - 24.0f) < 0.001f);
    assert(lock_depth == 0);

    puts("BMI270 config upload, retained config, CHIP_ID, range/scale, burst-read and temperature tests passed");
    return 0;
}
