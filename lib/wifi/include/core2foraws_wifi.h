/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_wifi.h
 * @brief Core2 for AWS IoT Kit Wi-Fi station APIs.
 *
 * The BSP owns the station interface: its connection, configuration, and
 * saved credentials. Applications may call read-only ESP-IDF functions such as
 * `esp_wifi_sta_get_ap_info()`, but must not call `esp_wifi_connect()`,
 * `esp_wifi_disconnect()`, `esp_wifi_set_config()`, or `esp_wifi_stop()`.
 *
 * Credentials are saved by the Wi-Fi driver in the default NVS partition, so
 * they survive reboots and application reflashes. @ref core2foraws_wifi_connect
 * and BLE provisioning (core2foraws_wifi_prov.h) share that single store.
 *
 * Typical boot sequence with `CONFIG_CORE2FORAWS_WIFI_PROVISIONING` enabled:
 * @code{c}
 *  #include "core2foraws.h"
 *
 *  void app_main( void )
 *  {
 *      ESP_ERROR_CHECK( core2foraws_init() );
 *      char ssid[ MAX_SSID_LEN + 1 ];
 *      esp_err_t err = core2foraws_wifi_saved_ssid_get( ssid );
 *      if( err == ESP_OK )
 *          ESP_ERROR_CHECK( core2foraws_wifi_reconnect( 0 ) );
 *      else if( err == ESP_ERR_NOT_FOUND )
 *          ESP_ERROR_CHECK( core2foraws_wifi_provisioning_start() );
 *      else
 *          ESP_ERROR_CHECK( err );
 *
 *      xEventGroupWaitBits( wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY );
 *  }
 * @endcode
 */

#ifndef _CORE2FORAWS_WIFI_H_
#define _CORE2FORAWS_WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>

/**
 * @brief @ref wifi_event_group bit set while the station has an IPv4 address.
 *
 * Exactly one of @ref WIFI_CONNECTED_BIT, @ref WIFI_CONNECTING_BIT, and
 * @ref WIFI_DISCONNECTED_BIT is set after @ref core2foraws_wifi_init.
 */
#define WIFI_CONNECTED_BIT      BIT0

/** @brief @ref wifi_event_group bit set while the station has no link and is not retrying. */
#define WIFI_DISCONNECTED_BIT   BIT1

/** @brief @ref wifi_event_group bit set while a connection attempt or automatic reconnect is in progress. */
#define WIFI_CONNECTING_BIT     BIT2

/**
 * @brief @ref wifi_event_group bit set while a BLE provisioning session is open.
 *
 * Independent of the link bits. Bits above BIT3 are reserved for the BSP.
 */
#define WIFI_PROVISIONING_BIT   BIT3

/** @brief Station link state reported by @ref core2foraws_wifi_state_get. */
typedef enum
{
    CORE2FORAWS_WIFI_STATE_IDLE = 0,     /**< Initialized, radio off */
    CORE2FORAWS_WIFI_STATE_DISCONNECTED, /**< Radio on, no link, not retrying */
    CORE2FORAWS_WIFI_STATE_CONNECTING,   /**< Attempting to connect or waiting to retry */
    CORE2FORAWS_WIFI_STATE_CONNECTED,    /**< Associated with an IPv4 address */
} core2foraws_wifi_state_t;

/**
 * @brief Wi-Fi state event group.
 *
 * Created by the first @ref core2foraws_wifi_init and never deleted, so tasks
 * may keep waiting on it across deinit/init cycles. NULL before first init.
 */
extern EventGroupHandle_t wifi_event_group;

/**
 * @brief Initialize the network stack and Wi-Fi driver in station mode.
 *
 * Called automatically by core2foraws_init() when the Wi-Fi module is enabled.
 * Idempotent. Does not start the radio, connect, or begin provisioning, and
 * never erases the default NVS partition.
 *
 * @return
 *  - ESP_OK          : Success or already initialized
 *  - ESP_ERR_NO_MEM  : Out of memory
 *  - ESP_ERR_TIMEOUT : Lifecycle lock was busy
 *  - Other           : NVS, netif, event-loop, or driver initialization error
 */
esp_err_t core2foraws_wifi_init( void );

/**
 * @brief Release BSP-owned Wi-Fi resources.
 *
 * Cancels a pending connect attempt, closes any provisioning session, stops
 * the radio, and deinitializes the driver. Saved credentials are kept and
 * @ref wifi_event_group is reset to @ref WIFI_DISCONNECTED_BIT. If the driver
 * fails to stop or deinitialize, resources are retained so the call can be retried.
 *
 * @return ESP_OK, ESP_ERR_TIMEOUT if the lifecycle lock was busy, or a driver error.
 */
esp_err_t core2foraws_wifi_deinit( void );

/**
 * @brief Blocking scan for nearby access points.
 *
 * Starts the radio if needed without connecting.
 *
 * @param[out]   records Destination for the scan results.
 * @param[inout] count   Capacity of @p records in, number of results out.
 * @return
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG   : NULL argument or zero capacity
 *  - ESP_ERR_INVALID_STATE : Not initialized, a connect attempt is running, or a phone provisioning session is active
 *  - Other                 : Driver scan error
 */
