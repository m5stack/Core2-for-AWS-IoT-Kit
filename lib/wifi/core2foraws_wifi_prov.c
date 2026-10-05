/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/* This library referenced the Espressif Systems (Shanghai) PTE LTD's, Public Domain
 * provisioning example:
 * https://github.com/espressif/esp-idf/tree/release/v4.3/examples/provisioning/wifi_prov_mgr
 */

/**
 * @file core2foraws_wifi_prov.c
 * @brief Core2 for AWS IoT Kit BLE Wi-Fi provisioning.
 */

#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
#include <sdkconfig.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <network_provisioning/manager.h>
#include <network_provisioning/scheme_ble.h>
#include <protocomm_ble.h>
#ifdef CONFIG_CORE2FORAWS_WIFI_PROV_CONSOLE_QR
#include "qrcode.h"
#endif
#ifdef CONFIG_CORE2FORAWS_WIFI_RELEASE_BLE_WHEN_PROVISIONED
#include <esp_bt.h>
#endif

#include "core2foraws_wifi_prov.h"
#include "core2foraws_wifi_priv.h"

#define PROV_QR_VERSION       "v1"
#define PROV_TRANSPORT        "ble"
#define PROV_SERVICE_PREFIX   "CORE2FORAWS_"
#define PROV_QRCODE_BASE_URL  "https://espressif.github.io/esp-jumpstart/qrcode.html"

static const char *_TAG = "CORE2FORAWS_WIFI_PROV";

static atomic_bool _session_open;
static atomic_bool _phone_connected;
static atomic_bool _applying;
static atomic_bool _ble_released;
/* Touched only from the event task */
static unsigned int _cred_failures;
/* Written by session open and read by payload_get, both under the lifecycle lock */
static char _service_name[ sizeof( PROV_SERVICE_PREFIX ) + 6 ];
static char _pop[ 9 ];

/* The BLE scheme keeps this pointer; LSB first */
static uint8_t _service_uuid[] =
{
    0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
    0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
};

core2foraws_wifi_prov_state_t core2foraws_wifi_provisioning_state_get( void )
{
    if( !atomic_load( &_session_open ) ) return CORE2FORAWS_WIFI_PROV_OFF;
    if( atomic_load( &_applying ) ) return CORE2FORAWS_WIFI_PROV_APPLYING;
    if( atomic_load( &_phone_connected ) ) return CORE2FORAWS_WIFI_PROV_PHONE_CONNECTED;
    return CORE2FORAWS_WIFI_PROV_WAITING;
}

core2foraws_wifi_prov_state_t core2foraws_wifi_priv_prov_state( void )
{
    return core2foraws_wifi_provisioning_state_get();
}

/* Safe from any context; whichever caller clears _session_open deinitializes the manager. */
static void _session_close( void )
{
    if( !atomic_exchange( &_session_open, false ) ) return;

    esp_err_t err = network_prov_mgr_deinit();
    if( err != ESP_OK ) ESP_LOGW( _TAG, "\tProvisioning manager deinit failed: 0x%x", err );
    atomic_store( &_applying, false );
    atomic_store( &_phone_connected, false );
    _cred_failures = 0;
    xEventGroupClearBits( wifi_event_group, WIFI_PROVISIONING_BIT );
#ifdef CONFIG_CORE2FORAWS_WIFI_RELEASE_BLE_WHEN_PROVISIONED
    /* The FREE_BTDM scheme handler has released the controller memory */
    atomic_store( &_ble_released, true );
#endif
    ESP_LOGI( _TAG, "\tProvisioning session closed" );
}

