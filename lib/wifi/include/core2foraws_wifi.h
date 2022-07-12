/*
 * Core2 for AWS IoT EduKit BSP v2.0.0
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

/* This library referenced the Espressif Systems (Shanghai) PTE LTD's, Public Domain
 * provisioning example:
 * https://github.com/espressif/esp-idf/tree/release/v4.3/examples/provisioning/wifi_prov_mgr
*/

/**
 * @file core2foraws_wifi.h
 * @brief Core2 for AWS IoT EduKit Wi-Fi helper APIs
 */

#ifndef _CORE2FORAWS_WIFI_H_
#define _CORE2FORAWS_WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>

/**
 * @brief The FreeRTOS event group bit for the device being in a Wi-Fi connected state.
 * 
 * The Wi-Fi event group @ref wifi_event_group has a bit for the connected, disconnected,
 * and connecting states of the Wi-Fi connectivity cycle.
 */
/* @[declare_core2foraws_wifi_connected_bit] */
#define WIFI_CONNECTED_BIT BIT0
/* @[declare_core2foraws_wifi_connected_bit] */

/**
 * @brief The FreeRTOS event group bit for the device being in a Wi-Fi disconnected state.
 * 
 * The Wi-Fi event group @ref wifi_event_group has a bit for the connected, disconnected,
 * and connecting states of the Wi-Fi connectivity cycle.
 */
/* @[declare_core2foraws_wifi_disconnected_bit] */
#define WIFI_DISCONNECTED_BIT BIT1
/* @[declare_core2foraws_wifi_disconnected_bit] */

/**
 * @brief The FreeRTOS event group bit for the device being in a Wi-Fi connecting state.
 * 
 * The Wi-Fi event group @ref wifi_event_group has a bit for the connected, disconnected,
 * and connecting states of the Wi-Fi connectivity cycle.
 */
/* @[declare_core2foraws_wifi_connecting_bit] */
#define WIFI_CONNECTING_BIT BIT2
/* @[declare_core2foraws_wifi_connecting_bit] */

/**
 * @brief The maximum length allowed for the Wi-Fi SSID.
 */
/* @[declare_core2foraws_wifi_prov_str_len] */
#define WIFI_PROV_STR_LEN 78U
/* @[declare_core2foraws_wifi_prov_str_len] */

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
 * @brief The maximum number of failures when attempting to connect using provided 
 * credentials before calling @ref core2foraws_wifi_reset().
 */
/* @[declare_core2foraws_wifi_retries_max_fails] */
#ifndef WIFI_RETRIES_MAX_FAILS
#define WIFI_RETRIES_MAX_FAILS 5U
#endif
/* @[declare_core2foraws_wifi_retries_max_fails] */

/**
 * @brief The FreeRTOS event group bit for Wi-Fi event states.
 * 
 * The Wi-Fi event group has a bit for the connected (@ref WIFI_CONNECTED_BIT), disconnected 
 * (@ref WIFI_DISCONNECTED_BIT), and connecting (@ref WIFI_CONNECTING_BIT) states of the Wi-Fi 
 * connectivity cycle.
 */
/* @[declare_core2foraws_wifi_event_group] */
EventGroupHandle_t wifi_event_group;
/* @[declare_core2foraws_wifi_event_group] */

/**
 * @brief Starts Wi-Fi provisioning using BLE.
 * 
 * @note This function is automatically called by @ref core2foraws_init if the feature
 * is enabled.
 * 
 * This function will start up BLE provisioning process to collect Wi-Fi credentials 
 * using the [iOS](https://apps.apple.com/in/app/esp-ble-provisioning/id1473590141) 
 * or [Android](https://play.google.com/store/apps/details?id=com.espressif.provble) app. 
 * After entering the credentials in the app, the device will attempt to connect to
 * the Wi-Fi network.
 * 
 * After a successful connection, the device will store the credentials into the device 
 * non-volatile storage (NVS). On reboot, the device will read the stored credentials
 * and use them to connect to the wireless network.
 * 
 * After @ref WIFI_RETRIES_MAX_FAILS failures to connect, the app will restart the 
 * provisioning cycle over BLE.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_WIFI_NOT_INIT : WiFi is not initialized
 *  - ESP_ERR_NO_MEM        : Out of memory
 *  - ESP_ERR_WIFI_CONN     : Internal error, station or soft-AP control block wrong
 *  - ESP_FAIL              : Failed to connect to Wi-Fi or start up the provisioning manager.
 */
/* @[declare_core2foraws_wifi_prov_ble_init] */
esp_err_t core2foraws_wifi_prov_ble_init( void );
/* @[declare_core2foraws_wifi_prov_ble_init] */

