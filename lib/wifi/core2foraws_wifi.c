/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_wifi.c
 * @brief Core2 for AWS IoT Kit Wi-Fi station state machine.
 */

#include <string.h>
#include <stdatomic.h>
#include <sdkconfig.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <nvs_flash.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "core2foraws_wifi.h"
#include "core2foraws_wifi_priv.h"

#define WIFI_LINK_BITS           ( WIFI_CONNECTED_BIT | WIFI_DISCONNECTED_BIT | WIFI_CONNECTING_BIT )
/* Private wifi_event_group bits */
#define WIFI_ATTEMPT_DONE_BIT    BIT4
#define WIFI_LINK_DOWN_BIT       BIT5
#define WIFI_STA_STARTED_BIT     BIT6

#define WIFI_CONNECT_ATTEMPTS    3U
#define WIFI_RECONNECT_BASE_MS   1000U
#define WIFI_LINK_DOWN_WAIT_MS   500U
#define WIFI_RADIO_START_WAIT_MS 1000U
#define WIFI_LOCK_TIMEOUT_MS     1000U

typedef enum
{
    ATTEMPT_NONE = 0,
    ATTEMPT_ACTIVE,
    ATTEMPT_FINISHING,
} wifi_attempt_t;

static const char *_TAG = "CORE2FORAWS_WIFI";

EventGroupHandle_t wifi_event_group = NULL;
static StaticEventGroup_t _event_group_storage;

static StaticSemaphore_t _lock_storage;
static SemaphoreHandle_t _lock = NULL;
static atomic_uchar _lock_state;

static bool _initialized;
static esp_netif_t *_netif;
static esp_timer_handle_t _reconnect_timer;
static atomic_bool _auto_reconnect;
static atomic_uint _reconnect_failures;
static atomic_int _state = CORE2FORAWS_WIFI_STATE_IDLE;
static atomic_int _last_error;
static atomic_int _attempt = ATTEMPT_NONE;
static atomic_uint _attempt_failures;
/* Saved network restored when a connect attempt fails; owned by whoever claims the attempt. */
static wifi_config_t _previous;

esp_err_t core2foraws_wifi_priv_lock( void )
{
    if( atomic_load( &_lock_state ) != 2 )
    {
        unsigned char expected = 0;
        if( atomic_compare_exchange_strong( &_lock_state, &expected, 1 ) )
        {
            _lock = xSemaphoreCreateMutexStatic( &_lock_storage );
            atomic_store( &_lock_state, _lock != NULL ? 2 : 0 );
        }
        else
        {
            while( atomic_load( &_lock_state ) == 1 ) vTaskDelay( 1 );
        }
    }

    if( _lock == NULL ) return ESP_ERR_NO_MEM;
    return xSemaphoreTake( _lock, pdMS_TO_TICKS( WIFI_LOCK_TIMEOUT_MS ) ) == pdTRUE
               ? ESP_OK
               : ESP_ERR_TIMEOUT;
}

void core2foraws_wifi_priv_unlock( void )
{
    xSemaphoreGive( _lock );
}

bool core2foraws_wifi_priv_initialized( void )
{
    return _initialized;
}

bool core2foraws_wifi_priv_attempt_active( void )
{
    return atomic_load( &_attempt ) != ATTEMPT_NONE;
}

void core2foraws_wifi_priv_link_state_set( core2foraws_wifi_state_t state )
{
    atomic_store( &_state, state );
    EventBits_t bit = state == CORE2FORAWS_WIFI_STATE_CONNECTED  ? WIFI_CONNECTED_BIT :
                      state == CORE2FORAWS_WIFI_STATE_CONNECTING ? WIFI_CONNECTING_BIT :
                                                                   WIFI_DISCONNECTED_BIT;
    /* Set before clearing so a waiter never observes no link bit */
    xEventGroupSetBits( wifi_event_group, bit );
    xEventGroupClearBits( wifi_event_group, WIFI_LINK_BITS & ~bit );
}

