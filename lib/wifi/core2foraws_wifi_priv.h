/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_wifi_priv.h
 * @brief Internal hooks shared by the Wi-Fi station and provisioning modules.
 */

#pragma once

#include <stdbool.h>
#include <sdkconfig.h>

#include "core2foraws_wifi.h"
#include "core2foraws_wifi_prov.h"

esp_err_t core2foraws_wifi_priv_lock( void );
void core2foraws_wifi_priv_unlock( void );

/* Call with the lifecycle lock held. */
bool core2foraws_wifi_priv_initialized( void );
bool core2foraws_wifi_priv_attempt_active( void );

void core2foraws_wifi_priv_link_state_set( core2foraws_wifi_state_t state );
void core2foraws_wifi_priv_last_error_set( esp_err_t err );
/* Stop automatic reconnects so another owner can drive the station. */
void core2foraws_wifi_priv_link_yield( void );

#ifdef CONFIG_CORE2FORAWS_WIFI_PROVISIONING
/* Implemented by core2foraws_wifi_prov.c. Lock-held unless noted. */
esp_err_t core2foraws_wifi_priv_prov_init( void );
void core2foraws_wifi_priv_prov_deinit( void );
void core2foraws_wifi_priv_prov_stop( void );
core2foraws_wifi_prov_state_t core2foraws_wifi_priv_prov_state( void );
/* Event task, after the station obtains an IPv4 address. */
void core2foraws_wifi_priv_prov_on_connected( void );
#else
static inline esp_err_t core2foraws_wifi_priv_prov_init( void ) { return ESP_OK; }
static inline void core2foraws_wifi_priv_prov_deinit( void ) {}
static inline void core2foraws_wifi_priv_prov_stop( void ) {}
static inline core2foraws_wifi_prov_state_t core2foraws_wifi_priv_prov_state( void ) { return CORE2FORAWS_WIFI_PROV_OFF; }
static inline void core2foraws_wifi_priv_prov_on_connected( void ) {}
#endif
