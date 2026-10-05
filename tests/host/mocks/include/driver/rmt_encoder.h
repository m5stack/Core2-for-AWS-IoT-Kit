/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include "rmt_tx.h"
#ifndef __containerof
#define __containerof(ptr, type, member) ((type *)(void *)((char *)(ptr) - offsetof(type, member)))
#endif
typedef enum {
    RMT_ENCODING_RESET = 0,
    RMT_ENCODING_COMPLETE = 1 << 0,
    RMT_ENCODING_MEM_FULL = 1 << 1,
} rmt_encode_state_t;
typedef union {
    struct {
        uint16_t duration0 : 15;
        uint16_t level0 : 1;
        uint16_t duration1 : 15;
        uint16_t level1 : 1;
    };
    uint32_t val;
} rmt_symbol_word_t;
struct rmt_encoder_t {
    size_t (*encode)(rmt_encoder_t *encoder, rmt_channel_handle_t tx_channel,
                     const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state);
    esp_err_t (*reset)(rmt_encoder_t *encoder);
    esp_err_t (*del)(rmt_encoder_t *encoder);
};
typedef struct { int unused; } rmt_copy_encoder_config_t;
typedef struct {
    struct { unsigned int duration0, level0, duration1, level1; } bit0, bit1;
    struct { bool msb_first; } flags;
} rmt_bytes_encoder_config_t;
esp_err_t rmt_new_bytes_encoder(const rmt_bytes_encoder_config_t *config, rmt_encoder_handle_t *encoder);
esp_err_t rmt_del_encoder(rmt_encoder_handle_t encoder);
esp_err_t rmt_new_copy_encoder(const rmt_copy_encoder_config_t *config, rmt_encoder_handle_t *encoder);
esp_err_t rmt_encoder_reset(rmt_encoder_handle_t encoder);