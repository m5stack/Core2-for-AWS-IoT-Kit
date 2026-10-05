/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include "esp_err.h"
typedef void *adc_cali_handle_t;
esp_err_t adc_cali_raw_to_voltage(adc_cali_handle_t handle, int raw, int *voltage);