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
 * @file core2foraws_wifi.h
 * @brief Core2 for AWS IoT EduKit Wi-Fi helper APIs
 */

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event.h"

/**
 * @brief The FreeRTOS event group bit for the device being in a Wi-Fi connected state.
 * 
 * The Wi-Fi event group @ref wifi_event_group has a bit for the connected, disconnected,
 * and connecting states of the Wi-Fi connectivity cycle.
 */
/* @[declare_core2foraws_wifi_connected_bit] */
#define CONNECTED_BIT BIT0
/* @[declare_core2foraws_wifi_connected_bit] */

/**
 * @brief The FreeRTOS event group bit for the device being in a Wi-Fi disconnected state.
 * 
 * The Wi-Fi event group @ref wifi_event_group has a bit for the connected, disconnected,
 * and connecting states of the Wi-Fi connectivity cycle.
 */
/* @[declare_core2foraws_wifi_disconnected_bit] */
#define DISCONNECTED_BIT BIT1
/* @[declare_core2foraws_wifi_disconnected_bit] */

/**
 * @brief The FreeRTOS event group bit for the device being in a Wi-Fi connecting state.
 * 
 * The Wi-Fi event group @ref wifi_event_group has a bit for the connected, disconnected,
 * and connecting states of the Wi-Fi connectivity cycle.
 */
/* @[declare_core2foraws_wifi_connecting_bit] */
#define CONNECTING_BIT BIT2
/* @[declare_core2foraws_wifi_connecting_bit] */

/**
 * @brief The maximum length allowed for the Wi-Fi SSID.
 */
/* @[declare_core2foraws_wifi_ssid_max_len] */
#define WIFI_SSID_MAX_LEN MAX_SSID_LEN
/* @[declare_core2foraws_wifi_ssid_max_len] */

/**
 * @brief The maximum length allowed for the Wi-Fi password.
 */
/* @[declare_core2foraws_wifi_pass_max_len_pass] */
#define WIFI_PASS_MAX_LEN MAX_PASSPHRASE_LEN
/* @[declare_core2foraws_wifi_pass_max_len_pass] */

/**
 * @brief The FreeRTOS event group bit for Wi-Fi event states.
 * 
 * The Wi-Fi event group has a bit for the connected (@ref CONNECTED_BIT), disconnected 
 * (@ref DISCONNECTED_BIT), and connecting (@ref CONNECTING_BIT) states of the Wi-Fi 
 * connectivity cycle.
 */
/* @[declare_core2foraws_wifi_event_group] */
EventGroupHandle_t wifi_event_group;
/* @[declare_core2foraws_wifi_event_group] */

/**
 * @brief Connects to Wi-Fi access point and stores credentials in NVS.
 * 
 * This function will connect to the Wi-Fi network using the configuration 
 * provided. If NULL is input to either of the parameters, it attempts to 
 * read from the non-volatile storage (NVS) for existing Wi-Fi credentials.
 * 
 * 
 * @param[in] wifi_ssid A pointer to the provided Wi-Fi SSID.
 * @param[in] wifi_password A pointer to the provided Wi-Fi password.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_wifi_connect] */
esp_err_t core2foraws_wifi_connect( const char *wifi_ssid, const char *wifi_password );
/* @[declare_core2foraws_wifi_connect] */

/**
 * @brief Disconnects from Wi-Fi access point.
 * This function will disconnect from the Wi-Fi network and free up 
 * resources that was allocated.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_wifi_disconnect] */
esp_err_t core2foraws_wifi_disconnect( void );
/* @[declare_core2foraws_wifi_disconnect] */