/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
typedef struct test_rmt_channel *rmt_channel_handle_t;
typedef struct rmt_encoder_t rmt_encoder_t;
typedef rmt_encoder_t *rmt_encoder_handle_t;
#define GPIO_NUM_25 25
#define RMT_CLK_SRC_DEFAULT 0
typedef struct {
    int gpio_num, clk_src, resolution_hz, mem_block_symbols, trans_queue_depth;
} rmt_tx_channel_config_t;
typedef struct { int loop_count; } rmt_transmit_config_t;
esp_err_t rmt_new_tx_channel(const rmt_tx_channel_config_t *config, rmt_channel_handle_t *channel);
esp_err_t rmt_enable(rmt_channel_handle_t channel);
esp_err_t rmt_disable(rmt_channel_handle_t channel);
esp_err_t rmt_del_channel(rmt_channel_handle_t channel);
esp_err_t rmt_tx_wait_all_done(rmt_channel_handle_t channel, int timeout_ms);
esp_err_t rmt_transmit(rmt_channel_handle_t channel, rmt_encoder_handle_t encoder,
                       const void *buffer, size_t size, const rmt_transmit_config_t *config);