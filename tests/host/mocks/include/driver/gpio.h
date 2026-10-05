/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include "esp_err.h"
#include <stdint.h>
typedef int gpio_num_t;
enum { GPIO_NUM_13 = 13, GPIO_NUM_14 = 14, GPIO_NUM_21 = 21, GPIO_NUM_22 = 22,
	   GPIO_NUM_26 = 26, GPIO_NUM_32 = 32, GPIO_NUM_33 = 33, GPIO_NUM_34 = 34, GPIO_NUM_36 = 36 };
enum { GPIO_INTR_DISABLE, GPIO_MODE_INPUT, GPIO_MODE_OUTPUT,
	   GPIO_PULLDOWN_DISABLE, GPIO_PULLDOWN_ENABLE, GPIO_PULLUP_DISABLE,
	   GPIO_PULLUP_ENABLE, GPIO_MODE_INPUT_OUTPUT_OD };
typedef struct { uint64_t pin_bit_mask; int intr_type, mode, pull_down_en, pull_up_en; } gpio_config_t;
esp_err_t gpio_reset_pin(int pin);
esp_err_t gpio_config(const gpio_config_t *config);
int gpio_get_level(gpio_num_t pin);
esp_err_t gpio_set_level(gpio_num_t pin, int level);