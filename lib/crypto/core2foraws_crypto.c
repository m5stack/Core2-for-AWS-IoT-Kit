/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_crypto.c
 * @brief Core2 for AWS IoT Kit cryptographic hardware driver APIs
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>

#include "cryptoauthlib.h"
#include "core2foraws_common.h"
#include "core2foraws_crypto.h"

#define ATECC608_I2C_ADDRESS_8BIT 0x6A
#define ATECC608_I2C_BAUD_HZ      100000
#define CRYPTO_LOCK_TIMEOUT_MS    1000U

_Static_assert( CRYPTO_MAX_SIGNATURE_SIZE == ATCA_SIG_SIZE,
                "CRYPTO_MAX_SIGNATURE_SIZE must match ATCA_SIG_SIZE" );
_Static_assert( CRYPTO_SERIAL_STR_SIZE == ( ATCA_SERIAL_NUM_SIZE * 2 + 1 ),
                "CRYPTO_SERIAL_STR_SIZE must match ATCA_SERIAL_NUM_SIZE" );

static ATCAIfaceCfg _crypto_iface_cfg;

static const char *_TAG = "CORE2FORAWS_CRYPTO";
static bool _crypto_initialized = false;
static StaticSemaphore_t _crypto_mutex_storage;
static SemaphoreHandle_t _crypto_mutex = NULL;
static atomic_uchar _crypto_mutex_state;

static esp_err_t _crypto_lock( void )
{
    SemaphoreHandle_t mutex = core2foraws_common_mutex_ensure(
        &_crypto_mutex_state, &_crypto_mutex, &_crypto_mutex_storage,
        xSemaphoreCreateMutexStatic );
    if( mutex == NULL )
    {
        return ESP_ERR_NO_MEM;
    }
    return xSemaphoreTake(
               mutex, CORE2FORAWS_DELAY_MS_TO_TICKS( CRYPTO_LOCK_TIMEOUT_MS ) ) ==
                   pdTRUE
               ? ESP_OK
               : ESP_ERR_TIMEOUT;
}

static void _crypto_unlock( void )
{
    xSemaphoreGive( _crypto_mutex );
}

