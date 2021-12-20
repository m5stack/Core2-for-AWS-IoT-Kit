/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit
 * Cloud Connected Blinky v1.4.0
 * wifi.c
 * 
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Additions Copyright 2016 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <string.h>
#include <esp_event.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "lwip/err.h"
#include "lwip/sys.h"

#include "core2foraws_wifi.h"
#include "core2foraws_common.h"

#define NVS_WIFI_NAMESPACE "wifi_c"
#define NVS_KEY_NAME_SSID "s"
#define NVS_KEY_NAME_PASS "p"

static const char *_s_TAG = "CORE2AWS_WIFI";

static esp_netif_t *_s_wifi_netif = NULL;
static esp_ip_addr_t _s_ip_addr;
static esp_ip6_addr_t _s_ipv6_addr;

static void _s_on_got_ip( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data );
static void _s_on_got_ipv6( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data );
static void _s_on_wifi_disconnect( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data );
static void _s_on_wifi_connect( void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data );
static esp_err_t _s_nvs_wifi_credentials_get( char *ssid, char *password );
static esp_err_t _s_nvs_wifi_credentials_set( const char *ssid, const char *password );

static void _s_on_got_ip( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data )
{
    ip_event_got_ip_t *event = ( ip_event_got_ip_t * )event_data;
    ESP_LOGI( _s_TAG, "Got IPv4 address: " IPSTR, IP2STR( &event->ip_info.ip ) );
    memcpy( &_s_ip_addr, &event->ip_info.ip, sizeof( _s_ip_addr ) );

    xEventGroupClearBits( wifi_event_group, DISCONNECTED_BIT );
    xEventGroupClearBits( wifi_event_group, CONNECTING_BIT );
    xEventGroupSetBits( wifi_event_group, CONNECTED_BIT );
}

static void _s_on_got_ipv6( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data )
{
    ip_event_got_ip6_t *event = ( ip_event_got_ip6_t * )event_data;

    ESP_LOGI( _s_TAG, "Got IPv6 address: " IPV6STR, IPV62STR( event->ip6_info.ip ) );
    memcpy( &_s_ipv6_addr, &event->ip6_info.ip, sizeof( _s_ipv6_addr ) );

    xEventGroupClearBits( wifi_event_group, DISCONNECTED_BIT );
    xEventGroupClearBits( wifi_event_group, CONNECTING_BIT );
    xEventGroupSetBits( wifi_event_group, CONNECTED_BIT );
}

static void _s_on_wifi_disconnect( void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data )
{
    ESP_LOGI( _s_TAG, "Disconnected, attempting to reconnect..." );
    xEventGroupClearBits( wifi_event_group, CONNECTED_BIT );
    xEventGroupSetBits( wifi_event_group, DISCONNECTED_BIT );

    esp_err_t err = esp_wifi_connect();
    if ( err == ESP_ERR_WIFI_NOT_STARTED )
    {
        return;
    }
    ESP_ERROR_CHECK( err );

    xEventGroupSetBits( wifi_event_group, CONNECTING_BIT );
}

static void _s_on_wifi_connect( void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data )
{
    ESP_LOGI( _s_TAG, "\tConnected" );
    esp_netif_create_ip6_linklocal( esp_netif );
}

static esp_err_t _s_nvs_wifi_credentials_get( char *ssid, char *password )
{
    size_t ssid_str_len = WIFI_SSID_MAX_LEN + 1;
    size_t password_str_len = WIFI_PASS_MAX_LEN + 1;

    esp_err_t err;
    nvs_handle_t nvs_wifi_config_handle;
    
    ESP_ERROR_CHECK( nvs_open( NVS_WIFI_NAMESPACE, NVS_READWRITE, &nvs_wifi_config_handle ) );

    err = nvs_get_str( nvs_wifi_config_handle, NVS_KEY_NAME_SSID, ssid, &ssid_str_len );
    if( err == ESP_OK )
        err = nvs_get_str( nvs_wifi_config_handle, NVS_KEY_NAME_PASS, password, &password_str_len );
    nvs_close( nvs_wifi_config_handle );

    return err;
}