void core2foraws_wifi_priv_last_error_set( esp_err_t err )
{
    atomic_store( &_last_error, err );
}

void core2foraws_wifi_priv_link_yield( void )
{
    atomic_store( &_auto_reconnect, false );
    if( _reconnect_timer != NULL ) ( void )esp_timer_stop( _reconnect_timer );
}

static esp_err_t _error_from_reason( uint8_t reason )
{
    switch( reason )
    {
        case WIFI_REASON_AUTH_FAIL:
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
        case WIFI_REASON_MIC_FAILURE:
        case WIFI_REASON_802_1X_AUTH_FAILED:
            return ESP_ERR_WIFI_PASSWORD;
        case WIFI_REASON_NO_AP_FOUND:
        case WIFI_REASON_NO_AP_FOUND_W_COMPATIBLE_SECURITY:
        case WIFI_REASON_NO_AP_FOUND_IN_AUTHMODE_THRESHOLD:
        case WIFI_REASON_NO_AP_FOUND_IN_RSSI_THRESHOLD:
            return ESP_ERR_NOT_FOUND;
        default:
            return ESP_FAIL;
    }
}

static bool _attempt_claim( void )
{
    int expected = ATTEMPT_ACTIVE;
    return atomic_compare_exchange_strong( &_attempt, &expected, ATTEMPT_FINISHING );
}

/* Only the caller that won _attempt_claim() may finish the attempt. */
static void _attempt_finish( esp_err_t result )
{
    if( result != ESP_OK )
    {
        esp_err_t err = esp_wifi_set_config( WIFI_IF_STA, &_previous );
        if( err != ESP_OK ) ESP_LOGE( _TAG, "\tFailed to restore the saved network: 0x%x", err );
        core2foraws_wifi_priv_last_error_set( result );
        core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_DISCONNECTED );
    }
    memset( &_previous, 0, sizeof( _previous ) );
    atomic_store( &_attempt, ATTEMPT_NONE );
    xEventGroupSetBits( wifi_event_group, WIFI_ATTEMPT_DONE_BIT );
}

static bool _radio_started( void )
{
    return ( xEventGroupGetBits( wifi_event_group ) & WIFI_STA_STARTED_BIT ) != 0;
}

/* Disconnect and wait briefly for the driver's event so it cannot be misread later. */
static void _station_disconnect_wait( void )
{
    xEventGroupClearBits( wifi_event_group, WIFI_LINK_DOWN_BIT );
    if( esp_wifi_disconnect() == ESP_OK )
    {
        xEventGroupWaitBits( wifi_event_group, WIFI_LINK_DOWN_BIT, pdTRUE, pdTRUE,
                             pdMS_TO_TICKS( WIFI_LINK_DOWN_WAIT_MS ) );
    }
}

static void _attempt_cancel( esp_err_t result )
{
    if( !_attempt_claim() ) return;
    _station_disconnect_wait();
    _attempt_finish( result );
}

static void _reconnect_now( void )
{
    if( !atomic_load( &_auto_reconnect ) || atomic_load( &_attempt ) != ATTEMPT_NONE ) return;

    esp_err_t err = esp_wifi_connect();
    if( err != ESP_OK ) ESP_LOGE( _TAG, "\tReconnect failed to start: 0x%x", err );
}

static void _on_reconnect_timer( void *arg )
{
    ( void )arg;
    _reconnect_now();
}

static void _reconnect_schedule( uint8_t reason )
{
    /* The first retry is immediate; later ones back off exponentially up to the Kconfig cap */
    unsigned int failures = atomic_fetch_add( &_reconnect_failures, 1 );
    uint64_t delay_ms = 0;
    if( failures > 0 )
    {
        unsigned int shift = failures - 1 < 16 ? failures - 1 : 16;
        delay_ms = ( uint64_t )WIFI_RECONNECT_BASE_MS << shift;
        uint64_t max_ms = ( uint64_t )CONFIG_CORE2FORAWS_WIFI_RECONNECT_MAX_BACKOFF_S * 1000U;
        if( delay_ms > max_ms ) delay_ms = max_ms;
    }

    ESP_LOGW( _TAG, "\tLink lost (reason %u), reconnect attempt %u in %llu ms",
              reason, failures + 1, ( unsigned long long )delay_ms );

    if( delay_ms == 0 || _reconnect_timer == NULL )
    {
        _reconnect_now();
        return;
    }

    ( void )esp_timer_stop( _reconnect_timer );
    esp_err_t err = esp_timer_start_once( _reconnect_timer, delay_ms * 1000U );
    if( err != ESP_OK ) ESP_LOGE( _TAG, "\tFailed to schedule reconnect: 0x%x", err );
}