static void _on_prov_event( void *arg, esp_event_base_t base, int32_t id, void *data )
{
    ( void )arg; ( void )base;
    switch( id )
    {
        case NETWORK_PROV_START:
            ESP_LOGI( _TAG, "\tProvisioning started as %s", _service_name );
            break;

        case NETWORK_PROV_WIFI_CRED_RECV:
        {
            const wifi_sta_config_t *config = data;
            ESP_LOGI( _TAG, "\tReceived credentials for SSID %.*s", MAX_SSID_LEN, ( const char * )config->ssid );
            atomic_store( &_applying, true );
            core2foraws_wifi_priv_link_yield();
            core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_CONNECTING );
            break;
        }

        case NETWORK_PROV_WIFI_CRED_FAIL:
        {
            const network_prov_wifi_sta_fail_reason_t *reason = data;
            esp_err_t err = *reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR ? ESP_ERR_WIFI_PASSWORD
                                                                        : ESP_ERR_NOT_FOUND;
            ESP_LOGW( _TAG, "\tPhone credentials failed: %s", esp_err_to_name( err ) );
            atomic_store( &_applying, false );
            core2foraws_wifi_priv_last_error_set( err );
            core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_DISCONNECTED );
            if( ++_cred_failures >= CONFIG_CORE2FORAWS_WIFI_PROV_MAX_FAILS )
            {
                ESP_LOGI( _TAG, "\tResetting provisioning after %u failures", _cred_failures );
                ( void )network_prov_mgr_reset_wifi_sm_state_on_failure();
                _cred_failures = 0;
            }
            break;
        }

        case NETWORK_PROV_WIFI_CRED_SUCCESS:
            ESP_LOGI( _TAG, "\tPhone credentials connected" );
            atomic_store( &_applying, false );
            _cred_failures = 0;
            break;

        case NETWORK_PROV_END:
            _session_close();
            break;

        default:
            break;
    }
}

static void _on_ble_event( void *arg, esp_event_base_t base, int32_t id, void *data )
{
    ( void )arg; ( void )base; ( void )data;
    if( id == PROTOCOMM_TRANSPORT_BLE_CONNECTED )
    {
        ESP_LOGI( _TAG, "\tPhone connected" );
        atomic_store( &_phone_connected, true );
    }
    else if( id == PROTOCOMM_TRANSPORT_BLE_DISCONNECTED )
    {
        ESP_LOGI( _TAG, "\tPhone disconnected" );
        atomic_store( &_phone_connected, false );
    }
}

esp_err_t core2foraws_wifi_priv_prov_init( void )
{
    esp_err_t err = esp_event_handler_register( NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, _on_prov_event, NULL );
    if( err != ESP_OK ) return err;
    err = esp_event_handler_register( PROTOCOMM_TRANSPORT_BLE_EVENT, ESP_EVENT_ANY_ID, _on_ble_event, NULL );
    if( err != ESP_OK ) esp_event_handler_unregister( NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, _on_prov_event );
    return err;
}

void core2foraws_wifi_priv_prov_deinit( void )
{
    _session_close();
    esp_event_handler_unregister( PROTOCOMM_TRANSPORT_BLE_EVENT, ESP_EVENT_ANY_ID, _on_ble_event );
    esp_event_handler_unregister( NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, _on_prov_event );
}

void core2foraws_wifi_priv_prov_stop( void )
{
    _session_close();
}

void core2foraws_wifi_priv_prov_on_connected( void )
{
#ifdef CONFIG_CORE2FORAWS_WIFI_RELEASE_BLE_WHEN_PROVISIONED
    if( atomic_load( &_session_open ) || atomic_load( &_ble_released ) ) return;

    esp_err_t err = esp_bt_controller_mem_release( ESP_BT_MODE_BTDM );
    if( err == ESP_OK )
    {
        atomic_store( &_ble_released, true );
        ESP_LOGI( _TAG, "\tReleased BLE controller memory; provisioning is unavailable until reboot" );
    }
    else if( err != ESP_ERR_INVALID_STATE )
    {
        ESP_LOGW( _TAG, "\tFailed to release BLE controller memory: 0x%x", err );
    }
#endif
}

static esp_err_t _identity_set( void )
{
    uint8_t mac[ 6 ];
    esp_err_t err = esp_wifi_get_mac( WIFI_IF_STA, mac );
    if( err != ESP_OK ) return err;

    snprintf( _service_name, sizeof( _service_name ), PROV_SERVICE_PREFIX "%02X%02X%02X",
              mac[ 3 ], mac[ 4 ], mac[ 5 ] );
    snprintf( _pop, sizeof( _pop ), "%02x%02x%02x%02x", mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );
    return ESP_OK;
}