static esp_err_t _s_nvs_wifi_credentials_set( const char *ssid, const char *password )
{
    esp_err_t err;
    nvs_handle_t nvs_wifi_config_handle;
    
    ESP_ERROR_CHECK( nvs_open( NVS_WIFI_NAMESPACE, NVS_READWRITE, &nvs_wifi_config_handle ) );

    err = nvs_set_str( nvs_wifi_config_handle, NVS_KEY_NAME_SSID, ssid );
    if( err == ESP_OK )
        err = nvs_set_str( nvs_wifi_config_handle, NVS_KEY_NAME_PASS, password );
    nvs_commit( nvs_wifi_config_handle );
    nvs_close( nvs_wifi_config_handle );
    
    return err;
}

esp_err_t core2foraws_wifi_disconnect( void )
{
    esp_err_t err = ESP_OK;
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &_s_on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &_s_on_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &_s_on_got_ipv6));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &_s_on_wifi_connect));

    err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return err;
    }
    
    err |= esp_wifi_deinit();
    err |= esp_wifi_clear_default_wifi_driver_and_handlers( _s_wifi_netif );
    esp_netif_destroy( _s_wifi_netif );
    _s_wifi_netif = NULL;

    return err;
}

esp_err_t core2foraws_wifi_connect( const char *wifi_ssid, const char *wifi_password )
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if ( err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND )
    {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    wifi_event_group = xEventGroupCreate();
    xEventGroupSetBits(wifi_event_group, DISCONNECTED_BIT);

    ESP_ERROR_CHECK( esp_netif_init() );
    ESP_ERROR_CHECK( esp_event_loop_create_default() );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init( &cfg ) );

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    _s_wifi_netif = esp_netif_create_wifi( WIFI_IF_STA, &esp_netif_config );
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK( esp_event_handler_register( WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &_s_on_wifi_disconnect, NULL ) );
    ESP_ERROR_CHECK( esp_event_handler_register( IP_EVENT, IP_EVENT_STA_GOT_IP, &_s_on_got_ip, NULL ) );
    ESP_ERROR_CHECK( esp_event_handler_register( WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &_s_on_wifi_connect, _s_wifi_netif ) );
    ESP_ERROR_CHECK( esp_event_handler_register( IP_EVENT, IP_EVENT_GOT_IP6, &_s_on_got_ipv6, NULL ) );

    ESP_ERROR_CHECK( esp_wifi_set_storage( WIFI_STORAGE_RAM ) );
    
    char *temp_ssid = ( char * )heap_caps_malloc( WIFI_SSID_MAX_LEN + 1, MALLOC_CAP_SPIRAM );
    char *temp_password = ( char * )heap_caps_malloc( WIFI_PASS_MAX_LEN + 1, MALLOC_CAP_SPIRAM );

    wifi_config_t user_wifi_config = 
        {
            .sta = 
            {
                .scan_method = WIFI_ALL_CHANNEL_SCAN,
                .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
                .threshold.authmode = WIFI_AUTH_WPA2_PSK
            }
        };
    
    if( ( wifi_ssid != NULL ) && ( wifi_password != NULL ) )
    {
        memcpy( ( void * )temp_ssid, ( void * )wifi_ssid, ( int )( ( strlen( wifi_ssid ) + 1 ) * sizeof( char ) ) );
        memcpy( ( void * )temp_password, ( void * )wifi_password,  ( int )( ( strlen( wifi_password ) + 1 ) * sizeof( char ) ) );
        _s_nvs_wifi_credentials_set( temp_ssid, temp_password );
    }
    else
    {
        err = _s_nvs_wifi_credentials_get( temp_ssid, temp_password );
        if(  err == ESP_OK )
        {
            strcpy( ( char * )user_wifi_config.sta.ssid, temp_ssid );
            strcpy( ( char * )user_wifi_config.sta.password, temp_password );
            ESP_LOGI( _s_TAG, "\tConnecting to Wi-Fi SSID: %s", user_wifi_config.sta.ssid );
            ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
            ESP_ERROR_CHECK( esp_wifi_set_config( WIFI_IF_STA, &user_wifi_config ) );
            ESP_ERROR_CHECK( esp_wifi_start() );

            err = esp_wifi_connect();

            if( err == ESP_OK )
            {
                xEventGroupSetBits(wifi_event_group, CONNECTING_BIT);
            }
        }
        else
            core2foraws_wifi_disconnect();
    }
    
    free( temp_ssid );
    free( temp_password );

    return err;
}