/**
 * @file microphone.h
 * @brief Functions for the SPM1423 microphone.
 */

#pragma once

/**
 * @brief Microphone I2S port number. 
 */
/* @[declare_microphone_mici2s_number] */
#define MIC_I2S_NUMBER I2S_NUM_0
/* @[declare_microphone_mici2s_number] */

/**
 * @brief Initializes the microphone over I2S.
 * 
 * @note The microphone cannot be enabled at the same time
 * as the speaker since they both share a common pin (GPIO0).
 * Attempting to enable and use both at the same time will 
 * cause the device to hard fault.
 */
/* @[declare_microphone_init] */
void Microphone_Init();
/* @[declare_microphone_init] */

/**
 * @brief De-initializes the microphone over I2S. 
 */
/* @[declare_microphone_deinit] */
void Microphone_Deinit();
/* @[declare_microphone_deinit] */
