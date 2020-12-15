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

#ifndef _AMR_DECODER_H_
#define _AMR_DECODER_H_

#include <audio_codec.h>

#define AMR_DECODER_TASK_STACK_SIZE    4096
#define AMR_DECODER_TASK_PRIORITY      3

typedef struct amr_decoder {
    audio_codec_t base;
    void *handle;
    bool is_raw;
    unsigned int inbuf_size;
    unsigned int outbuf_size;
    unsigned char *in_buf;
    short *out_buf;
    int framecnt;
    int pcmcnt;
    int amr_type;
    int _current_sampling_freq;
} amr_decoder_t;

#define DEFAULT_AMR_DECODER_CONFIG() {                  \
    .task_stack         = AMR_DECODER_TASK_STACK_SIZE,  \
    .task_prio          = AMR_DECODER_TASK_PRIORITY,    \
    .is_raw             = false,                        \
}

/* Audio type */
typedef enum {
    AUDIO_TYPE_UNKNOWN,
    AUDIO_TYPE_AMRNB,
    AUDIO_TYPE_AMRWB,
} amr_audio_type_t;

/**
 * @brief      AMR Decoder configuration
 */
typedef struct {
    int                     task_stack;         /*!< Task stack size */
    int                     task_prio;          /*!< Task priority (based on freeRTOS priority) */
    bool                    is_raw;             /*!< If set to raw, decoder will decode it as raw stream */
    amr_audio_type_t        amr_type;           /*!< If is raw, specify type: AUDIO_TYPE_AMRNB or AUDIO_TYPE_AMRWB */
} amr_decoder_cfg_t;

void amr_decoder_set_offset(amr_decoder_t *codec, int offset);
void amr_decoder_set_stack_size(amr_decoder_t *codec, ssize_t stack_size);
amr_decoder_t *amr_decoder_create(amr_decoder_cfg_t *cfg);
esp_err_t amr_decoder_destroy(amr_decoder_t *codec);

#endif /* _AMR_DECODER_H_ */
