/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include <stdint.h>
#include "esp_err.h"
#include "esp_event.h"

#define ESP_ERR_WIFI_NOT_STARTED 0x3004
#define ESP_ERR_WIFI_PASSWORD    0x300b
#define MAX_SSID_LEN 32
#define MAX_PASSPHRASE_LEN 64

typedef enum { WIFI_MODE_NULL = 0, WIFI_MODE_STA } wifi_mode_t;
typedef enum { WIFI_IF_STA = 0 } wifi_interface_t;
typedef enum { WIFI_FAST_SCAN = 0, WIFI_ALL_CHANNEL_SCAN } wifi_scan_method_t;
typedef enum { WIFI_CONNECT_AP_BY_SIGNAL = 0 } wifi_sort_method_t;
typedef enum { WPA3_SAE_PWE_BOTH = 2 } wifi_sae_pwe_method_t;

typedef struct
{
    uint8_t ssid[32];
    uint8_t password[64];
    wifi_scan_method_t scan_method;
    wifi_sort_method_t sort_method;
    wifi_sae_pwe_method_t sae_pwe_h2e;
} wifi_sta_config_t;

typedef union { wifi_sta_config_t sta; } wifi_config_t;
typedef struct { uint8_t ssid[33]; int8_t rssi; } wifi_ap_record_t;
typedef struct { int unused; } wifi_init_config_t;
#define WIFI_INIT_CONFIG_DEFAULT() { 0 }

enum
{
    WIFI_REASON_MIC_FAILURE = 14,
    WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT = 15,
    WIFI_REASON_802_1X_AUTH_FAILED = 23,
    WIFI_REASON_BEACON_TIMEOUT = 200,
    WIFI_REASON_NO_AP_FOUND = 201,
    WIFI_REASON_AUTH_FAIL = 202,
    WIFI_REASON_HANDSHAKE_TIMEOUT = 204,
    WIFI_REASON_NO_AP_FOUND_W_COMPATIBLE_SECURITY = 210,
    WIFI_REASON_NO_AP_FOUND_IN_AUTHMODE_THRESHOLD = 211,
    WIFI_REASON_NO_AP_FOUND_IN_RSSI_THRESHOLD = 212,
};

enum { WIFI_EVENT_STA_START = 2, WIFI_EVENT_STA_STOP = 3, WIFI_EVENT_STA_DISCONNECTED = 5 };
typedef struct { uint8_t reason; } wifi_event_sta_disconnected_t;
extern esp_event_base_t const WIFI_EVENT;

esp_err_t esp_wifi_init(const wifi_init_config_t *config);
esp_err_t esp_wifi_deinit(void);
esp_err_t esp_wifi_set_mode(wifi_mode_t mode);
esp_err_t esp_wifi_start(void);
esp_err_t esp_wifi_stop(void);
esp_err_t esp_wifi_connect(void);
esp_err_t esp_wifi_disconnect(void);
esp_err_t esp_wifi_set_config(wifi_interface_t interface, wifi_config_t *config);
esp_err_t esp_wifi_get_config(wifi_interface_t interface, wifi_config_t *config);
esp_err_t esp_wifi_scan_start(const void *config, bool block);
esp_err_t esp_wifi_scan_get_ap_records(uint16_t *count, wifi_ap_record_t *records);
esp_err_t esp_wifi_clear_ap_list(void);
