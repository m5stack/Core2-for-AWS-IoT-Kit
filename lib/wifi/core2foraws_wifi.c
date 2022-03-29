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
 * @file core2foraws_wifi.c
 * @brief Core2 for AWS IoT EduKit Wi-Fi helper APIs
 */

#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>

#include "qrcode.h"

#include "core2foraws_wifi.h"
#include "core2foraws_display.h"
#include "core2foraws_common.h"

#define PROV_QR_VERSION "v1"
#define PROV_TRANSPORT  "ble"
#define PROV_POP        "EduKit1234"
#define QRCODE_BASE_URL "https://espressif.github.io/esp-jumpstart/qrcode.html"

static const char *_s_TAG = "CORE2AWS_WIFI";

static esp_netif_t *_s_wifi_netif = NULL;
static SemaphoreHandle_t _s_service_name_mutex;
static char service_name[ 19 ];

static void _s_on_prov_event_handler( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data );
static void _s_on_got_ip( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data );
static void _s_on_wifi_start( void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data );
static void _s_on_wifi_connect( void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data );
static void _s_on_wifi_disconnect( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data );
static void _s_device_service_name_set( void );
static void _s_wifi_prov_qr_print( void );

static void _s_on_prov_event_handler( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data )
{
    static uint8_t _s_retries;
    
    if ( event_id == WIFI_PROV_START )
        ESP_LOGI( _s_TAG, "\tProvisioning started" );
    else if ( event_id == WIFI_PROV_CRED_RECV )
    {
        wifi_sta_config_t *wifi_sta_cfg = ( wifi_sta_config_t * )event_data;
        ESP_LOGI( _s_TAG, "\tReceived Wi-Fi credentials"
                    "\n\tSSID     : %s",
                    ( const char * ) wifi_sta_cfg->ssid );
        ESP_LOGD( _s_TAG, "\tPassword : %s",
                    ( const char * ) wifi_sta_cfg->password );
    }
    else if ( event_id == WIFI_PROV_CRED_FAIL )
    {
        wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
        ESP_LOGE( _s_TAG, "\tProvisioning failed!\n\tReason : %s"
                    "\n\tWi-Fi will erase the credentials and restart provisioning in %d retries",
                    ( *reason == WIFI_PROV_STA_AUTH_ERROR ) ?
                    "Wi-Fi station authentication failed" : "Wi-Fi access-point not found",
                    WIFI_RETRIES_MAX_FAILS - _s_retries );
        _s_retries++;
        if ( _s_retries >= WIFI_RETRIES_MAX_FAILS )
        {
            ESP_LOGI( _s_TAG, "\tFailed to connect with provisioned AP, reseting provisioned credentials" );
            wifi_prov_mgr_reset_sm_state_on_failure();
            _s_retries = 0;
        }
    }
    else if ( event_id == WIFI_PROV_CRED_SUCCESS )
    {
        ESP_LOGI( _s_TAG, "\tProvisioning successful");
        _s_retries = 0;
    }
    else if ( event_id == WIFI_PROV_END )
        wifi_prov_mgr_deinit();
}

static void _s_on_got_ip( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data )
{
    if ( event_id == IP_EVENT_STA_GOT_IP )
    {
        ip_event_got_ip_t *event = ( ip_event_got_ip_t * )event_data;

        ESP_LOGI( _s_TAG, "\tGot IPv4 address: " IPSTR, IP2STR( &event->ip_info.ip ) );

        xEventGroupClearBits( wifi_event_group, WIFI_DISCONNECTED_BIT );
        xEventGroupClearBits( wifi_event_group, WIFI_CONNECTING_BIT );
        xEventGroupSetBits( wifi_event_group, WIFI_CONNECTED_BIT );
    }
}

static void _s_on_wifi_start( void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data )
{
    ESP_LOGI( _s_TAG, "\tStarting Wi-Fi... %d", event_id );
    xEventGroupSetBits( wifi_event_group, WIFI_DISCONNECTED_BIT );
    esp_wifi_connect();
}

