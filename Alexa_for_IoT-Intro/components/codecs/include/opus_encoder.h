// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _OPUS_ENCODER_H_
#define _OPUS_ENCODER_H_

#include <audio_codec.h>

#define DISABLE_RESAMPLE

#ifndef DISABLE_RESAMPLE
#include "resample.h"
#endif

/**
 * Defaults if not set explicitly.
 *
 * Please note that, re-defining these have no effect.
 * This are for understanding only.
 * Use proper `cfg` instead to `opus_enc_create` API.
 */
#define OPUS_CODEC_TASK_STACK_SIZE    (8 * 4096)
#define OPUS_CODEC_TASK_PRIORITY      3

#ifndef DISABLE_RESAMPLE
/* ----------------IGNORE----------------- */
typedef struct {
    void *user_data;
    int channel;
    int samplerate;
    int coding_rate;
    int dataNum;
    short *inbuf;
    int bufsize;
    int consumed;
    ogg_int64_t original_samples;
}resample_buf_t;

#endif

/**
 * @brief   default opus encoder config
 *          e.g.: `opus_encoder_cfg_t cfg = DEFAULT_OPUS_ENCODER_CONFIG();` will
 *                create default cfg to be passed to `opus_enc_create`
 */
#define DEFAULT_OPUS_ENCODER_CONFIG() {                 \
    .task_stack         = OPUS_CODEC_TASK_STACK_SIZE,   \
    .task_prio          = OPUS_CODEC_TASK_PRIORITY,     \
    .bit_rate           = 32000,                        \
    .sample_rate        = 16000,                        \
    .channels           = 1,                            \
    .frame_size         = 960,                          \
    .complexity         = 0,                            \
    .is_raw             = false,                        \
}

/**
 * @brief      Opus Encoder configuration
 */
typedef struct {
    int                     task_stack;         /*!< Task stack size */
    int                     task_prio;          /*!< Task priority (based on freeRTOS priority) */
    int                     bit_rate;           /*!< Desired output bitrate rate: e.g. 32000 for 32Kbps */
    int                     sample_rate;        /*!< Input sample rate: e.g. 16000 for 16K */
    int                     channels;           /*!< Number of channels in input PCM */
    int                     complexity;         /*!< Encoder complexity in [0, 10] */
    int                     frame_size;         /*!< Frame size. e.g. 960 is 20ms frame */
    bool                    is_raw;             /*!< If set to raw, ogg wrappers are not used */
} opus_encoder_cfg_t;

typedef struct opus_encoder {
    audio_codec_t base;   /* Base codec pointer. Use audio_codec APIs on this. */

    /* private members */
    short *out_buf;
    int offset; //offset in ms
    int outbuf_size;

    void *st; //OpusMSEncoder *st;

#ifdef OPUS_ENC_FLOAT
    float *pcm_buffer;
#else
    int16_t *pcm_buffer;
#endif

#ifndef DISABLE_RESAMPLE
    resample_buf_t resample_buf;
#endif
    unsigned char *packet;
////oe_enc_opt inopt;
    int64_t total_samples_per_channel;
    int channels;
    int opus_application_type;
    int32_t rate;  ///the real sample-rate
    int32_t coding_rate;  ///the codec input sample-rate
    int gain;
    int endianness;
    bool is_raw;

    char *comments;
    int comments_length;
    int skip;
    int samplesize;
    int extraout;

    void *og; ///ogg_page       og;
    void *op; ///ogg_packet     op;
    void *os; ///ogg_stream_state os;

    int64_t last_granulepos;
    int64_t enc_granulepos;
    int64_t original_samples;
    int32_t id;
    int framecnt;
    int last_segments;
    void *header; //OpusHeader header;

    int64_t nb_encoded;
    int64_t bytes_written;
    int64_t pages_out;
    int64_t total_bytes;
    int64_t total_samples;
    int32_t nbBytes;
    int32_t nb_samples;
    int32_t peak_bytes;
    int32_t min_bytes;

    int max_frame_bytes;
    int32_t bitrate;

    int32_t asked_frame_size;
    int32_t asked_bit_rate;
    int32_t asked_sample_rate;

    int32_t frame_size;
    int with_hard_cbr;
    int with_cvbr;
    int expect_loss;
    int complexity;

    int max_ogg_delay;
    int serialno;
    int lookahead;
} opus_encoder_t;

/**
 * @brief   create opus encoder with given config.
 *          This also allocate major chunks of memory needed for encoder.
 *          Some dynamic memory could still be allocated run-time.
 */
opus_encoder_t *opus_enc_create(opus_encoder_cfg_t *cfg);

/**
 * @brief   destroy opus encoder.
 *          This also deallocates chunks of memory allocated with `opus_enc_create`.
 */
esp_err_t opus_enc_destroy(opus_encoder_t *codec);

#endif
