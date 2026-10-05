/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once

typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL (-1)
#define ESP_ERR_TIMEOUT 0x107
#define CORE2FORAWS_I2C_INTERNAL 0

esp_err_t core2foraws_init(void);
esp_err_t core2foraws_i2c_init(int bus);
esp_err_t core2foraws_power_init(void);
esp_err_t core2foraws_display_init(void);
esp_err_t core2foraws_button_init(void);
esp_err_t core2foraws_motion_init(void);
esp_err_t core2foraws_rtc_init(void);
esp_err_t core2foraws_crypto_init(void);
esp_err_t core2foraws_rgb_led_init(void);
esp_err_t core2foraws_wifi_init(void);
esp_err_t core2foraws_common_error(esp_err_t error);