static void _on_got_ip( void *arg, esp_event_base_t base, int32_t id, void *data )
{
    ( void )arg; ( void )base; ( void )id;
    const ip_event_got_ip_t *event = data;
    ESP_LOGI( _TAG, "\tGot IPv4 address: " IPSTR, IP2STR( &event->ip_info.ip ) );

    atomic_store( &_reconnect_failures, 0 );
    atomic_store( &_auto_reconnect, true );
    core2foraws_wifi_priv_last_error_set( ESP_OK );
    core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_CONNECTED );
    if( _attempt_claim() ) _attempt_finish( ESP_OK );
    core2foraws_wifi_priv_prov_on_connected();
}

static void _on_sta_start( void *arg, esp_event_base_t base, int32_t id, void *data )
{
    ( void )arg; ( void )base; ( void )id; ( void )data;
    xEventGroupSetBits( wifi_event_group, WIFI_STA_STARTED_BIT );
    if( atomic_load( &_state ) == CORE2FORAWS_WIFI_STATE_IDLE )
    {
        core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_DISCONNECTED );
    }
}

static void _on_sta_stop( void *arg, esp_event_base_t base, int32_t id, void *data )
{
    ( void )arg; ( void )base; ( void )id; ( void )data;
    xEventGroupClearBits( wifi_event_group, WIFI_STA_STARTED_BIT );
    core2foraws_wifi_priv_link_yield();
    core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_IDLE );
}

static void _on_sta_disconnected( void *arg, esp_event_base_t base, int32_t id, void *data )
{
    ( void )arg; ( void )base; ( void )id;
    const wifi_event_sta_disconnected_t *event = data;
    uint8_t reason = event != NULL ? event->reason : 0;
    esp_err_t err = _error_from_reason( reason );
    xEventGroupSetBits( wifi_event_group, WIFI_LINK_DOWN_BIT );

    if( atomic_load( &_attempt ) == ATTEMPT_ACTIVE )
    {
        /* A wrong password fails the same way every time, so only retry other reasons */
        if( err != ESP_ERR_WIFI_PASSWORD &&
            atomic_fetch_add( &_attempt_failures, 1 ) + 1 < WIFI_CONNECT_ATTEMPTS &&
            esp_wifi_connect() == ESP_OK )
        {
            ESP_LOGW( _TAG, "\tConnect attempt failed (reason %u), retrying", reason );
            return;
        }
        ESP_LOGW( _TAG, "\tConnect failed (reason %u)", reason );
        if( _attempt_claim() ) _attempt_finish( err );
        return;
    }

    /* The provisioning manager drives the station while it applies phone credentials */
    if( core2foraws_wifi_priv_prov_state() == CORE2FORAWS_WIFI_PROV_APPLYING ) return;

    if( !atomic_load( &_auto_reconnect ) )
    {
        ESP_LOGI( _TAG, "\tDisconnected (reason %u)", reason );
        core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_DISCONNECTED );
        return;
    }

    core2foraws_wifi_priv_last_error_set( err );
    /* One handshake failure can be radio noise; repeated ones mean the saved password changed */
    if( err == ESP_ERR_WIFI_PASSWORD &&
        atomic_load( &_reconnect_failures ) + 1 >= WIFI_CONNECT_ATTEMPTS )
    {
        ESP_LOGW( _TAG, "\tSaved password rejected (reason %u); not retrying", reason );
        core2foraws_wifi_priv_link_yield();
        core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_DISCONNECTED );
        return;
    }

    core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_CONNECTING );
    _reconnect_schedule( reason );
}

