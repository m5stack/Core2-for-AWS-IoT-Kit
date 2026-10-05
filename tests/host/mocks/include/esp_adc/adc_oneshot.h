/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include "esp_err.h"
typedef void *adc_oneshot_unit_handle_t;
enum { ADC_CHANNEL_0, ADC_ATTEN_DB_12, ADC_UNIT_1, ADC_BITWIDTH_12 };
typedef struct { int unit_id; } adc_oneshot_unit_init_cfg_t;
typedef struct { int atten, bitwidth; } adc_oneshot_chan_cfg_t;
esp_err_t adc_oneshot_new_unit(const adc_oneshot_unit_init_cfg_t *config, adc_oneshot_unit_handle_t *handle);
esp_err_t adc_oneshot_config_channel(adc_oneshot_unit_handle_t handle, int channel, const adc_oneshot_chan_cfg_t *config);
esp_err_t adc_oneshot_del_unit(adc_oneshot_unit_handle_t handle);
esp_err_t adc_oneshot_read(adc_oneshot_unit_handle_t handle, int channel, int *value);