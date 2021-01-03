// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#ifndef _MP3_DECODER_H_
#define _MP3_DECODER_H_

#include <audio_codec.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Defaults if not set explicitly.
 *
 * Please note that, re-defining these have no effect.
 * This are for understanding only.
 * Use `mp3_codec_set_stack_size` instead after `mp3_codec_create`.
 */
#define MP3_CODEC_TASK_STACK_SIZE    (8 * 1024)
#define MP3_CODEC_TASK_PRIORITY      3

/**
 * @brief   mp3 codec structure
 */
typedef struct mp3_codec {
    audio_codec_t base; /* Base codec pointer. Use audio_codec APIs on this. */

    /* Private members */
    void *pvmp3_decoder;
    uint8_t *decode_buf;

    uint8_t *in_buf;
    uint8_t *out_buf;
    uint8_t _run;
    int _skip_id3;
    int _current_sampling_freq;
    int _current_channels;
    int offset_in_ms; //offset in ms for seek
} mp3_codec_t;

/**
 * @brief   create mp3 decoder.
 *          This also allocate major chunks of memory needed for decoder.
 *          Some dynamic memory could still be allocated run-time.
 */
mp3_codec_t *mp3_codec_create();

/**
 * @brief   destroy mp3 decoder.
 *          This also deallocates chunks of memory allocated with `mp3_codec_create`.
 */
esp_err_t mp3_codec_destroy(mp3_codec_t *codec);

/**
 * @brief   set stack size of mp3 decoder.
 *
 * @note    must be set before `audio_codec_init`, ineffective otherwise
 */
void mp3_codec_set_stack_size(mp3_codec_t *codec, ssize_t stack_size);

#ifdef __cplusplus
}
#endif

#endif /* _MP3_DECODER_H_ */