static void _s_on_wifi_connect( void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data )
{
    ESP_LOGI( _s_TAG, "\tConnected" );
}

static void _s_on_wifi_disconnect( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data )
{
    ESP_LOGI( _s_TAG, "\tDisconnected, attempting to reconnect..." );
    xEventGroupClearBits( wifi_event_group, WIFI_CONNECTED_BIT );
    xEventGroupSetBits( wifi_event_group, WIFI_DISCONNECTED_BIT );

    esp_err_t err = esp_wifi_connect();
    if ( err == ESP_ERR_WIFI_NOT_STARTED )
    {
        return;
    }
    ESP_ERROR_CHECK( err );

    xEventGroupSetBits( wifi_event_group, WIFI_CONNECTING_BIT );
}

static void _s_device_service_name_set( void )
{
    uint8_t eth_mac[ 6 ];
    const char *ssid_prefix = "CORE2FORAWS_";
    esp_wifi_get_mac( WIFI_IF_STA, eth_mac );
    
    if (  xSemaphoreTake( _s_service_name_mutex, 40 ) == pdTRUE )
    {
        snprintf( service_name, sizeof( service_name ), "%s%02X%02X%02X",
                ssid_prefix, eth_mac[ 3 ], eth_mac[ 4 ], eth_mac[ 5 ] );
        xSemaphoreGive( _s_service_name_mutex );
    }
    else
        ESP_LOGE( _s_TAG, "Failed to set service name." );
}

static void _s_wifi_prov_qr_print( void )
{
    char provisioning_payload[ WIFI_PROV_STR_LEN ] = { 0 };

    if ( core2foraws_wifi_prov_str_get( provisioning_payload ) == ESP_OK )
    {
        ESP_LOGI( _s_TAG, "\tScan this QR code from the provisioning application for Wi-Fi provisioning." );
        esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
        esp_qrcode_generate( &cfg, provisioning_payload );
        ESP_LOGI( _s_TAG, "\tIf QR code is not visible, copy paste the below URL in a browser.\n%s?data=%s", QRCODE_BASE_URL, provisioning_payload);
    }
}

esp_err_t core2foraws_wifi_prov_ble_init( void )
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if ( err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND )
    {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ESP_ERROR_CHECK( nvs_flash_init() );
    }

    ESP_LOGI( _s_TAG, "\tInitializing" );

    /* Initialize TCP/IP */
    ESP_ERROR_CHECK( esp_netif_init() );

    /* Initialize the event loop */
    ESP_ERROR_CHECK( esp_event_loop_create_default() );
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK( esp_event_handler_register( IP_EVENT, ESP_EVENT_ANY_ID, &_s_on_got_ip, NULL ) );
    ESP_ERROR_CHECK( esp_event_handler_register( WIFI_EVENT, WIFI_EVENT_STA_START, &_s_on_wifi_start, _s_wifi_netif ) );
    ESP_ERROR_CHECK( esp_event_handler_register( WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &_s_on_wifi_connect, _s_wifi_netif ) );
    ESP_ERROR_CHECK( esp_event_handler_register( WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &_s_on_wifi_disconnect, NULL ) );
    ESP_ERROR_CHECK( esp_event_handler_register( WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &_s_on_prov_event_handler, NULL ) );

    /* Initialize Wi-Fi including netif with default config */
    _s_wifi_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init( &cfg ) );

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = 
    {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
    };

    /* Initialize provisioning manager with the
     * configuration parameters set above */
    ESP_ERROR_CHECK( wifi_prov_mgr_init( config ) );

    bool wifi_is_provisioned = false;
    /* Let's find out if the device is provisioned */
    ESP_ERROR_CHECK( wifi_prov_mgr_is_provisioned( &wifi_is_provisioned ) );

    _s_service_name_mutex = xSemaphoreCreateMutex();

    _s_device_service_name_set();
    ESP_LOGI( _s_TAG, "\tService Name: %s", service_name );

    /* If device is not yet provisioned start provisioning service */
    if ( !wifi_is_provisioned )
    {
        ESP_LOGI( _s_TAG, "\tStarting Wi-Fi provisioning over BLE" );

        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

        const char *service_key = NULL;

        /* This step is only useful when scheme is wifi_prov_scheme_ble. This will
         * set a custom 128 bit UUID which will be included in the BLE advertisement
         * and will correspond to the primary GATT service that provides provisioning
         * endpoints as GATT characteristics. Each GATT characteristic will be
         * formed using the primary service UUID as base, with different auto assigned
         * 12th and 13th bytes (assume counting starts from 0th byte). The client side
         * applications must identify the endpoints by reading the User Characteristic
         * Description descriptor (0x2901) for each characteristic, which contains the
         * endpoint name of the characteristic */
        uint8_t custom_service_uuid[] = 
        {
            /* LSB <---------------------------------------
             * ---------------------------------------> MSB */
            0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
            0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
        };
        err = wifi_prov_scheme_ble_set_service_uuid( custom_service_uuid );
        
        err = xSemaphoreTake( _s_service_name_mutex, portMAX_DELAY );
        if (  err == pdTRUE )
        {
            err = wifi_prov_mgr_start_provisioning( security, PROV_POP, service_name, service_key );
            xSemaphoreGive( _s_service_name_mutex );
        }
        else
            return err;

        /* Print QR code for provisioning */
        _s_wifi_prov_qr_print();
    }
    else
    {
        ESP_LOGI( _s_TAG, "\tAlready provisioned, starting Wi-Fi STA");

        wifi_prov_mgr_deinit();
        ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
        err = esp_wifi_start();
    }

    return err;
}

