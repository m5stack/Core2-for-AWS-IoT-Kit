/*
 * Core2 for AWS IoT Kit BSP v2.0.0
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
 */

/**
 * @file core2foraws.h
 * @brief Core2 for AWS IoT Kit general hardware driver APIs
 */

#ifndef _CORE2FORAWS_H_
#define _CORE2FORAWS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <esp_err.h>

#ifdef CONFIG_SOFTWARE_BSP_SUPPORT
#include "core2foraws_common.h"
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

#ifdef CONFIG_SOFTWARE_EXPPORTS_SUPPORT
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
#endif
#endif

/**
 * @brief Initializes enabled hardware features.
 *
 * If no features are enabled, it does nothing.
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
 * The speaker, microphone (audio) and SD card drivers need to be 
 * initialized separately as needed.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros). 
 *  - ESP_OK    : Success
 *  - ESP_FAIL  : Failed to initialize one or more features
 */
/* @[declare_core2foraws_init] */
esp_err_t core2foraws_init( void );
/* @[declare_core2foraws_init] */

#ifdef __cplusplus
}
#endif
#endif
