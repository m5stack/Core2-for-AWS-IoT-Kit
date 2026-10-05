/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include "esp_err.h"
typedef void *dac_oneshot_handle_t;
typedef struct { int chan_id; } dac_oneshot_config_t;
esp_err_t dac_oneshot_new_channel(const dac_oneshot_config_t *config, dac_oneshot_handle_t *handle);
esp_err_t dac_oneshot_del_channel(dac_oneshot_handle_t handle);
esp_err_t dac_oneshot_output_voltage(dac_oneshot_handle_t handle, int value);