/*
 * Core2 for AWS IoT EduKit BSP v2.0.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://aws.amazon.com/iot/edukit
 *
 */

/**
 * @file core2foraws_audio.h
 * @brief Core2 for AWS IoT EduKit audio hardware driver APIs
 */

#ifndef _CORE2FORAWS_AUDIO_H_
#define _CORE2FORAWS_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <driver/i2s.h>
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
 *
 * @param[in] state Desired state of the speaker. 1 to enable, 0 to 
 * disable.
 * 
 * @return Error code of changing speaker state. 0 or `ESP_OK` if 
 * successful.
 */
/* @[declare_core2foraws_audio_speaker_enable] */
esp_err_t core2foraws_audio_speaker_enable( bool state );
/* @[declare_core2foraws_audio_speaker_enable] */

/**
 * @brief Enables or disables the device microphone driver.
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
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
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
 *      core2foraws_audio_speaker_enable( true );
 *      const unsigned char sound[16] = [0x01,0x00,0xff,0xff,0x01,0x00,0xff,0xff,0x01,0x00,0xff,0xff,0xff,0xff,0xff,0xff];
 *      core2foraws_audio_speaker_write( ( const uint8_t * )sound, 16 );
 *      core2foraws_audio_speaker_enable( false );
 *  }
 * @endcode
 * 
 * @param[in] sound_buffer The sound buffer to play.
 * @param[in] to_write_length Length of the buffer to play.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_audio_speaker_write] */
esp_err_t core2foraws_audio_speaker_write( const uint8_t *sound_buffer, size_t to_write_length );
/* @[declare_core2foraws_audio_speaker_write] */

/**
 * @brief Writes audio to the provided buffer from the microphone.
 * 
 * @note The microphone cannot be enabled at the same time as the 
 * speaker since they both share a common pin (GPIO0). Attempting to 
 * enable and use both at the same time will return an error.
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
 *          size_t was_read_length;
 *          esp_err_t err = core2foraws_audio_mic_read( mic_buffer, read_length, &was_read_length );
 *          if ( err == ESP_OK )
 *              ESP_LOGI( TAG, "\tRead %d bytes from mic!", was_read_length );
 *          core2foraws_audio_mic_enable( false );
 *
 *          err = core2foraws_audio_speaker_enable( true );
 *          if (err == ESP_OK )
 *          {
 *              err = core2foraws_audio_speaker_write( ( uint8_t * )mic_buffer, was_read_length );
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
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_audio_mic_read] */
esp_err_t core2foraws_audio_mic_read( int8_t *sound_buffer, size_t to_read_length , size_t *was_read_length );
/* @[declare_core2foraws_audio_mic_read] */

#ifdef __cplusplus
}
#endif
#endif