static void _payload_write( char *payload, size_t len )
{
    snprintf( payload, len, "{\"ver\":\"%s\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"%s\"}",
              PROV_QR_VERSION, _service_name, _pop, PROV_TRANSPORT );
}

static void _console_qr_print( void )
{
#ifdef CONFIG_CORE2FORAWS_WIFI_PROV_CONSOLE_QR
    char payload[ CORE2FORAWS_WIFI_PROV_PAYLOAD_LEN ];
    _payload_write( payload, sizeof( payload ) );
    ESP_LOGI( _TAG, "\tScan this QR code from the provisioning app." );
    esp_qrcode_config_t config = ESP_QRCODE_CONFIG_DEFAULT();
    esp_err_t err = esp_qrcode_generate( &config, payload );
    if( err != ESP_OK ) ESP_LOGW( _TAG, "\tConsole QR code generation failed: 0x%x", err );
    ESP_LOGI( _TAG, "\tIf the QR code is not visible, open this URL in a browser:\n%s?data=%s",
              PROV_QRCODE_BASE_URL, payload );
#endif
}

static esp_err_t _session_open_locked( void )
{
    network_prov_mgr_config_t config =
    {
        .scheme = network_prov_scheme_ble,
#ifdef CONFIG_CORE2FORAWS_WIFI_RELEASE_BLE_WHEN_PROVISIONED
        .scheme_event_handler = NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
#else
        /* Keeps the BLE controller memory so provisioning can run again this boot */
        .scheme_event_handler = NETWORK_PROV_EVENT_HANDLER_NONE,
#endif
    };

    esp_err_t err = network_prov_mgr_init( config );
    if( err != ESP_OK ) return err;

    err = _identity_set();
    if( err == ESP_OK ) err = network_prov_scheme_ble_set_service_uuid( _service_uuid );
    if( err == ESP_OK )
    {
        atomic_store( &_phone_connected, false );
        atomic_store( &_applying, false );
        atomic_store( &_session_open, true );
        xEventGroupSetBits( wifi_event_group, WIFI_PROVISIONING_BIT );
        err = network_prov_mgr_start_provisioning( NETWORK_PROV_SECURITY_1, _pop, _service_name, NULL );
    }

    if( err != ESP_OK )
    {
        atomic_store( &_session_open, false );
        xEventGroupClearBits( wifi_event_group, WIFI_PROVISIONING_BIT );
        ( void )network_prov_mgr_deinit();
        return err;
    }

    _console_qr_print();
    return ESP_OK;
}

esp_err_t core2foraws_wifi_provisioning_start( void )
{
    esp_err_t err = core2foraws_wifi_priv_lock();
    if( err != ESP_OK ) return err;

    if( !core2foraws_wifi_priv_initialized() || core2foraws_wifi_priv_attempt_active() )
    {
        err = ESP_ERR_INVALID_STATE;
    }
    else if( atomic_load( &_session_open ) )
    {
        err = ESP_OK;
    }
    else if( atomic_load( &_ble_released ) )
    {
        err = ESP_ERR_NOT_SUPPORTED;
    }
    else
    {
        err = _session_open_locked();
    }
    core2foraws_wifi_priv_unlock();
    return err;
}

esp_err_t core2foraws_wifi_provisioning_stop( void )
{
    esp_err_t err = core2foraws_wifi_priv_lock();
    if( err != ESP_OK ) return err;
    _session_close();
    core2foraws_wifi_priv_unlock();
    return ESP_OK;
}

esp_err_t core2foraws_wifi_provisioning_payload_get( char *payload, size_t len )
{
    if( payload == NULL ) return ESP_ERR_INVALID_ARG;
    if( len < CORE2FORAWS_WIFI_PROV_PAYLOAD_LEN ) return ESP_ERR_INVALID_SIZE;
    payload[ 0 ] = '\0';

    esp_err_t err = core2foraws_wifi_priv_lock();
    if( err != ESP_OK ) return err;
    if( atomic_load( &_session_open ) )
    {
        _payload_write( payload, len );
    }
    else
    {
        err = ESP_ERR_INVALID_STATE;
    }
    core2foraws_wifi_priv_unlock();
    return err;
}