esp_err_t core2foraws_wifi_scan( wifi_ap_record_t *records, uint16_t *count );

/**
 * @brief Connect to a network with new credentials.
 *
 * The credentials become the saved network before the attempt starts and are
 * replaced with the previously saved network if the attempt fails, so a wrong
 * password never survives. On success the BSP reconnects automatically after
 * link loss. A provisioning session that is still waiting for a phone is
 * closed first; one with a connected phone takes priority and is left alone.
 *
 * @param[in] ssid       Network name, 1 to 32 bytes.
 * @param[in] password   Passphrase, or NULL/empty for an open network.
 * @param[in] timeout_ms Time to wait for the result. 0 returns once the
 *                       attempt starts; follow @ref wifi_event_group and
 *                       @ref core2foraws_wifi_last_error_get for the outcome.
 * @return
 *  - ESP_OK                : Connected with an IPv4 address, or the attempt started when @p timeout_ms is 0
 *  - ESP_ERR_INVALID_ARG   : Invalid SSID or password length
 *  - ESP_ERR_INVALID_STATE : Not initialized, another connect attempt is running, or a phone is provisioning
 *  - ESP_ERR_WIFI_PASSWORD : Authentication failed, typically an incorrect password
 *  - ESP_ERR_NOT_FOUND     : The network was not found
 *  - ESP_ERR_TIMEOUT       : No result within @p timeout_ms; the attempt is cancelled
 *  - ESP_FAIL              : Other connection failure
 */
esp_err_t core2foraws_wifi_connect( const char *ssid, const char *password, uint32_t timeout_ms );

/**
 * @brief Connect to the saved network and keep the link up.
 *
 * Retries with exponential backoff until connected, and again after any later
 * link loss, until @ref core2foraws_wifi_disconnect, @ref core2foraws_wifi_forget,
 * or an authentication failure.
 *
 * @param[in] timeout_ms Time to wait for a connection. 0 returns immediately.
 *                       On timeout the BSP keeps retrying in the background.
 * @return
 *  - ESP_OK                : Connected, or retrying started when @p timeout_ms is 0
 *  - ESP_ERR_NOT_FOUND     : No network is saved
 *  - ESP_ERR_INVALID_STATE : Not initialized, a connect attempt is running, or a phone is provisioning
 *  - ESP_ERR_WIFI_PASSWORD : The saved password was rejected; retrying has stopped
 *  - ESP_ERR_TIMEOUT       : Not yet connected within @p timeout_ms; still retrying
 */
esp_err_t core2foraws_wifi_reconnect( uint32_t timeout_ms );

/**
 * @brief Drop the link and stop automatic reconnection.
 *
 * Cancels a pending connect attempt. The link stays down until the next
 * @ref core2foraws_wifi_connect or @ref core2foraws_wifi_reconnect.
 *
 * @return ESP_OK, ESP_ERR_INVALID_STATE if not initialized, or ESP_ERR_TIMEOUT if the lifecycle lock was busy.
 */
esp_err_t core2foraws_wifi_disconnect( void );

/**
 * @brief Disconnect and erase the saved network.
 *
 * Other Wi-Fi driver settings and other NVS namespaces are untouched.
 *
 * @return
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_STATE : Not initialized, or a phone provisioning session is active
 *  - ESP_ERR_TIMEOUT       : Lifecycle lock was busy
 *  - Other                 : Driver configuration error
 */
esp_err_t core2foraws_wifi_forget( void );

/**
 * @brief Get the saved network name without starting the radio.
 *
 * @param[out] ssid Destination of at least `MAX_SSID_LEN + 1` bytes.
 * @return ESP_OK, ESP_ERR_NOT_FOUND if no network is saved, ESP_ERR_INVALID_ARG,
 *         ESP_ERR_INVALID_STATE if not initialized, or ESP_ERR_TIMEOUT if the lifecycle lock was busy.
 */
esp_err_t core2foraws_wifi_saved_ssid_get( char ssid[ MAX_SSID_LEN + 1 ] );

/**
 * @brief Get the current link state.
 *
 * @param[out] state Receives the link state; @ref CORE2FORAWS_WIFI_STATE_IDLE before init.
 * @return ESP_OK on success, or ESP_ERR_INVALID_ARG if @p state is NULL.
 */
esp_err_t core2foraws_wifi_state_get( core2foraws_wifi_state_t *state );

/**
 * @brief Get the reason the most recent connection attempt or link failed.
 *
 * Covers @ref core2foraws_wifi_connect, @ref core2foraws_wifi_reconnect,
 * automatic reconnects, and credentials sent by a phone during provisioning.
 * Reset to ESP_OK whenever the station obtains an IPv4 address.
 *
 * @return ESP_OK, ESP_ERR_WIFI_PASSWORD, ESP_ERR_NOT_FOUND, ESP_ERR_TIMEOUT, or ESP_FAIL.
 */
esp_err_t core2foraws_wifi_last_error_get( void );

#ifdef __cplusplus
}
#endif
#endif
