// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#ifndef _ALEXA_H_
#define _ALEXA_H_

#include <esp_err.h>
#include "voice_assistant.h"
#include "auth_delegate.h"

/** Enable Larger tones.
 *
 * This API will enable a tone which would be played if dialog is in progress and timer/alarm goes off.
 * Enabling this tone would increase size of binary by around 356 KB.
 */
void alexa_tone_enable_larger_tones();

/** Change Alexa Language.
 *
 * This API can be used to change the default locale i.e language and/or accent for Alexa. The supported locales can be found here: https://developer.amazon.com/docs/alexa-voice-service/settings.html
 */
void alexa_change_locale(const char *locale);

/** Application callback for a successful Alexa sign in */
typedef void (*alexa_app_signin_handler_t)(void *data);

/** Application callback for a successful Alexa sign out */
typedef void (*alexa_app_signout_handler_t)(void *data);

/** Initialize authentication module for Alexa
 *
 * @param: signin_handler Callback called upon successful sign in
 * @param: signout_handler Callback called upong successful sign out
 */
void alexa_auth_delegate_init(alexa_app_signin_handler_t signin_handler, alexa_app_signout_handler_t signout_handler);
void alexa_auth_delegate_signin(auth_delegate_config_t *cfg);
void alexa_auth_delegate_signout();

/**
 * @brief Check and init the device in BT standalone mode.
 *
 * The device will become BT-SINK speaker only. All Alexa functionality is disabled in this case.
 *
 * \return
 *      - ESP_OK on success
 *      - ESP_FAIL if device is not set to be run in Alexa+BT mode.
 */
esp_err_t alexa_bt_only_mode_init();

/**
 * @brief Restart the device in BT-only mode.
 *
 * The device will become BT-SINK speaker only. All Alexa functionality is disabled in this case.
 *
 * \return
 *      - ESP_OK on success
 *      - ESP_FAIL if device is already in BT-only mode.
 *
 * @note the api has no effect if a2dp_sink is not enabled using `alexa_bt_a2dp_sink_init`.
 */
int alexa_enable_bt_only_mode();

/**
 * @brief Restart the device in Alexa+BT mode.
 *
 * The device will have all the functionality of Alexa+BT.
 *
 * \return
 *      - ESP_OK on success
 *      - ESP_FAIL if the device is already in Alexa+BT mode
 *
 * @note the api has no effect if a2dp_sink is not enabled using `alexa_bt_a2dp_sink_init`.
 */
int alexa_disable_bt_only_mode();

/** Initialize Alexa
 *
 * This call must be made after the Wi-Fi connection has been established with the configured Access Point.
 *
 * \param[in] cfg The Alexa Configuration
 *
 * \return
 *    - 0 on Success
 *    - an error code otherwise
 */
int alexa_init();

int alexa_early_init();

/**
 * Enable BT A2DP sink and source
 *
 * Function enables BT A2DP sink and source functionality and registers it with Alexa. Function should be called before
 * the call to alexa_init()
 *
 * @note this function just sets a2dp_sink to be inited. Actual initialization is done by `alexa_init()`
 */
void alexa_bt_init();

#endif /*_ALEXA_H_ */
