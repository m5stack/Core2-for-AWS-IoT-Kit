// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#pragma once

#include <stdlib.h>
#include <i2s_stream.h>

#define SAMPLE_RATE_ADC 48000UL
#define SAMPLE_RATE_DAC 48000UL
#define M5STACKCORE2_ADC_IN_CH_CNT 1
#define M5STACKCORE2_DAC_OUT_CH_CNT 1
#define I2S_PORT_DAC  I2S_NUM_0
#define M5STACKCORE2_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define SAMPLE_LENGTH_MS 20

extern xSemaphoreHandle mic_state; /* To protect I2S writes from I2S uninstall */

typedef struct m5stackcore2_config {
    int sample_rate;    /* Typically 48K / 16K etc. */
    int channels;       /* 1 or 2 or 3 or 4 */
    int i2s_number;     /* 0 or 1 */
    bool set_i2s_clk;
} m5stackcore2_config_t;

/*
 * API to pause i2s writes to speaker. To be used when Mic mode of I2S is enabled
 */
void m5stackcore2_audio_stream_pause();
/*
 * API to resume i2s writes to speaker. To be used when Speaker mode of I2S is enabled
 */
void m5stackcore2_audio_stream_resume();
/*
 * API to indicate dsp to stop sending microphone data
 */
void m5stackcore2_stop_capture();
/*
 * API to indicate dsp to start sending microphone data
 * @Note: This API is same as custom_dsp_tap_to_talk();
 */
void m5stackcore2_start_capture();
/*
 * API for dsp to send audio data, should return number of bytes read from the DSP audio buffer
 * @Note: Audio data format is 16KHz, 16BIT, MonO Channel. Little Endian
 */
int m5stackcore2_stream_audio(uint8_t *buffer, int size, int wait);
/*
 * Put DSP in Mute state(Microphones to be shut off)
 */
void m5stackcore2_mic_mute();
/*
 * Disable Mic Mute state, Microphones to be turned on
 */
void m5stackcore2_mic_unmute();
/*
 * Initilize acoustic echo cancellation on dsp
 */
void m5stackcore2_aec_init();
/*
 * API to put dsp in low power mode
 * @NOTE: Implementation of this api is optional and doesnot affect the Alexa Functionality
 */
void m5stackcore2_enter_low_power();
/*
 * API to get 'ALEXA' phrase length, returns phrase length in bytes
 */
size_t m5stackcore2_get_ww_len();
/*
 * API to reset dsp
 */
void m5stackcore2_reset();

/*
 * API to indicate dsp to start sending microphone data
 */
void custom_dsp_tap_to_talk();

/**
 * @brief Configure m5stackcore2 board.
 *
 * Actual memory allocations/initializations will be done with `m5stackcore2_init`
 */
void m5stackcore2_configure(m5stackcore2_config_t *cfg);

void m5stackcore2_init();
