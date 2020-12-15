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

#ifndef _FLAC_ENCODER_H_
#define _FLAC_ENCODER_H_

#include <audio_codec.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FLAC_ENCODER_TASK_STACK_SIZE (6 * 1024)
#define FLAC_ENCODER_TASK_PRIORITY 5
#define MAX_CHANNELS 6

typedef struct flac_encoder {
    audio_codec_t base;
    void *encoder;
    unsigned long long tell_value;
    int frame_cnt;
    int sample_rate;
    int channels;
    int16_t *read_buf;
    int32_t *in_buf[MAX_CHANNELS];
} flac_encoder_t;

flac_encoder_t *flac_encoder_create();
esp_err_t flac_encoder_destroy(flac_encoder_t *codec);
void flac_encoder_set_stack_size(flac_encoder_t *codec, ssize_t stack_size);

#ifdef __cplusplus
}
#endif

#endif /* _FLAC_ENCODER_H_ */
