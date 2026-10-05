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

esp_err_t core2foraws_i2c_device_add(core2foraws_i2c_port_t port, uint16_t address, uint32_t speed, i2c_master_dev_handle_t *device)
{
    (void)port; (void)address; (void)speed; (void)device;
    return ESP_FAIL;
}
esp_err_t core2foraws_i2c_read(core2foraws_i2c_port_t port, i2c_master_dev_handle_t device, uint32_t reg, uint8_t *buffer, uint16_t size)
{
    (void)port; (void)device; (void)reg; (void)buffer; (void)size;
    return ESP_FAIL;
}
esp_err_t core2foraws_i2c_write(core2foraws_i2c_port_t port, i2c_master_dev_handle_t device, uint32_t reg, const uint8_t *buffer, uint16_t size)
{
    (void)port; (void)device; (void)reg; (void)buffer; (void)size;
    assert(!"Invalid year must not reach I2C");
    return ESP_FAIL;
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
    puts("RTC 1900/1970/leap-day/2038/2099 epoch and year-boundary tests passed");
    return 0;
}