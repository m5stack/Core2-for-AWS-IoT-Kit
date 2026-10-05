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
 * @file core2foraws_wifi_prov.h
 * @brief Core2 for AWS IoT Kit BLE Wi-Fi provisioning APIs.
 *
 * Provisioning uses Espressif's
 * [iOS](https://apps.apple.com/in/app/esp-ble-provisioning/id1473590141) and
 * [Android](https://play.google.com/store/apps/details?id=com.espressif.provble)
 * BLE provisioning apps with Security 1. The service name is
 * `CORE2FORAWS_XXXXXX` and the proof of possession is derived from the MAC.
 *
 * A phone that is connected, or whose credentials are being applied, takes
 * priority: @ref core2foraws_wifi_connect and @ref core2foraws_wifi_scan return
 * ESP_ERR_INVALID_STATE until it finishes or disconnects.
 * @ref core2foraws_wifi_provisioning_stop is the explicit override.
 *
 * Requires `CONFIG_CORE2FORAWS_WIFI_PROVISIONING`.
 */

#ifndef _CORE2FORAWS_WIFI_PROV_H_
#define _CORE2FORAWS_WIFI_PROV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <esp_err.h>

/** @brief Buffer size, including the terminator, for @ref core2foraws_wifi_provisioning_payload_get. */
#define CORE2FORAWS_WIFI_PROV_PAYLOAD_LEN 78U

/** @brief Provisioning session phase reported by @ref core2foraws_wifi_provisioning_state_get. */
typedef enum
{
    CORE2FORAWS_WIFI_PROV_OFF = 0,          /**< No session */
    CORE2FORAWS_WIFI_PROV_WAITING,          /**< Advertising over BLE; no phone connected */
    CORE2FORAWS_WIFI_PROV_PHONE_CONNECTED,  /**< A phone is connected */
    CORE2FORAWS_WIFI_PROV_APPLYING,         /**< Trying credentials sent by the phone */
} core2foraws_wifi_prov_state_t;

/**
 * @brief Open a BLE provisioning session.
 *
 * Sets @ref WIFI_PROVISIONING_BIT. An existing link stays up while waiting for
 * a phone. Credentials that connect successfully become the saved network and
 * close the session. After `CONFIG_CORE2FORAWS_WIFI_PROV_MAX_FAILS` failed
 * credential attempts the provisioning state machine resets so the phone can retry.
 *
 * @return
 *  - ESP_OK                : Session open, or already open
 *  - ESP_ERR_INVALID_STATE : Wi-Fi not initialized, or a connect attempt is running
 *  - ESP_ERR_NOT_SUPPORTED : BLE memory was released earlier this boot
 *  - ESP_ERR_TIMEOUT       : Lifecycle lock was busy
 *  - Other                 : Provisioning manager error
 */
esp_err_t core2foraws_wifi_provisioning_start( void );

/**
 * @brief Close the provisioning session, even if a phone is connected.
 *
 * @return ESP_OK (including when no session is open) or ESP_ERR_TIMEOUT if the lifecycle lock was busy.
 */
esp_err_t core2foraws_wifi_provisioning_stop( void );

/**
 * @brief Get the JSON payload the provisioning app reads from a QR code.
 *
 * @param[out] payload Destination buffer.
 * @param[in]  len     Size of @p payload; at least @ref CORE2FORAWS_WIFI_PROV_PAYLOAD_LEN.
 * @return
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG   : @p payload is NULL
 *  - ESP_ERR_INVALID_SIZE  : @p len is too small
 *  - ESP_ERR_INVALID_STATE : No session is open
 *  - ESP_ERR_TIMEOUT       : Lifecycle lock was busy
 */
esp_err_t core2foraws_wifi_provisioning_payload_get( char *payload, size_t len );

/**
 * @brief Get the provisioning session phase.
 *
 * @return The current phase; @ref CORE2FORAWS_WIFI_PROV_OFF when no session is open.
 */
core2foraws_wifi_prov_state_t core2foraws_wifi_provisioning_state_get( void );

#ifdef __cplusplus
}
#endif
#endif
