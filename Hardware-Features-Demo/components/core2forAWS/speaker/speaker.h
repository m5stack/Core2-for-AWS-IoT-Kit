/**
 * @file speaker.h
 * @brief Functions for the on-board 1W-0928 speaker using the NS4168 power amplifier.
 */

#pragma once
#include "stdint.h"

/**
 * @brief Initializes the speaker over I2S. The I2S bus is initialized
 * for output with a sample rate of 44100Hz, with a 16-bit depth, and 
 * right channel audio. Other configurations requires using the 
 * ESP-IDF I2S driver directly.
 * 
 * @note You must enable the speaker after initializing it with @ref Core2ForAWS_Speaker_Enable().
 * The speaker cannot be initialized at the same time
 * as the microphone since they both share a common pin (GPIO0).
 * Attempting to enable or use both at the same time will 
 * cause the device to hard fault.
 * 
 */
/* @[declare_speaker_init] */
void Speaker_Init();
/* @[declare_speaker_init] */

/**
 * @brief Plays buffer through the speaker.
 * 
 * @note The speaker cannot be used at the same time
 * as the microphone since they both share a common pin (GPIO0).
 * Attempting to enable or use both at the same time will 
 * cause the device to hard fault.
 * 
 * **Example:**
 * 
 * Play a sound buffer. The audio clip is too short to be 
 * recognized and is just to serve as an example.
 * @code{c}
 *  Microphone_Deinit(); // If the microphone was initialized, be sure to deinit it first.
 *  Speaker_Init();
 *  Core2ForAWS_Speaker_Enable(1);
 *  const uint8_t sound[16] = [0x01,0x00,0xff,0xff,0x01,0x00,0xff,0xff,0x01,0x00,0xff,0xff,0xff,0xff,0xff,0xff];
 *  Speaker_WriteBuff(&sound, 16, portMAX_DELAY);
 *  Core2ForAWS_Speaker_Enable(0);
 *  Speaker_Deinit();
 * @endcode
 * 
 * @param[out] buff The sound buffer to play.
 * @param[in] len Length of the buffer.
 * @param[in] timeout UNUSED.
 */
/* @[declare_speaker_writebuff] */
void Speaker_WriteBuff(uint8_t* buff, uint32_t len, uint32_t timeout);
/* @[declare_speaker_writebuff] */

/**
 * @brief De-initializes the speaker.
 */
/* @[declare_speaker_deinit] */
void Speaker_Deinit();
/* @[declare_speaker_deinit] */