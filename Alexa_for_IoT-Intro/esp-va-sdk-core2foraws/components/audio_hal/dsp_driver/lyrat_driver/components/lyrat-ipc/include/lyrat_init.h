// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#pragma once

#include <stdlib.h>
#include <driver/i2s.h>

#define SAMPLE_RATE_DEFAULT 48000UL
#define CHANNELS_DEFAULT 2
#define I2S_NUMBER_DEFAULT I2S_NUM_0

#define LYRAT_CONFIG_DEFAULT()              \
    {                                       \
        .sample_rate = SAMPLE_RATE_DEFAULT, \
        .channels = CHANNELS_DEFAULT,       \
        .i2s_number = I2S_NUMBER_DEFAULT,   \
        .set_i2s_clk = true,                \
    }

typedef struct lyrat_config {
    int sample_rate;    /* Typically 48K / 16K etc. */
    int channels;       /* 1 or 2 */
    int i2s_number;     /* 0 or 1 */
    bool set_i2s_clk;
} lyrat_config_t;

void lyrat_stop_capture();
void lyrat_start_capture();
int lyrat_stream_audio(uint8_t *buffer, int size, int wait);
void lyrat_mic_mute();
void lyrat_mic_unmute();

/**
 * @brief Configure lyrat board.
 *
 * Actual memory allocations/initializations will be done with `lyrat_init`
 */
void lyrat_configure(lyrat_config_t *cfg);

void lyrat_init();
