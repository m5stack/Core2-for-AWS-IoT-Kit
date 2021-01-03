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

#ifndef _WAV_DECODER_H_
#define _WAV_DECODER_H_

#include <audio_codec.h>

/**
 * Defaults if not set explicitly.
 *
 * Please note that, re-defining these have no effect.
 * This are for understanding only.
 * Use `wav_decoder_set_stack_size` instead after `wav_decoder_create`.
 */
#define WAV_DECODER_TASK_STACK_SIZE     4096
#define WAV_DECODER_TASK_PRIORITY       3

typedef struct wav_decoder {
    audio_codec_t base;  /* Base codec pointer. Use audio_codec APIs on this. */

    /* private members */
    void *handle;
    unsigned int inbuf_size;
    unsigned char *in_buf;
    int pcmcnt;
    int _current_sampling_freq;
    int _current_channels;
    bool header_parsed;
} wav_decoder_t;

/**
 * @brief   set stack size of wav decoder.
 *
 * @note    must be set before `audio_codec_init`, ineffective otherwise
 */
void wav_decoder_set_stack_size(wav_decoder_t *codec, ssize_t stack_size);

/**
 * @brief   create wav decoder.
 *          This also allocate major chunks of memory needed for decoder.
 *          Some dynamic memory could still be allocated run-time.
 */
wav_decoder_t *wav_decoder_create();

/**
 * @brief   destroy wav decoder.
 *          This also deallocates chunks of memory allocated with `wav_decoder_create`.
 */
esp_err_t wav_decoder_destroy(wav_decoder_t *codec);

#endif /* _WAV_DECODER_H_ */
