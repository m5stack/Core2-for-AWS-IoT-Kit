/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws.h
 * @brief Core2 for AWS IoT Kit general hardware driver APIs
 */

#ifndef _CORE2FORAWS_H_
#define _CORE2FORAWS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <esp_err.h>
#include <stdlib.h>

#ifdef CONFIG_SOFTWARE_BSP_SUPPORT
#include "core2foraws_common.h"
#include "core2foraws_board.h"
#include "core2foraws_power.h"

#ifdef CONFIG_SOFTWARE_AUDIO_SUPPORT
#include "core2foraws_audio.h"
#endif

#ifdef CONFIG_SOFTWARE_BUTTON_SUPPORT
#include "core2foraws_button.h"
#endif

#ifdef CONFIG_SOFTWARE_CRYPTO_SUPPORT
#include "core2foraws_crypto.h"
#endif

#ifdef CONFIG_SOFTWARE_DISPLAY_SUPPORT
#include "core2foraws_display.h"
#endif

#ifdef CONFIG_SOFTWARE_EXPANSION_PORTS_SUPPORT
#include "core2foraws_expports.h"
#endif

#ifdef CONFIG_SOFTWARE_MOTION_SUPPORT
#include "core2foraws_motion.h"
#endif

#ifdef CONFIG_SOFTWARE_RGB_LED_SUPPORT
#include "core2foraws_rgb_led.h"
#endif

#ifdef CONFIG_SOFTWARE_RTC_SUPPORT
#include "core2foraws_rtc.h"
#endif

#ifdef CONFIG_SOFTWARE_SDCARD_SUPPORT
#include "core2foraws_sd.h"
#endif

#ifdef CONFIG_SOFTWARE_WIFI_SUPPORT
#include "core2foraws_wifi.h"
#ifdef CONFIG_CORE2FORAWS_WIFI_PROVISIONING
#include "core2foraws_wifi_prov.h"
#endif
#endif
#endif

  /**
   * @brief Initializes enabled hardware features.
   *
  * If CONFIG_SOFTWARE_BSP_SUPPORT is disabled, common and hardware modules
  * are not compiled and this function returns ESP_OK without doing any work.
   * If the buttons on the touch screen are enabled, it initializes the
   * virtual button driver.
   * If the crypto chip is enabled, it initializes the the secure element.
   * If the display is enabled, it initializes the display and touch screen
   * driver.
   * If the motion sensor is enabled, it initializes the 6-axis IMU
   * w/ temperature driver.
   * If the real-time-clock is enabled, it initializes the RTC driver.
   * If the side RGB LED bars are enabled, it initializes the RGB LED
   * driver.
  * If Wi-Fi is enabled, it initializes the network stack without starting
  * the radio; call core2foraws_wifi_reconnect(), core2foraws_wifi_connect(),
  * or core2foraws_wifi_provisioning_start() to bring up the link.
  * The speaker, microphone (audio), SD card, and expansion-port sessions
  * need to be initialized separately as needed.
  *
  * Internal I2C or PMU failure returns its original error immediately, without
  * attempting downstream peripherals. Later module
  * failures are logged and aggregated so every remaining module is attempted;
  * the final return is ESP_FAIL if any of them failed. Applications that need
  * the original module error can call that module's init function directly.
  * Repeating this function after a successful call is safe.
   *
   * @return
  * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK    : Success
   *  - ESP_FAIL  : Failed to initialize one or more features
  *  - Other     : Original internal-I2C or PMU initialization error
   */
  /* @[declare_core2foraws_init] */
  esp_err_t core2foraws_init( void );
  /* @[declare_core2foraws_init] */

#ifdef __cplusplus
}
#endif
#endif