esp_err_t core2foraws_wifi_deinit( void )
{
    esp_err_t err = ESP_OK;
    ESP_ERROR_CHECK( esp_event_handler_unregister( IP_EVENT, ESP_EVENT_ANY_ID, &_s_on_got_ip ) );
    ESP_ERROR_CHECK( esp_event_handler_unregister( WIFI_EVENT, WIFI_EVENT_STA_START, &_s_on_wifi_start ) );
    ESP_ERROR_CHECK( esp_event_handler_unregister( WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &_s_on_wifi_connect ) );
    ESP_ERROR_CHECK( esp_event_handler_unregister( WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &_s_on_wifi_disconnect ) );
    ESP_ERROR_CHECK( esp_event_handler_unregister( WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &_s_on_prov_event_handler ) );

    err = esp_wifi_stop();
    if ( err == ESP_ERR_WIFI_NOT_INIT )
    {
        return err;
    }
    
    err = esp_wifi_deinit();
    esp_wifi_clear_default_wifi_driver_and_handlers( _s_wifi_netif );
    esp_netif_destroy( _s_wifi_netif );
    _s_wifi_netif = NULL;

    return err;
}

esp_err_t core2foraws_wifi_disconnect( void )
{
    return esp_wifi_disconnect();
}

esp_err_t core2foraws_wifi_connect( void )
{
    return esp_wifi_connect();
}

esp_err_t core2foraws_wifi_reset( void )
{
    return esp_wifi_restore();
}

esp_err_t core2foraws_wifi_prov_str_get( char *wifi_prov_str )
{
    int err = -1;
    if ( xSemaphoreTake( _s_service_name_mutex, 40 ) == pdTRUE )
    {        
        err = snprintf( wifi_prov_str, WIFI_PROV_STR_LEN, "{\"ver\":\"%s\",\"name\":\"%s\"" \
                    ",\"pop\":\"%s\",\"transport\":\"%s\"}",
                    PROV_QR_VERSION, service_name, PROV_POP, PROV_TRANSPORT );
        xSemaphoreGive( _s_service_name_mutex );

        if ( err > 0 )
        {
            ESP_LOGI( _s_TAG, "\tProvisioning string, length: %d. String: '%s'", err, wifi_prov_str );
            err = 0;
        }
        else
            ESP_LOGE( _s_TAG, "\tFailed to write provisioning string." );
    }

    return core2foraws_common_error( err );
}