typedef struct
{
    const esp_event_base_t *base;
    int32_t id;
    esp_event_handler_t handler;
} wifi_handler_t;

static const wifi_handler_t _handlers[] =
{
    { &IP_EVENT,   IP_EVENT_STA_GOT_IP,         _on_got_ip },
    { &WIFI_EVENT, WIFI_EVENT_STA_START,        _on_sta_start },
    { &WIFI_EVENT, WIFI_EVENT_STA_STOP,         _on_sta_stop },
    { &WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, _on_sta_disconnected },
};
#define WIFI_HANDLER_COUNT ( sizeof( _handlers ) / sizeof( _handlers[ 0 ] ) )

static esp_err_t _handlers_unregister( size_t count )
{
    esp_err_t ret = ESP_OK;
    while( count-- > 0 )
    {
        esp_err_t err = esp_event_handler_unregister( *_handlers[ count ].base, _handlers[ count ].id,
                                                      _handlers[ count ].handler );
        if( err != ESP_OK && ret == ESP_OK ) ret = err;
    }
    return ret;
}

static esp_err_t _init_locked( void )
{
    if( _initialized ) return ESP_OK;

    if( wifi_event_group == NULL )
    {
        wifi_event_group = xEventGroupCreateStatic( &_event_group_storage );
        core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_IDLE );
    }

    esp_err_t err = nvs_flash_init();
    if( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "NVS initialization failed without erasing application data: 0x%x", err );
        return err;
    }

    err = esp_netif_init();
    if( err != ESP_OK ) return err;

    err = esp_event_loop_create_default();
    if( err != ESP_OK && err != ESP_ERR_INVALID_STATE ) return err;

    const esp_timer_create_args_t timer_args =
    {
        .callback = _on_reconnect_timer,
        .name = "wifiReconnect",
    };
    err = esp_timer_create( &timer_args, &_reconnect_timer );
    if( err != ESP_OK )
    {
        _reconnect_timer = NULL;
        return err;
    }

    size_t registered = 0;
    bool driver_initialized = false;
    _netif = esp_netif_create_default_wifi_sta();
    if( _netif == NULL )
    {
        err = ESP_ERR_NO_MEM;
        goto cleanup;
    }

    for( ; registered < WIFI_HANDLER_COUNT; registered++ )
    {
        err = esp_event_handler_register( *_handlers[ registered ].base, _handlers[ registered ].id,
                                          _handlers[ registered ].handler, NULL );
        if( err != ESP_OK ) goto cleanup;
    }

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init( &config );
    if( err != ESP_OK ) goto cleanup;
    driver_initialized = true;

    err = esp_wifi_set_mode( WIFI_MODE_STA );
    if( err == ESP_OK ) err = core2foraws_wifi_priv_prov_init();
    if( err != ESP_OK ) goto cleanup;

    _initialized = true;
    ESP_LOGD( _TAG, "Initialized" );
    return ESP_OK;

cleanup:
    ESP_LOGE( _TAG, "Wi-Fi initialization failed: 0x%x", err );
    if( driver_initialized ) ( void )esp_wifi_deinit();
    ( void )_handlers_unregister( registered );
    if( _netif != NULL )
    {
        esp_wifi_clear_default_wifi_driver_and_handlers( _netif );
        esp_netif_destroy( _netif );
        _netif = NULL;
    }
    esp_timer_delete( _reconnect_timer );
    _reconnect_timer = NULL;
    return err;
}

esp_err_t core2foraws_wifi_init( void )
{
    esp_err_t err = core2foraws_wifi_priv_lock();
    if( err != ESP_OK ) return err;
    err = _init_locked();
    core2foraws_wifi_priv_unlock();
    return err;
}

