/*
 * Core2 for AWS IoT EduKit BSP v2.0.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

/**
 * @file core2foraws_crypto.c
 * @brief Core2 for AWS IoT EduKit cryptographic hardware driver APIs
 */

#include <stdint.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_freertos_hooks.h>
#include <freertos/semphr.h>

#include <mbedtls/config.h>
#include <mbedtls/atca_mbedtls_wrap.h>
#include <mbedtls/platform.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/pk.h>
#include <esp_log.h>

#include "cryptoauthlib.h"
#include "core2foraws_common.h"
#include "core2foraws_crypto.h"

static mbedtls_entropy_context _entropy;
static mbedtls_ctr_drbg_context _ctr_drbg;

static const char *_TAG = "CORE2FORAWS_CRYPTO";

static int _configure_mbedtls_rng( void );
static void _close_mbedtls_rng( void );

static esp_err_t _configure_mbedtls_rng( void )
{
    esp_err_t err;
    const char * seed = "\tAWS IoT EduKit random seed string";
    mbedtls_ctr_drbg_init( &_ctr_drbg );

    ESP_LOGD(_TAG, "\tSeeding mbedTLS random number generator");

    mbedtls_entropy_init( &_entropy );
    err = mbedtls_ctr_drbg_seed( &_ctr_drbg, mbedtls_entropy_func, &_entropy,
        ( const unsigned char * )seed, strlen( seed ) );
    
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "\tFailed to seed mbedTLS ring. 'mbedtls_ctr_drbg_seed' returned %d", err );
    } 
    else 
    {
        ESP_LOGD( _TAG, "\tSuccessfully seeded mbedTLS ring" );
    }

    return err;
}

static void _close_mbedtls_rng( void )
{
    fflush( stdout );
    mbedtls_ctr_drbg_free( &_ctr_drbg );
    mbedtls_entropy_free( &_entropy );
    ESP_LOGD( _TAG, "\tClosed mbedTLS ring" );
}

esp_err_t core2foraws_crypto_init( void )
{
    ESP_LOGI( _TAG, "\tInitializing" );

    ATCA_STATUS err = ATCA_SUCCESS;

    if ( _configure_mbedtls_rng() != ESP_OK )
    {
        return ESP_FAIL;
    }

    err |= atcab_init( &cfg_ateccx08a_i2c_default );
    
    if ( err != ATCA_SUCCESS ) 
    {
        ESP_LOGE( _TAG, "\tFailed to initialize ATECC608. atcab_init returned %x", err );
        _close_mbedtls_rng();
        return core2foraws_common_error( err );
    }
    ESP_LOGD( _TAG, "\tSuccessfully initialized ATECC608" );
    
    return core2foraws_common_error( err ); 
}

esp_err_t core2foraws_crypto_serial_get( char *serial_number )
{
    ATCA_STATUS err = ATCA_SUCCESS;
    uint8_t serial[ ATCA_SERIAL_NUM_SIZE ];
       
    err |= atcab_read_serial_number( serial );
    
    if ( err != ATCA_SUCCESS )
    {
        ESP_LOGE( _TAG, "\tFailed to read ATECC608 serial number. atcab_read_serial_number returned %x", err );
    }
    else
    {
        for ( size_t i = 0; i < ATCA_SERIAL_NUM_SIZE; i++ )
        {
            sprintf( serial_number + i * 2, "%02X", serial[ i ] );
        }
    }

    return core2foraws_common_error( err );

}

esp_err_t core2foraws_crypto_pubkey_base64_get( char *public_key )
{
    ATCA_STATUS err = ATCA_SUCCESS;
    
    size_t buf_len = CRYPTO_PUB_KEY_SIZE;
    uint8_t buf[ buf_len ];
    uint8_t * tmp;

    static const uint8_t public_key_x509_header[] = 
    {
        0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A,
        0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04
    };

    size_t public_key_x509_header_len = sizeof( public_key_x509_header );
    uint8_t pubkey[ ATCA_PUB_KEY_SIZE ];

    err |= atcab_get_pubkey( 0, pubkey );
    if ( err != ATCA_SUCCESS )
    {
        ESP_LOGE( _TAG, "\tFailed to get public key from ATECC608. atcab_get_pubkey returned %x", err );
        return core2foraws_common_error( err );
    }

    /* Calculate where the raw data will fit into the buffer */
    tmp = buf + sizeof( buf ) - ATCA_PUB_KEY_SIZE - public_key_x509_header_len;
    /* Copy the header */
    memcpy( tmp, public_key_x509_header, public_key_x509_header_len);

    /* Copy the key bytes */
    memcpy( tmp + public_key_x509_header_len, pubkey, ATCA_PUB_KEY_SIZE );

    /* Convert to base 64 */
    err |= atcab_base64encode( tmp, ATCA_PUB_KEY_SIZE + public_key_x509_header_len, ( char * )buf, &buf_len );
 
    memcpy( public_key, buf, buf_len );
    
    /* Add a null terminator */
    public_key[ buf_len ] = '\0';

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_crypto_sha256_sign( const unsigned char *message, uint8_t *signature, size_t *signature_length )
{
    int mbed_err = 0;
    mbedtls_pk_context pkey;

    mbed_err |= atca_mbedtls_pk_init( &pkey, 0 );
    if ( mbed_err != 0 )
    {
        ESP_LOGE( _TAG, "\tFailed to initialize private key access from ATECC608. atca_mbedtls_pk_init returned -0x%x", -mbed_err );
        return core2foraws_common_error( mbed_err );
    }

    mbed_err |= mbedtls_pk_sign( &pkey, MBEDTLS_MD_SHA256, message, 0, signature, signature_length, mbedtls_ctr_drbg_random, &_ctr_drbg );
    if (mbed_err != 0) {
        ESP_LOGE( _TAG, "\tFailed to sign message with ATECC608 private key. mbedtls_pk_sign returned -0x%x", -mbed_err );
    }

    return core2foraws_common_error( mbed_err );

}

esp_err_t core2foraws_crypto_sha256_verify( const unsigned char *message, const uint8_t *signature, const size_t signature_length, bool *verified )
{
    int mbed_err = 0;
    mbedtls_pk_context pkey;
    *verified = false;

    mbed_err |= atca_mbedtls_pk_init( &pkey, 0 );
    if ( mbed_err != 0 )
    {
        ESP_LOGE( _TAG, "\tFailed to initialize private key access from ATECC608. atca_mbedtls_pk_init returned -0x%x", -mbed_err );
        return core2foraws_common_error( mbed_err );
    }

    mbed_err |= mbedtls_pk_verify( &pkey, MBEDTLS_MD_SHA256, message, 0, signature, signature_length );
    
    if ( mbed_err != 0 )
    {
        ESP_LOGE( _TAG, "\tFailed to verify message with ATECC608 private key. mbedtls_pk_sign returned -0x%x", -mbed_err );
    }
    else
    {
        *verified = true;
    }

    return core2foraws_common_error( mbed_err );
}