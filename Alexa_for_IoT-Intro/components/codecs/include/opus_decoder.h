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

#ifndef _OPUS_DECODER_H_
#define _OPUS_DECODER_H_

#include <audio_codec.h>

/**
 * Defaults if not set explicitly.
 *
 * Please note that, re-defining these have no effect.
 * This are for understanding only.
 * Use proper `cfg` instead to `opus_codec_create` API.
 */
#define OPUS_CODEC_TASK_STACK_SIZE    (12 * 4096)
#define OPUS_CODEC_TASK_PRIORITY      3

/**
 * @brief   default opus decoder config
 *          e.g.: `opus_decoder_cfg_t cfg = DEFAULT_OPUS_DECODER_CONFIG();` will
 *                create default cfg to be passed to `opus_codec_create`
 */
#define DEFAULT_OPUS_DECODER_CONFIG() {                 \
    .task_stack         = OPUS_CODEC_TASK_STACK_SIZE,   \
    .task_prio          = OPUS_CODEC_TASK_PRIORITY,     \
    .out_sample_rate    = 48000,                        \
    .is_raw             = false,                        \
    .channels           = 2,                            \
    .in_sample_rate     = 48000,                        \
    .frame_size         = 160                           \
}

/**
 * @brief      Opus Decoder configuration
 */
typedef struct {
    int                     task_stack;         /*!< Task stack size */
    int                     task_prio;          /*!< Task priority (based on freeRTOS priority) */
    int                     out_sample_rate;    /*!< Desired output sample rate: 48000/24000/16000/12000/8000} */
    bool                    is_raw;             /*!< If set to raw, decoder will decode it as raw stream */
    int                     channels;           /*!< If raw stream, provide this info to decoder */
    int                     in_sample_rate;     /*!< If raw stream, provide this info to decoder */
    int                     frame_size;         /*!< If raw stream, provide this info to decoder */
} opus_codec_cfg_t;

typedef struct opus_codec {
    audio_codec_t base; /* Base codec pointer. Use audio_codec APIs on this. */

    /* Private members */
    uint8_t *in_buf; /* Used in raw opus case. */
    int in_buf_size; /* Used in raw opus case. */
    short *out_buf;
    int offset; //offset in ms
    bool is_raw;
    int outbuf_size;

    void *oy; ///ogg_sync_state oy;
    void *og; ///ogg_page       og;
    void *op; ///ogg_packet     op;
    void *os; ///ogg_stream_state os;

    int frame_cnt;
    int pcm_cnt;
    int opus_page_pos;
    int total_bytes_read;
    int eos;
    long long packet_count;
    int frame_size;
    int stream_init;

    long long page_granule;
    int has_opus_stream;
    int has_tags_packet;
    void *st; //OpusMSDecoder *st;
    int opus_serialno;
    int streams;
    int channels;
    int out_sample_rate;

    void *header;//OpusHeader header;

    int in_sample_rate;
    int current_sample_rate;
    int current_channels;
} opus_codec_t;

/**
 * @brief   create opus decoder with given config.
 *          This also allocate major chunks of memory needed for decoder.
 *          Some dynamic memory could still be allocated run-time.
 */
opus_codec_t *opus_codec_create(opus_codec_cfg_t *cfg);

/**
 * @brief   destroy opus decoder.
 *          This also deallocates chunks of memory allocated with `opus_codec_create`.
 */
esp_err_t opus_codec_destroy(opus_codec_t *codec);

#endif