static esp_err_t _deinit_locked( void )
{
    if( !_initialized ) return ESP_OK;

    _attempt_cancel( ESP_FAIL );
    core2foraws_wifi_priv_prov_stop();
    core2foraws_wifi_priv_link_yield();

    esp_err_t err = esp_wifi_stop();
    if( err != ESP_OK && err != ESP_ERR_WIFI_NOT_STARTED ) return err;
    err = esp_wifi_deinit();
    if( err != ESP_OK ) return err;

    core2foraws_wifi_priv_prov_deinit();
    esp_err_t ret = _handlers_unregister( WIFI_HANDLER_COUNT );
    err = esp_wifi_clear_default_wifi_driver_and_handlers( _netif );
    if( err != ESP_OK && ret == ESP_OK ) ret = err;
    esp_netif_destroy( _netif );
    _netif = NULL;

    err = esp_timer_delete( _reconnect_timer );
    if( err != ESP_OK && ret == ESP_OK ) ret = err;
    _reconnect_timer = NULL;
    atomic_store( &_reconnect_failures, 0 );

    xEventGroupClearBits( wifi_event_group, WIFI_STA_STARTED_BIT | WIFI_PROVISIONING_BIT );
    core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_IDLE );
    _initialized = false;
    return ret;
}

esp_err_t core2foraws_wifi_deinit( void )
{
    esp_err_t err = core2foraws_wifi_priv_lock();
    if( err != ESP_OK ) return err;
    err = _deinit_locked();
    core2foraws_wifi_priv_unlock();
    return err;
}

static esp_err_t _radio_start_locked( void )
{
    if( _radio_started() ) return ESP_OK;

    esp_err_t err = esp_wifi_start();
    if( err != ESP_OK ) return err;
    EventBits_t bits = xEventGroupWaitBits( wifi_event_group, WIFI_STA_STARTED_BIT, pdFALSE, pdTRUE,
                                            pdMS_TO_TICKS( WIFI_RADIO_START_WAIT_MS ) );
    return ( bits & WIFI_STA_STARTED_BIT ) != 0 ? ESP_OK : ESP_ERR_TIMEOUT;
}

/* The station is free for device-side use: no attempt running and no phone mid-provisioning. */
static esp_err_t _station_available_locked( void )
{
    if( !_initialized || atomic_load( &_attempt ) != ATTEMPT_NONE ) return ESP_ERR_INVALID_STATE;

    core2foraws_wifi_prov_state_t prov = core2foraws_wifi_priv_prov_state();
    if( prov == CORE2FORAWS_WIFI_PROV_PHONE_CONNECTED || prov == CORE2FORAWS_WIFI_PROV_APPLYING )
    {
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

/* A session still waiting for a phone yields to device-side connection requests. */
static void _waiting_session_close_locked( void )
{
    if( core2foraws_wifi_priv_prov_state() == CORE2FORAWS_WIFI_PROV_WAITING )
    {
        core2foraws_wifi_priv_prov_stop();
    }
}

static void _link_drop_locked( void )
{
    core2foraws_wifi_priv_link_yield();
    if( !_radio_started() ) return;

    core2foraws_wifi_state_t state = atomic_load( &_state );
    if( state == CORE2FORAWS_WIFI_STATE_CONNECTED || state == CORE2FORAWS_WIFI_STATE_CONNECTING )
    {
        _station_disconnect_wait();
    }
    core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_DISCONNECTED );
}

esp_err_t core2foraws_wifi_scan( wifi_ap_record_t *records, uint16_t *count )
{
    if( records == NULL || count == NULL || *count == 0 ) return ESP_ERR_INVALID_ARG;

    esp_err_t err = core2foraws_wifi_priv_lock();
    if( err != ESP_OK ) return err;
    err = _station_available_locked();
    if( err == ESP_OK ) err = _radio_start_locked();
    if( err == ESP_OK )
    {
        err = esp_wifi_scan_start( NULL, true );
        if( err == ESP_OK ) err = esp_wifi_scan_get_ap_records( count, records );
        if( err != ESP_OK ) esp_wifi_clear_ap_list();
    }
    core2foraws_wifi_priv_unlock();
    return err;
}

static esp_err_t _attempt_start_locked( const char *ssid, size_t ssid_len,
                                        const char *password, size_t pass_len )
{
    esp_err_t err = _station_available_locked();
    if( err != ESP_OK ) return err;
    _waiting_session_close_locked();

    err = _radio_start_locked();
    if( err == ESP_OK ) err = esp_wifi_get_config( WIFI_IF_STA, &_previous );
    if( err != ESP_OK ) return err;

    _link_drop_locked();

    wifi_config_t config = { 0 };
    memcpy( config.sta.ssid, ssid, ssid_len );
    memcpy( config.sta.password, password, pass_len );
    config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    err = esp_wifi_set_config( WIFI_IF_STA, &config );
    memset( &config, 0, sizeof( config ) );
    if( err != ESP_OK )
    {
        memset( &_previous, 0, sizeof( _previous ) );
        return err;
    }

    atomic_store( &_attempt_failures, 0 );
    xEventGroupClearBits( wifi_event_group, WIFI_ATTEMPT_DONE_BIT );
    core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_CONNECTING );
    atomic_store( &_attempt, ATTEMPT_ACTIVE );

    err = esp_wifi_connect();
    if( err != ESP_OK && _attempt_claim() ) _attempt_finish( err );
    return err;
}