/**
 * @brief Deinitializes the Wi-Fi driver.
 * This function will disconnect from the Wi-Fi network and free up 
 * resources that were allocated for the Wi-Fi stack.
 * 
 * **Example:**
 * 
 * Initialize the Core2 for AWS IoT EduKit (including the enabled Wi-Fi), wait
 * 5 seconds, and then disconnect from the Wi-Fi network.
 * @code{c}
 *  #include <stdint.h>
 *  #include <esp_log.h>
 *  #include <freertos/FreeRTOS.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_WIFI_DEMO";
 * 
 *  void app_main( void )
 *  {   
 *      core2foraws_init();
 * 
 *      vTaskDelay( pdMS_TO_TICKS( 5000 ) );
 * 
 *      esp_err_t err = core2foraws_wifi_deinit();
 *      ESP_LOGI( TAG, "\tWi-Fi reset returned %d", err );
 *  }
 * @endcode
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 
 *  - ESP_OK                : Success
 *  - ESP_ERR_WIFI_NOT_INIT : WiFi is not initialized
 */
/* @[declare_core2foraws_wifi_deinit] */
esp_err_t core2foraws_wifi_deinit( void );
/* @[declare_core2foraws_wifi_deinit] */

/**
 * @brief Disconnect the device from the Wi-Fi AP.
 * 
 * A return of `ESP_OK` does not mean that the device is disconnected, it 
 * means that the configuration is suitable for a disconnection attempt. Use 
 * FreeRTOS event groups and the @ref WIFI_DISCONNECTED_BIT to monitor if the 
 * device is freed from the Wi-Fi network.
 * 
 * **Example:**
 * 
 * Initialize the Core2 for AWS IoT EduKit (including the enabled Wi-Fi), 
 * which is already provisioned and connected to an available network. Wait 5 
 * seconds and disconnect from the Wi-Fi network.
 * @code{c}
 *  #include <stdint.h>
 *  #include <esp_log.h>
 *  #include <freertos/FreeRTOS.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_WIFI_DEMO";
 * 
 *  void app_main( void )
 *  {   
 *      core2foraws_init();
 * 
 *      vTaskDelay( pdMS_TO_TICKS( 5000 ) );
 * 
 *      esp_err_t err core2foraws_wifi_disconnect();
 * 
 *      ESP_LOGI( TAG, "\tWi-Fi disconnect returned %d", err );
 *  }
 * @endcode
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 
 *  - ESP_OK                    : Success
 *  - ESP_ERR_WIFI_NOT_INIT     : WiFi is not initialized
 *  - ESP_ERR_WIFI_NOT_STARTED  : WiFi is not started by esp_wifi_start
 *  - ESP_FAIL                  : other WiFi internal errors
 */
/* @[declare_core2foraws_wifi_disconnect] */
esp_err_t core2foraws_wifi_disconnect( void );
/* @[declare_core2foraws_wifi_disconnect] */

/**
 * @brief Connect to the configured Wi-Fi AP.
 * This function connects to the Wi-Fi network already configured during the
 * provisioning process. If the device hasn't successfully connected to an
 * access point already, this function will return an error.
 * 
 * A return of `ESP_OK` does not mean that the device is connected, it means
 * that the configuration is suitable for a connection attempt. Use FreeRTOS
 * event groups and the @ref WIFI_CONNECTED_BIT to monitor if the device is
 * connected to the Wi-Fi network.
 * 
 * **Example:**
 * 
 * Initialize the Core2 for AWS IoT EduKit (including the enabled Wi-Fi), 
 * which is already provisioned to an available network. Wait 5 seconds, 
 * disconnect from the Wi-Fi network, then after another 5 seconds, reconnect.
 * @code{c}
 *  #include <stdint.h>
 *  #include <esp_log.h>
 *  #include <freertos/FreeRTOS.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_WIFI_DEMO";
 * 
 *  void app_main( void )
 *  {   
 *      core2foraws_init();
 * 
 *      vTaskDelay( pdMS_TO_TICKS( 5000 ) );
 * 
 *      core2foraws_wifi_disconnect();
 * 
 *      vTaskDelay( pdMS_TO_TICKS( 5000 ) );
 * 
 *      esp_err_t err = core2foraws_wifi_connect();
 * 
 *      ESP_LOGI( TAG, "\tWi-Fi connect returned %d", err );
 *  }
 * @endcode
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros). 
 *  - ESP_OK                    : Success
 *  - ESP_ERR_WIFI_NOT_INIT     : WiFi is not initialized
 *  - ESP_ERR_WIFI_NOT_STARTED  : WiFi is not started by esp_wifi_start
 *  - ESP_ERR_WIFI_CONN         : WiFi internal error, station or soft-AP control block wrong
 *  - ESP_ERR_WIFI_SSID         : SSID of AP which station connects is invalid
 */