esp_err_t core2foraws_crypto_init( void )
{
    ESP_LOGI( _TAG, "\tInitializing" );

    esp_err_t lock_err = _crypto_lock();
    if( lock_err != ESP_OK )
    {
        return lock_err;
    }

    if ( _crypto_initialized )
    {
        _crypto_unlock();
        return ESP_OK;
    }

    _crypto_iface_cfg = cfg_ateccx08a_i2c_default;
    _crypto_iface_cfg.atcai2c.address = ATECC608_I2C_ADDRESS_8BIT;
    _crypto_iface_cfg.atcai2c.baud = ATECC608_I2C_BAUD_HZ;

    ATCA_STATUS err = atcab_init( &_crypto_iface_cfg );

    if ( err != ATCA_SUCCESS )
    {
        _crypto_unlock();
        ESP_LOGE( _TAG, "\tFailed to initialize ATECC608. atcab_init returned %x", err );
        return core2foraws_common_error( err );
    }

    _crypto_initialized = true;
    _crypto_unlock();
    ESP_LOGD( _TAG, "\tSuccessfully initialized ATECC608" );

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_crypto_serial_get( char *serial_number )
{
    if ( serial_number == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t lock_err = _crypto_lock();
    if( lock_err != ESP_OK )
    {
        return lock_err;
    }
    if( !_crypto_initialized )
    {
        _crypto_unlock();
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t serial[ ATCA_SERIAL_NUM_SIZE ];

    ATCA_STATUS err = atcab_read_serial_number( serial );
    _crypto_unlock();

    if ( err != ATCA_SUCCESS )
    {
        ESP_LOGE( _TAG, "\tFailed to read ATECC608 serial number. atcab_read_serial_number returned %x", err );
    }
    else
    {
        for ( size_t i = 0; i < ATCA_SERIAL_NUM_SIZE; i++ )
        {
            snprintf( serial_number + i * 2, 3, "%02X", serial[ i ] );
        }

        serial_number[ CRYPTO_SERIAL_STR_SIZE - 1 ] = '\0';
    }

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_crypto_pubkey_base64_get( char *public_key )
{
    if ( public_key == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t lock_err = _crypto_lock();
    if( lock_err != ESP_OK )
    {
        return lock_err;
    }
    if( !_crypto_initialized )
    {
        _crypto_unlock();
        return ESP_ERR_INVALID_STATE;
    }

    size_t buf_len = CRYPTO_PUB_KEY_SIZE;
    uint8_t buf[ buf_len ];
    uint8_t *tmp;

    static const uint8_t public_key_x509_header[] =
    {
        0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A,
        0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04
    };

    size_t public_key_x509_header_len = sizeof( public_key_x509_header );
    uint8_t pubkey[ ATCA_PUB_KEY_SIZE ];

    ATCA_STATUS err = atcab_get_pubkey( 0, pubkey );
    _crypto_unlock();
    if ( err != ATCA_SUCCESS )
    {
        ESP_LOGE( _TAG, "\tFailed to get public key from ATECC608. atcab_get_pubkey returned %x", err );
        return core2foraws_common_error( err );
    }

    tmp = buf + sizeof( buf ) - ATCA_PUB_KEY_SIZE - public_key_x509_header_len;
    memcpy( tmp, public_key_x509_header, public_key_x509_header_len );
    memcpy( tmp + public_key_x509_header_len, pubkey, ATCA_PUB_KEY_SIZE );

    err = atcab_base64encode( tmp, ATCA_PUB_KEY_SIZE + public_key_x509_header_len, ( char * )buf, &buf_len );
    if ( err != ATCA_SUCCESS )
    {
        ESP_LOGE( _TAG, "\tFailed to base64 encode public key. atcab_base64encode returned %x", err );
        return core2foraws_common_error( err );
    }

    if ( buf_len >= CRYPTO_PUB_KEY_SIZE )
    {
        return ESP_FAIL;
    }

    memcpy( public_key, buf, buf_len );
    public_key[ buf_len ] = '\0';

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_crypto_sha256_sign( const unsigned char *message, uint8_t *signature, size_t *signature_length )
{
    if ( message == NULL || signature == NULL || signature_length == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    *signature_length = 0;
    esp_err_t lock_err = _crypto_lock();
    if( lock_err != ESP_OK )
    {
        return lock_err;
    }
    if( !_crypto_initialized )
    {
        _crypto_unlock();
        return ESP_ERR_INVALID_STATE;
    }

    ATCA_STATUS err = atcab_sign( 0, message, signature );
    _crypto_unlock();
    if ( err != ATCA_SUCCESS )
    {
        ESP_LOGE( _TAG, "\tFailed to sign digest with ATECC608. atcab_sign returned 0x%x", err );
        return core2foraws_common_error( err );
    }

    *signature_length = ATCA_SIG_SIZE;
    return ESP_OK;
}

esp_err_t core2foraws_crypto_sha256_verify( const unsigned char *message, const uint8_t *signature, const size_t signature_length, bool *verified )
{
    if ( message == NULL || signature == NULL || verified == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    *verified = false;
    if ( signature_length != ATCA_SIG_SIZE )
    {
        return ESP_ERR_INVALID_SIZE;
    }

    esp_err_t lock_err = _crypto_lock();
    if( lock_err != ESP_OK )
    {
        return lock_err;
    }
    if( !_crypto_initialized )
    {
        _crypto_unlock();
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t public_key[ ATCA_PUB_KEY_SIZE ];
    ATCA_STATUS err = atcab_get_pubkey( 0, public_key );
    if ( err == ATCA_SUCCESS )
    {
        err = atcab_verify_extern( message, signature, public_key, verified );
    }
    _crypto_unlock();

    if ( err != ATCA_SUCCESS )
    {
        ESP_LOGE( _TAG, "\tFailed to verify digest with ATECC608. CryptoAuthLib returned 0x%x", err );
    }

    return core2foraws_common_error( err );
}
