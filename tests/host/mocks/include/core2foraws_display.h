/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include <stdint.h>
#include "esp_err.h"

typedef struct
{
    uint16_t x;
    uint16_t y;
} esp_lcd_touch_point_data_t;

esp_err_t core2foraws_display_touch_data_get(esp_lcd_touch_point_data_t *data,
                                             uint8_t *touch_cnt,
                                             uint8_t max_points);