static esp_err_t _attempt_wait( uint32_t timeout_ms )
{
    EventBits_t bits = xEventGroupWaitBits( wifi_event_group, WIFI_ATTEMPT_DONE_BIT, pdFALSE, pdTRUE,
                                            pdMS_TO_TICKS( timeout_ms ) );
    if( ( bits & WIFI_ATTEMPT_DONE_BIT ) == 0 )
    {
        if( core2foraws_wifi_priv_lock() == ESP_OK )
        {
            _attempt_cancel( ESP_ERR_TIMEOUT );
            core2foraws_wifi_priv_unlock();
        }
        /* Another path may have finished it concurrently; wait for that result */
        xEventGroupWaitBits( wifi_event_group, WIFI_ATTEMPT_DONE_BIT, pdFALSE, pdTRUE,
                             pdMS_TO_TICKS( WIFI_LOCK_TIMEOUT_MS ) );
    }

    if( atomic_load( &_state ) == CORE2FORAWS_WIFI_STATE_CONNECTED ) return ESP_OK;
    esp_err_t err = atomic_load( &_last_error );
    return err != ESP_OK ? err : ESP_FAIL;
}

esp_err_t core2foraws_wifi_connect( const char *ssid, const char *password, uint32_t timeout_ms )
{
    if( password == NULL ) password = "";
    size_t ssid_len = ssid != NULL ? strnlen( ssid, MAX_SSID_LEN + 1 ) : 0;
    size_t pass_len = strnlen( password, MAX_PASSPHRASE_LEN + 1 );
    if( ssid_len == 0 || ssid_len > MAX_SSID_LEN || pass_len > MAX_PASSPHRASE_LEN )
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = core2foraws_wifi_priv_lock();
    if( err != ESP_OK ) return err;
    err = _attempt_start_locked( ssid, ssid_len, password, pass_len );
    core2foraws_wifi_priv_unlock();

    if( err != ESP_OK || timeout_ms == 0 ) return err;
    return _attempt_wait( timeout_ms );
}

static esp_err_t _reconnect_start_locked( void )
{
    esp_err_t err = _station_available_locked();
    if( err != ESP_OK ) return err;

    wifi_config_t saved;
    err = esp_wifi_get_config( WIFI_IF_STA, &saved );
    bool has_saved = err == ESP_OK && saved.sta.ssid[ 0 ] != '\0';
    memset( &saved, 0, sizeof( saved ) );
    if( err != ESP_OK ) return err;
    if( !has_saved ) return ESP_ERR_NOT_FOUND;

    _waiting_session_close_locked();
    err = _radio_start_locked();
    if( err != ESP_OK ) return err;
    if( atomic_load( &_state ) == CORE2FORAWS_WIFI_STATE_CONNECTED ) return ESP_OK;

    atomic_store( &_reconnect_failures, 0 );
    atomic_store( &_auto_reconnect, true );
    core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_CONNECTING );
    err = esp_wifi_connect();
    if( err != ESP_OK )
    {
        core2foraws_wifi_priv_link_yield();
        core2foraws_wifi_priv_link_state_set( CORE2FORAWS_WIFI_STATE_DISCONNECTED );
    }
    return err;
}

