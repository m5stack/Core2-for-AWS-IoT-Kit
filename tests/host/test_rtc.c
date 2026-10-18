/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "core2foraws_i2c.h"
#define COMMON_I2C_INTERNAL CORE2FORAWS_I2C_INTERNAL
#define CONFIG_TIME_ZONE "UTC0"
#include "../../lib/rtc/core2foraws_rtc.c"

static uint8_t registers[16];
static bool allow_i2c;
static bool saw_stop_during_time_write;

esp_err_t core2foraws_i2c_device_add(core2foraws_i2c_port_t port, uint16_t address, uint32_t speed, i2c_master_dev_handle_t *device)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && address == 0x51 && speed == 400000);
    if (!allow_i2c) return ESP_FAIL;
    *device = (i2c_master_dev_handle_t)registers;
    return ESP_OK;
}
esp_err_t core2foraws_i2c_read(core2foraws_i2c_port_t port, i2c_master_dev_handle_t device, uint32_t reg, uint8_t *buffer, uint16_t size)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && device != NULL && reg + size <= sizeof(registers));
    if (!allow_i2c) return ESP_FAIL;
    memcpy(buffer, registers + reg, size);
    return ESP_OK;
}
esp_err_t core2foraws_i2c_write(core2foraws_i2c_port_t port, i2c_master_dev_handle_t device, uint32_t reg, const uint8_t *buffer, uint16_t size)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && device != NULL && reg + size <= sizeof(registers));
    assert(allow_i2c && "Invalid input must not reach I2C");
    if (reg == BM8563_REG_CTRL_STATUS2)
    {
        /* BM8563 write-0-to-clear semantics: writing 0 clears AF/TF, writing 1 preserves. */
        uint8_t val = buffer[0];
        uint8_t af_tf = registers[BM8563_REG_CTRL_STATUS2] & (BM8563_CTRL2_AF | BM8563_CTRL2_TF);
        af_tf &= val & (BM8563_CTRL2_AF | BM8563_CTRL2_TF);
        registers[BM8563_REG_CTRL_STATUS2] = (val & ~(BM8563_CTRL2_AF | BM8563_CTRL2_TF)) | af_tf;
        return ESP_OK;
    }
    if (reg == BM8563_REG_SECONDS && size == 7)
    {
        saw_stop_during_time_write = (registers[BM8563_REG_CTRL_STATUS1] & BM8563_CTRL1_STOP) != 0;
    }
    memcpy(registers + reg, buffer, size);
    return ESP_OK;
}
esp_err_t core2foraws_i2c_lock(core2foraws_i2c_port_t port) { (void)port; return ESP_OK; }
esp_err_t core2foraws_i2c_unlock(core2foraws_i2c_port_t port) { (void)port; return ESP_OK; }

int main(void)
{
    struct tm date = { .tm_year = 0, .tm_mon = 0, .tm_mday = 1 };
    assert(_tm_utc_to_epoch(&date) == (time_t)INT64_C(-2208988800));
    date.tm_year = 70;
    assert(_tm_utc_to_epoch(&date) == 0);
    date = (struct tm){ .tm_year = 100, .tm_mon = 1, .tm_mday = 29 };
    assert(_tm_utc_to_epoch(&date) == (time_t)INT64_C(951782400));
    date = (struct tm){ .tm_year = 138, .tm_mon = 0, .tm_mday = 19,
                        .tm_hour = 3, .tm_min = 14, .tm_sec = 8 };
    assert(_tm_utc_to_epoch(&date) == (time_t)INT64_C(2147483648));
    date = (struct tm){ .tm_year = 199, .tm_mon = 11, .tm_mday = 31,
                        .tm_hour = 23, .tm_min = 59, .tm_sec = 59 };
    assert(_tm_utc_to_epoch(&date) == (time_t)INT64_C(4102444799));
    date.tm_year = INT_MAX;
    assert(core2foraws_rtc_utc_time_set(date) == ESP_ERR_INVALID_ARG);
    date.tm_year = INT_MIN;
    assert(core2foraws_rtc_utc_time_set(date) == ESP_ERR_INVALID_ARG);
    /* Non-leap Feb 29 and April 31 rejected before I2C */
    date = (struct tm){ .tm_year = 125, .tm_mon = 1, .tm_mday = 29 };
    assert(core2foraws_rtc_utc_time_set(date) == ESP_ERR_INVALID_ARG);
    date = (struct tm){ .tm_year = 126, .tm_mon = 3, .tm_mday = 31 };
    assert(core2foraws_rtc_utc_time_set(date) == ESP_ERR_INVALID_ARG);
    date = (struct tm){ .tm_year = 126, .tm_mon = 9, .tm_mday = 17 };
    assert(core2foraws_rtc_time_set(date) == ESP_ERR_INVALID_STATE);

    allow_i2c = true;
    registers[BM8563_REG_CLKOUT_FREQ] = 0x80;
    registers[BM8563_REG_SECONDS] = BM8563_SECONDS_VL;
    assert(core2foraws_rtc_init() == ESP_OK);
    assert(registers[BM8563_REG_CLKOUT_FREQ] == 0x00);
    assert(core2foraws_rtc_utc_time_set(date) == ESP_OK);
    assert(saw_stop_during_time_write);
    assert((registers[BM8563_REG_CTRL_STATUS1] & BM8563_CTRL1_STOP) == 0);
    assert((registers[BM8563_REG_SECONDS] & BM8563_SECONDS_VL) == 0);

    /* Clearing AF must not clear a concurrent TF (and vice versa). */
    registers[BM8563_REG_CTRL_STATUS2] = BM8563_CTRL2_AF | BM8563_CTRL2_TF;
    bool triggered = false;
    assert(core2foraws_rtc_alarm_status(&triggered, true) == ESP_OK && triggered);
    assert((registers[BM8563_REG_CTRL_STATUS2] & BM8563_CTRL2_AF) == 0);
    assert((registers[BM8563_REG_CTRL_STATUS2] & BM8563_CTRL2_TF) != 0);
    assert(core2foraws_rtc_timer_status(&triggered, true) == ESP_OK && triggered);
    assert((registers[BM8563_REG_CTRL_STATUS2] & BM8563_CTRL2_TF) == 0);

    puts("RTC 1900/1970/leap-day/2038/2099 epoch, calendar validation, STOP framing, CLKOUT and AF/TF tests passed");
    return 0;
}