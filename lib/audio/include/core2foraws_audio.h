/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_audio.h
 * @brief Core2 for AWS IoT Kit audio hardware driver APIs
 */

#ifndef _CORE2FORAWS_AUDIO_H_
#define _CORE2FORAWS_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <hal/i2s_types.h>
#include <esp_err.h>

/**
 * @brief Port number for Inter-IC Sound (I2S) communications
 */
/* @[declare_core2foraws_audio_i2s_port_num] */
#define AUDIO_I2S_PORT_NUM I2S_NUM_0
/* @[declare_core2foraws_audio_i2s_port_num] */

#ifndef AUDIO_SAMPLING_FREQ
/**
 * @brief Audio recording and playback sampling frequency.
 */
/* @[declare_core2foraws_audio_sampling_freq] */
#define AUDIO_SAMPLING_FREQ 44100U
/* @[declare_core2foraws_audio_sampling_freq] */
#endif

#ifndef AUDIO_IO_TIMEOUT_MS
/** @brief Maximum time for an audio I/O operation or lifecycle lock. */
#define AUDIO_IO_TIMEOUT_MS 1000U
#endif

/**
 * @brief Enables or disables the device speaker driver.
 *
 * Provides power to the NS4168 speaker amplifier and initializes 
 * the I2S bus for sending right-only channel audio, with 16-bit 
 * depth, at the sample rate defined by @ref 
 * AUDIO_SAMPLING_FREQ.
 * 
 * @note The speaker cannot be enabled at the same time as the 
 * microphone since they both share a common pin (GPIO0). Attempting 
 * to enable and use both at the same time will return an error.
 * Call this once with `true` before writing audio, then call it again
 * with `false` when you are done.
 * Disabling holds the NS4168 CTRL line low for more than the datasheet's
 * 100 us shutdown-entry requirement before releasing the I2S channel.
 * If shutdown/disable/delete fails, owned resources are retained and the
 * original error is returned. Retry with false before enabling either mode
 * or writing audio. Failed initialization uses the same retryable cleanup.
 *
 * @param[in] state Desired state of the speaker. 1 to enable, 0 to 
 * disable.
 * 
 * @return Error code of changing speaker state.
 *  - ESP_OK    : Success
 *  - ESP_FAIL  : Failed to enable the speaker
 */
/* @[declare_core2foraws_audio_speaker_enable] */
esp_err_t core2foraws_audio_speaker_enable( bool state );

/** Enqueue silence exceeding the TX DMA ring (including both 16-bit slots).
 * After success all previously accepted samples have played, though trailing
 * silence may remain. Call before disabling the speaker, with no other writer.
 * Propagates write timeout/short-write errors; does not disable the channel.
 */
esp_err_t core2foraws_audio_speaker_drain(void);
/* @[declare_core2foraws_audio_speaker_enable] */

/**
 * @brief Enables or disables the device microphone driver.
 *
 * A failed disable/delete retains its channel for a later false call. Neither
 * mode can be enabled, and microphone reads are rejected, while cleanup is
 * pending. Failed initialization also retains any resource it cannot release.
 *
 * Initializes the I2S bus for receiving right-only channel audio, 
 * with 16-bit depth, at the sample rate defined by @ref 
 * AUDIO_SAMPLING_FREQ.
 * 
 * @note The microphone cannot be enabled at the same time as the 
 * speaker since they both share a common pin (GPIO0). Attempting 
 * to enable and use both at the same time will return an error.
 *
 * @param[in] state Desired state of the microphone. 1 to enable, 0 to 
 * disable.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL  : Failed to enable the mic
 */
/* @[declare_core2foraws_audio_mic_enable] */
esp_err_t core2foraws_audio_mic_enable( bool state );
/* @[declare_core2foraws_audio_mic_enable] */

