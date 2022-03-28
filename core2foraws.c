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
 * @file core2foraws.c
 * @brief Core2 for AWS IoT EduKit general hardware driver APIs
 */

#include <esp_log.h>

#include "core2foraws.h"

static const char *TAG = "CORE2FORAWS";

esp_err_t core2foraws_init( void )
{
    esp_err_t err = ESP_FAIL;
    esp_err_t ret = ESP_OK;
    ESP_LOGI( TAG, "\tInitializing" );

#ifdef CONFIG_SOFTWARE_BSP_SUPPORT
    err = core2foraws_power_init();
    if ( err != ESP_OK )
        ESP_LOGE( TAG, "\tError initializing power. Error 0x%x", err );
    ret |= err;
#else
    ESP_LOGE( TAG, "\tHardware features are disabled. Nothing to initialize. \
            Must be enabled in the application KConfig menu");
#endif

#ifdef CONFIG_SOFTWARE_WIFI_SUPPORT
    err = core2foraws_wifi_prov_ble_init();
    if ( err != ESP_OK )
        ESP_LOGE( TAG, "\tError initializing Wi-Fi provisioning over BLE. Error 0x%x", err );
    ret |= err;
#endif

#ifdef CONFIG_SOFTWARE_DISPLAY_SUPPORT
    err = core2foraws_display_init();
    if ( err != ESP_OK )
        ESP_LOGE( TAG, "\tError initializing display. Error 0x%x", err );
    ret |= err;
#endif

#ifdef CONFIG_SOFTWARE_MOTION_SUPPORT
    err = core2foraws_motion_init();
    if ( err != ESP_OK )
        ESP_LOGE( TAG, "\tError initializing motion sensor. Error 0x%x", err );
    ret |= err;
#endif

#ifdef CONFIG_SOFTWARE_RTC_SUPPORT
    err = core2foraws_rtc_init();
    if ( err != ESP_OK )
        ESP_LOGE( TAG, "\tError initializing real time clock. Error 0x%x", err );
    ret |= err;
#endif

#ifdef CONFIG_SOFTWARE_CRYPTO_SUPPORT
    err = core2foraws_crypto_init();
    if ( err != ESP_OK )
        ESP_LOGE( TAG, "\tError initializing secure element (crypto chip). Error 0x%x", err );
    ret |= err;
#endif

#ifdef CONFIG_SOFTWARE_RGB_LED_SUPPORT
    err = core2foraws_rgb_led_init();
    if ( err != ESP_OK )
        ESP_LOGE( TAG, "\tError initializing rgb leds. Error 0x%x", err );
    ret |= err;
#endif

#ifdef CONFIG_SOFTWARE_BUTTON_SUPPORT
    err = core2foraws_button_init();
    if ( err != ESP_OK )
        ESP_LOGE( TAG, "\tError initializing button. Error 0x%x", err );
    ret |= err;
#endif

    return core2foraws_common_error( ret );
}