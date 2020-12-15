// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#ifndef _AAC_DECODER_H_
#define _AAC_DECODER_H_

#include <audio_codec.h>
#include <esp_audio_pm.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Defaults if not set explicitly.
 *
 * Please note that, re-defining these have no effect.
 * This are for understanding only.
 * Use `aac_codec_set_stack_size` instead after `aac_codec_create`.
 */
#define AAC_CODEC_TASK_STACK_SIZE    4096
#define AAC_CODEC_TASK_PRIORITY      3

/**
 * @brief   aac codec structure
 */
typedef struct aac_codec {
    audio_codec_t base; /* Base codec pointer. Use audio_codec APIs on this. */

    /* Private members */
    void *pvmp4_decoder;
    uint8_t *decode_buf;
    uint8_t *in_buf;
    short *out_buf;
    void *aacdata; ///aac_callbacks_t
    void *audio_info; //audio_info_t *audio_info;
    void *user_data; ///aac_buffer or m4a_buffer
    uint8_t _run;
    int _audio_type;
    int offset; //offset in ms
    esp_audio_pm_lock_handle_t pm_lock;
    bool is_ts_stream;
    int pcmcnt;
    int framecnt;
    int _samplerate_idx;
    int _channels;
    int numSamples;
    int sampleId;
    int offsetIdx;
    int streampos;
    int rawdatausedbyte;
    uint32_t mFixedHeader;
    int transportFmt;
    int frame_length;
    int bits;
    int _current_sampling_freq;
    int _current_channels;
} aac_codec_t;

/**
 * @brief   set stack size of aac decoder.
 *
 * @note    must be set before `audio_codec_init`, ineffective otherwise
 */
void aac_codec_set_stack_size(aac_codec_t *codec, ssize_t stack_size);

/**
 * @brief   create aac decoder.
 *          This also allocate major chunks of memory needed for decoder.
 *          Some dynamic memory could still be allocated run-time.
 */
aac_codec_t *aac_codec_create();

/**
 * @brief   destroy aac decoder.
 *          This also deallocates chunks of memory allocated with `aac_codec_create`.
 */
esp_err_t aac_codec_destroy(aac_codec_t *codec);

#ifdef __cplusplus
}
#endif

#endif /* _MP3_DECODER_H_ */