/* @[declare_core2foraws_wifi_connect] */
esp_err_t core2foraws_wifi_connect( void );
/* @[declare_core2foraws_wifi_connect] */

/**
 * @brief Reset internal Wi-Fi state machine and clear provisioned credentials.
 * 
 * This function erases the stored credentials and restarts the Wi-Fi provisioning 
 * process.
 * 
 * **Example:**
 * 
 * Initialize the Core2 for AWS IoT EduKit (including the enabled Wi-Fi), wait
 * 5 seconds, and then reset the Wi-Fi to enter new credentials using the mobile
 * app.
 * @code{c}
 *  #include <stdint.h>
 *  #include <esp_log.h>
 *  #include <freertos/FreeRTOS.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_WIFI_DEMO";
 * 
 *  void app_main( void )
 *  {   
 *      core2foraws_init();
 * 
 *      vTaskDelay( pdMS_TO_TICKS( 5000 ) );
 * 
 *      esp_err_t err = core2foraws_wifi_reset();
 *      ESP_LOGI( TAG, "\tWi-Fi reset returned %d", err );
 *  }
 * @endcode
 *
 * @return 
 *  - [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_WIFI_NOT_INIT : Wi-Fi is not initialized

 */
/* @[declare_core2foraws_wifi_reset] */
esp_err_t core2foraws_wifi_reset( void );
/* @[declare_core2foraws_wifi_reset] */

/**
 * @brief Gets the stringified JSON used by the companion mobile provisioning app
 * for setting up Wi-Fi.
 * 
 * @note This function can be used with the included LVGL QR Code library to 
 * generate QR codes that can be displayed on the screen.
 * 
 * **Example:**
 * 
 * Initialize the Core2 for AWS IoT EduKit (including the enabled Wi-Fi), display
 * the provisioning payload as a QR code on the screen to scan with companion 
 * mobile app. 
 * Removes the QR code once the device connects to Wi-Fi.
 * @code{c}
 *  #include <stdint.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_WIFI_DEMO";
 * 
 *  void app_main( void )
 *  {   
 *      core2foraws_init();
 * 
 *      xSemaphoreTake( core2foraws_common_spi_semaphore, pdMS_TO_TICKS( 80 ) )
 *      
 *      lv_coord_t qr_square_px = 200;
 *      lv_color_t amazon_squid_ink = lv_color_hex( 0x232F3E );
 *      lv_obj_t *display_wifi_qr = lv_qrcode_create( lv_scr_act(), qr_square_px, amazon_squid_ink, LV_COLOR_WHITE );
 *      char wifi_provisioning_str[ WIFI_PROV_STR_LEN ] = { 0 };
 * 
 *      if ( core2foraws_wifi_prov_str_get( wifi_provisioning_str ) == ESP_OK )
 *          lv_qrcode_update( display_wifi_qr, wifi_provisioning_str, strlen( wifi_provisioning_str ) );
 *      
 *      xSemaphoreGive( core2foraws_common_spi_semaphore );
 * 
 *      xEventGroupWaitBits(
 *          wifi_event_group,   // The event group being tested.
 *          WIFI_CONNECTED_BIT, // The bit(s) within the event group to wait for.
 *          pdFALSE,            // WIFI_CONNECTED_BIT should be cleared before returning.
 *          pdTRUE,             // Don't wait for all bits (there's only one anyway).
 *          portMAX_DELAY );    // Wait indefinitely.
 *
 *		xSemaphoreTake( core2foraws_common_spi_semaphore, pdMS_TO_TICKS( 80 ) );
 *	    lv_obj_del( display_wifi_qr );
 *		xSemaphoreGive( core2foraws_common_spi_semaphore );
 *      
 *  }
 * @endcode
 *
 * @param[out] wifi_prov_str The pointer to the wifi provisioning string.
 * @return 
 *  - [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK      : Success
 *  - ESP_FAIL    : Failed to retrieve the Wi-Fi provisioning string
 */
/* @[declare_core2foraws_wifi_prov_str_get] */
esp_err_t core2foraws_wifi_prov_str_get( char *wifi_prov_str );
/* @[declare_core2foraws_wifi_prov_str_get] */

#ifdef __cplusplus
}
#endif
#endif
