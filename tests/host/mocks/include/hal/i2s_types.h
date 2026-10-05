/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

typedef struct test_channel *i2s_chan_handle_t;
enum { I2S_NUM_0, I2S_ROLE_MASTER, I2S_GPIO_UNUSED, I2S_DATA_BIT_WIDTH_16BIT,
       I2S_SLOT_MODE_MONO, I2S_STD_SLOT_RIGHT, I2S_PDM_SLOT_RIGHT };
typedef struct { int dma_desc_num, dma_frame_num; bool auto_clear; } i2s_chan_config_t;
typedef struct { unsigned int sample_rate; } test_clk_config_t;
typedef struct { int slot_mask; } test_slot_config_t;
#define I2S_CHANNEL_DEFAULT_CONFIG(port, role) ((i2s_chan_config_t){0})
#define I2S_STD_CLK_DEFAULT_CONFIG(rate) ((test_clk_config_t){rate})
#define I2S_PDM_RX_CLK_DEFAULT_CONFIG(rate) ((test_clk_config_t){rate})
#define I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(width, mode) ((test_slot_config_t){0})
#define I2S_PDM_RX_SLOT_DEFAULT_CONFIG(width, mode) ((test_slot_config_t){0})

esp_err_t i2s_new_channel(const i2s_chan_config_t *config, i2s_chan_handle_t *tx, i2s_chan_handle_t *rx);
esp_err_t i2s_del_channel(i2s_chan_handle_t channel);
esp_err_t i2s_channel_enable(i2s_chan_handle_t channel);
esp_err_t i2s_channel_disable(i2s_chan_handle_t channel);
esp_err_t i2s_channel_write(i2s_chan_handle_t channel, const void *data, size_t size, size_t *written, uint32_t timeout_ms);
esp_err_t i2s_channel_read(i2s_chan_handle_t channel, void *data, size_t size, size_t *received, uint32_t timeout_ms);