/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include "hal/i2s_types.h"
typedef struct {
    test_clk_config_t clk_cfg;
    test_slot_config_t slot_cfg;
    struct {
        int mclk, bclk, ws, dout, din;
        struct { bool mclk_inv, bclk_inv, ws_inv; } invert_flags;
    } gpio_cfg;
} i2s_std_config_t;
esp_err_t i2s_channel_init_std_mode(i2s_chan_handle_t channel, const i2s_std_config_t *config);