esp_err_t core2foraws_wifi_reconnect( uint32_t timeout_ms )
{
    esp_err_t err = core2foraws_wifi_priv_lock();
    if( err != ESP_OK ) return err;
    err = _reconnect_start_locked();
    core2foraws_wifi_priv_unlock();
    if( err != ESP_OK || timeout_ms == 0 ) return err;

    EventBits_t bits = xEventGroupWaitBits( wifi_event_group,
                                            WIFI_CONNECTED_BIT | WIFI_DISCONNECTED_BIT,
                                            pdFALSE, pdFALSE, pdMS_TO_TICKS( timeout_ms ) );
    if( bits & WIFI_CONNECTED_BIT ) return ESP_OK;
    if( bits & WIFI_DISCONNECTED_BIT )
    {
        err = atomic_load( &_last_error );
        return err != ESP_OK ? err : ESP_FAIL;
    }
    return ESP_ERR_TIMEOUT;
}

esp_err_t core2foraws_wifi_disconnect( void )
{
    esp_err_t err = core2foraws_wifi_priv_lock();
    if( err != ESP_OK ) return err;
    if( !_initialized )
    {
        err = ESP_ERR_INVALID_STATE;
    }
    else
    {
        _attempt_cancel( ESP_FAIL );
        _link_drop_locked();
    }
    core2foraws_wifi_priv_unlock();
    return err;
}

esp_err_t core2foraws_wifi_forget( void )
{
    esp_err_t err = core2foraws_wifi_priv_lock();
    if( err != ESP_OK ) return err;

    core2foraws_wifi_prov_state_t prov = core2foraws_wifi_priv_prov_state();
    if( !_initialized || prov == CORE2FORAWS_WIFI_PROV_PHONE_CONNECTED ||
        prov == CORE2FORAWS_WIFI_PROV_APPLYING )
    {
        err = ESP_ERR_INVALID_STATE;
    }
    else
    {
        _attempt_cancel( ESP_FAIL );
        _link_drop_locked();
        wifi_config_t empty = { 0 };
        err = esp_wifi_set_config( WIFI_IF_STA, &empty );
    }
    core2foraws_wifi_priv_unlock();
    return err;
}

esp_err_t core2foraws_wifi_saved_ssid_get( char ssid[ MAX_SSID_LEN + 1 ] )
{
    if( ssid == NULL ) return ESP_ERR_INVALID_ARG;
    ssid[ 0 ] = '\0';

    esp_err_t err = core2foraws_wifi_priv_lock();
    if( err != ESP_OK ) return err;
    if( !_initialized )
    {
        err = ESP_ERR_INVALID_STATE;
    }
    else
    {
        wifi_config_t config;
        err = esp_wifi_get_config( WIFI_IF_STA, &config );
        if( err == ESP_OK )
        {
            size_t len = strnlen( ( const char * )config.sta.ssid, MAX_SSID_LEN );
            memcpy( ssid, config.sta.ssid, len );
            ssid[ len ] = '\0';
            if( len == 0 ) err = ESP_ERR_NOT_FOUND;
        }
        memset( &config, 0, sizeof( config ) );
    }
    core2foraws_wifi_priv_unlock();
    return err;
}

esp_err_t core2foraws_wifi_state_get( core2foraws_wifi_state_t *state )
{
    if( state == NULL ) return ESP_ERR_INVALID_ARG;
    *state = ( core2foraws_wifi_state_t )atomic_load( &_state );
    return ESP_OK;
}

esp_err_t core2foraws_wifi_last_error_get( void )
{
    return atomic_load( &_last_error );
}
