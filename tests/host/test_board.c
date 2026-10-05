/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../lib/board/core2foraws_board.c"

static uint8_t registers[256];
static bool imu_present;
static unsigned int lock_depth, probes, devices;

esp_err_t core2foraws_i2c_init(core2foraws_i2c_port_t port)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL);
    return ESP_OK;
}
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
    assert(lock_depth == 1);
    devices++;
    probes++;
    *handle = (i2c_master_dev_handle_t)registers;
    return ESP_OK;
}
esp_err_t core2foraws_i2c_device_remove(i2c_master_dev_handle_t handle)
{
    assert(handle == (i2c_master_dev_handle_t)registers && devices > 0);
    devices--;
    return ESP_OK;
}
esp_err_t core2foraws_i2c_read(core2foraws_i2c_port_t port, i2c_master_dev_handle_t handle, uint32_t reg, uint8_t *data, uint16_t size)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && handle != NULL && size == 1 && lock_depth == 1);
    if (!imu_present) return ESP_FAIL;
    *data = registers[reg];
    return ESP_OK;
}

static core2foraws_board_info_t detect(bool present, uint8_t reg00, uint8_t reg75)
{
    _imu_probed = false;
    imu_present = present;
    memset(registers, 0, sizeof(registers));
    registers[0x00] = reg00;
    registers[0x75] = reg75;
    core2foraws_board_info_t info;
    memset(&info, 0xA5, sizeof(info));
    assert(core2foraws_board_info_get(&info) == ESP_OK);
    assert(lock_depth == 0 && devices == 0);
    assert(info.model_name && info.imu_name && info.microphone_name && info.lcd_name);
    return info;
}

int main(void)
{
    core2foraws_board_info_t info = detect(true, 0x00, 0x19);
    assert(info.model == BOARD_MODEL_CORE2FORAWS && info.imu == BOARD_IMU_MPU6886);
    assert(info.microphone == BOARD_MIC_SPM1423 && strcmp(info.model_name, "Core2 for AWS") == 0);

    /* MPU6886 register 0x00 is a trim byte; WHO_AM_I wins over a 0x24 there. */
    info = detect(true, 0x24, 0x19);
    assert(info.imu == BOARD_IMU_MPU6886);

    info = detect(true, 0x24, 0x00);
    assert(info.model == BOARD_MODEL_CORE2FORAWS_V1_3 && info.imu == BOARD_IMU_BMI270);
    assert(info.microphone == BOARD_MIC_LMD4737T261);
    assert(strcmp(info.model_name, "Core2 for AWS v1.3") == 0 && strcmp(info.imu_name, "BMI270") == 0);

    info = detect(true, 0x00, 0x00);
    assert(info.model == BOARD_MODEL_UNKNOWN && info.imu == BOARD_IMU_NONE);
    info = detect(false, 0x24, 0x19);
    assert(info.model == BOARD_MODEL_UNKNOWN && info.microphone == BOARD_MIC_UNKNOWN);

    /* The probe runs once; later calls reuse the cached IMU. */
    detect(true, 0x24, 0x00);
    unsigned int probes_before = probes;
    imu_present = false;
    assert(core2foraws_board_info_get(&info) == ESP_OK && info.imu == BOARD_IMU_BMI270);
    assert(probes == probes_before);

    assert(info.lcd == BOARD_LCD_UNKNOWN && strcmp(info.lcd_name, "Unknown") == 0);
    core2foraws_board_lcd_set(BOARD_LCD_ILI9342E);
    assert(core2foraws_board_info_get(&info) == ESP_OK);
    assert(info.lcd == BOARD_LCD_ILI9342E && strcmp(info.lcd_name, "ILI9342E") == 0);

    assert(core2foraws_board_info_get(NULL) == ESP_ERR_INVALID_ARG);
    puts("Board IMU identification, probe caching and LCD reporting tests passed");
    return 0;
}