/**
 * @brief Writes the provided buffer for speaker playback.
 * 
 * @note The speaker cannot be enabled at the same time as the 
 * microphone since they both share a common pin (GPIO0). Attempting 
 * to enable and use both at the same time will return an error.
 * 
 * **Example:**
 * 
 * Play a sound buffer. The audio clip is too short to be recognized 
 * and is just to serve as an example.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <esp_log.h>
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_SPEAKER_EXAMPLE";
 * 
 *  void app_main( void )
 *  {
 *      ESP_LOGI( TAG, "\tStarting..." );
 *      core2foraws_init();
 * 
 *      if ( core2foraws_audio_speaker_enable( true ) == ESP_OK )
 *      {
 *          const uint8_t sound[16] = {
 *              0x01, 0x00, 0xff, 0xff,
 *              0x01, 0x00, 0xff, 0xff,
 *              0x01, 0x00, 0xff, 0xff,
 *              0xff, 0xff, 0xff, 0xff
 *          };
 *
 *          core2foraws_audio_speaker_write( sound, sizeof( sound ) );
 *          core2foraws_audio_speaker_enable( false );
 *      }
 *  }
 * @endcode
 * 
 * @param[in] sound_buffer The sound buffer to play.
 * @param[in] to_write_length Length of the buffer to play.
 *
 * @note Lock acquisition and the I2S transfer each use @ref AUDIO_IO_TIMEOUT_MS.
 * The transfer timeout is in milliseconds, independently of the RTOS tick rate.
 * The lock prevents speaker disable from deleting the active I2S channel.
 * ESP_OK means the entire buffer was accepted; a short successful driver write
 * is reported as ESP_FAIL. Acceptance does not mean playback has finished.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL  : Failed to write
 */
/* @[declare_core2foraws_audio_speaker_write] */
esp_err_t core2foraws_audio_speaker_write( const uint8_t *sound_buffer, size_t to_write_length );
/* @[declare_core2foraws_audio_speaker_write] */

/**
 * @brief Reads audio from the microphone into the provided buffer.
 * 
 * @note The microphone cannot be enabled at the same time as the 
 * speaker since they both share a common pin (GPIO0). Attempting to 
 * enable and use both at the same time will return an error.
 * Call this once with `true` before reading audio, then call it again
 * with `false` when you are done.
 * 
 * **Example:**
 * 
 * Record a few second sound buffer to dynamically allocated 
 * external memory and play it back through the speaker.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <esp_log.h>
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_MICROPHONE_EXAMPLE";
 * 
 *  void app_main( void )
 *  {
 *      ESP_LOGI( TAG, "\tStarting..." );
 *      core2foraws_init();
 *      
 *      size_t read_length = 307200;
 *      esp_err_t err = core2foraws_audio_mic_enable( true );
 *      if ( err == ESP_OK )
 *      {
 *          int8_t *mic_buffer = ( int8_t * )heap_caps_malloc( read_length * sizeof( int8_t ), MALLOC_CAP_SPIRAM );
 *          size_t was_read_length = 0;
 *
 *          if ( mic_buffer != NULL )
 *          {
 *              err = core2foraws_audio_mic_read( mic_buffer, read_length, &was_read_length );
 *              if ( err == ESP_OK )
 *                  ESP_LOGI( TAG, "\tRead %d bytes from mic!", was_read_length );
 *          }
 *
 *          core2foraws_audio_mic_enable( false );
 *
 *          err = core2foraws_audio_speaker_enable( true );
 *          if ( err == ESP_OK && mic_buffer != NULL )
 *          {
 *              err = core2foraws_audio_speaker_write( ( const uint8_t * ) mic_buffer, was_read_length );
 *              if ( err == ESP_OK )
 *                  ESP_LOGI( TAG, "\tWrote %d bytes to the speaker!", was_read_length );
 *              core2foraws_audio_speaker_enable( false );
 *          }
 *          free( mic_buffer );
 *      }
 *  }
 * @endcode
 * 
 * @param[in] sound_buffer The sound buffer to record to.
 * @param[in] to_read_length Length of the buffer to read.
 * @param[out] was_read_length Length of audio read.
 *
 * @note Lock acquisition and the I2S transfer each use @ref AUDIO_IO_TIMEOUT_MS.
 * The transfer timeout is in milliseconds, independently of the RTOS tick rate.
 * The lock prevents microphone disable from deleting the active I2S channel.
 * A timed-out read may still return partial data through was_read_length.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL  : Failed to read
 */
/* @[declare_core2foraws_audio_mic_read] */
esp_err_t core2foraws_audio_mic_read( int8_t *sound_buffer, size_t to_read_length , size_t *was_read_length );
/* @[declare_core2foraws_audio_mic_read] */

#ifdef __cplusplus
}
#endif
#endif
