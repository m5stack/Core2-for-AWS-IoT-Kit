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

#ifndef _AMRNB_ENCODER_H_
#define _AMRNB_ENCODER_H_

#include "audio_codec.h"

#define AMRNB_ENCODER_TASK_STACK_SIZE    4096
#define AMRNB_ENCODER_TASK_PRIORITY      3

typedef struct amrnb_encoder_t {
    audio_codec_t base;

    /*--- Private fields ---*/
    void *encoder;
    uint8_t *in_buf;
    uint8_t *out_buf;

    int pcmcnt;
    int framecnt;
    int total_bytes;
    int mode; /* encoding mode */
} amrnb_encoder_t;

/**
 * Set encoding mode to one of the below:
 * MR475 = 0 // 4.75 kbps
 * MR515 = 1 // 5.15 kbps
 * MR59  = 2 // 5.90 kbps
 * MR67  = 3 // 6.70 kbps
 * MR74  = 4 // 7.40 kbps
 * MR795 = 5 // 7.95 kbps
 * MR102 = 6 // 10.2 kbps
 * MR122 = 7 // 12.2 kbps
 * MRDTX = 8 // DTX
 */
void amrnb_encoder_set_mode(amrnb_encoder_t *codec, int mode);

void amrnb_encoder_set_stack_size(amrnb_encoder_t *codec, ssize_t stack_size);
amrnb_encoder_t *amrnb_encoder_create();
esp_err_t amrnb_encoder_destroy(amrnb_encoder_t *codec);

#endif /* _AMRNB_ENCODER_H_ */