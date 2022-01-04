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
 * @file core2foraws_crypto.h
 * @brief Core2 for AWS IoT EduKit cryptographic hardware driver APIs
 */

#ifndef _CORE2FORAWS_CRYPTO_H_
#define _CORE2FORAWS_CRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <esp_err.h>
#include "cryptoauthlib.h"

/**
 * @brief The size of the public key string
 */
/* @[declare_core2foraws_crypto_pub_key_size] */
#define CRYPTO_PUB_KEY_SIZE 128U
/* @[declare_core2foraws_crypto_pub_key_size] */

/**
 * @brief The maximum crypto signature size supported by the library
 */
/* @[declare_core2foraws_crypto_max_signature_size] */
#define CRYPTO_MAX_SIGNATURE_SIZE MBEDTLS_MPI_MAX_SIZE
/* @[declare_core2foraws_crypto_max_signature_size] */

/**
 * @brief The size of the serial number as a string.
 * 
 * @note Length includes 1 byte for null terminator.
 */
/* @[declare_core2foraws_crypto_strial_str_size] */
#define CRYPTO_SERIAL_STR_SIZE ( ATCA_SERIAL_NUM_SIZE * 2 + 1 )
/* @[declare_core2foraws_crypto_strial_str_size] */

/**
 * @brief Initializes the ATECC608 Trust&GO driver on the I2C bus.
 * 
 * @warning This function cannot be used if ESP_TLS is used.
 * ESP_TLS calls it's own initialization function which will 
 * result in an abort.
 * @note The core2foraws_init() calls this function
 * when the hardware feature is enabled.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL  : Failed to initialize cryptoauthlib library
 */
/* @[declare_core2foraws_crypto_init] */
esp_err_t core2foraws_crypto_init( void );
/* @[declare_core2foraws_crypto_init] */

/**
 * @brief Retrieves the unique serial number from the ATECC608 as a string.
 * 
 * The serial number of the ATECC608 is stored as a uint8 with the 
 * length defined in the macro ATCA_SERIAL_NUM_SIZE.
 * 
 * @note It might take a little time to wake the secure element and
 * retrieve the serial number.
 * 
 * **Example:**
 * 
 * Create a variable named `serial_num` with enough dynamically allocated 
 * memory in external RAM to store the ATECC608 serial number.
 * @code{c}
 *  #include <stdint.h>
 *  #include <esp_log.h>
 *  #include "core2foraws.h" 
 * 
 *  static const char *TAG = "MAIN_ATECC608_EXAMPLE";
 * 
 *  void app_main( void )
 *  {
 *      ESP_LOGI( TAG, "\tStarting...");
 * 
 *      core2foraws_init();
 * 
 *      char *serial_num = ( char * )heap_caps_malloc( CRYPTO_SERIAL_STR_SIZE, MALLOC_CAP_SPIRAM );
 *      esp_err_t err = core2foraws_crypto_serial_get(serial_num);
 *      if ( err == ESP_OK )
 *      {
 *          ESP_LOGI(TAG, "\t%s", serial_num);
 *      }
 * 
 *      free( serial_num );
 *  }
 * @endcode
 * 
 * @param[out] serial_number A pointer to the unique serial number 
 * of the secure element.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL  : Failed to get the serial over the I2C bus
 */
/* @[declare_core2foraws_crypto_serial_get] */
esp_err_t core2foraws_crypto_serial_get( char *serial_number );
/* @[declare_core2foraws_crypto_serial_get] */

/**
 * @brief Gets the public key that's been paired with the 
 * pre-provisioned private key.
 * 
 * The maximum length of the public key is defined in the macro @ref
 * CRYPTO_PUB_KEY_SIZE.
 * 
 * @note It might take a little time to wake the secure element and
 * retrieve the public key.
 * 
 * **Example:**
 * 
 * Create a variable named `pub_key_str` with enough dynamically 
 * allocated memory in external RAM to store the public key.
 * @code{c}
 *  #include <stdint.h>
 *  #include <esp_log.h>
 *  #include "core2foraws.h" 
 * 
 *  static const char *TAG = "MAIN_ATECC608_EXAMPLE";
 * 
 *  void app_main( void )
 *  {
 *      ESP_LOGI( TAG, "\tStarting...");
 * 
 *      core2foraws_init();
 * 
 *      char *pub_key_str = ( char * )heap_caps_malloc( CRYPTO_PUB_KEY_SIZE, MALLOC_CAP_SPIRAM );
 *      esp_err_t err = core2foraws_crypto_pubkey_base64_get( pub_key_str );
 *      if ( err == ESP_OK )
 *      {
 *          ESP_LOGI( TAG, "\t%s", pub_key_str );
 *      }
 * 
 *      free( pub_key_str );
 *  }
 * @endcode
 * 
 * @param[out] public_key The preprovisioned public key.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL  : Failed to get the device public key over the I2C bus
 */
/* @[declare_core2foraws_crypto_pubkey_base64_get] */
esp_err_t core2foraws_crypto_pubkey_base64_get( char *public_key );
/* @[declare_core2foraws_crypto_pubkey_base64_get] */

/**
 * @brief Signs the provided message with the pre-provisioned 
 * private key using ECDSA.
 * 
 * The maximum length of the signature is defined in the macro @ref
 * CRYPTO_MAX_SIGNATURE_SIZE.
 *  
 * @note It might take a little time to wake the secure element and
 * generate a signature.
 * 
 * **Example:**
 * 
 * Create a variable named `sig` to store the ECDSA signature of a 
 * character hash stored in the variable `hash`. Then verify the 
 * signature.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <esp_log.h>
 *  #include "core2foraws.h" 
 * 
 *  static const char *TAG = "MAIN_ATECC608_EXAMPLE";
 * 
 *  static unsigned char hash[ 32 ] = 
 *  {
 *      0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
 *      0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
 *  };
 * 
 *  void app_main( void )
 *  {
 *      ESP_LOGI( TAG, "\tStarting...");
 * 
 *      core2foraws_init();
 * 
 *      unsigned char sig[ CRYPTO_MAX_SIGNATURE_SIZE ];
 *      size_t sig_len = 0;
 *      esp_err_t err = core2foraws_crypto_sha256_sign( hash, sig, &sig_len );
 *      if ( err == ESP_OK)
 *      {
 *          ESP_LOGI( TAG, "\tSignature: %u", ( unsigned int )sig );
 *      }
 *      
 *      bool is_verified = false;
 *      err = core2foraws_crypto_sha256_verify( hash, sig, sig_len, &is_verified );
 *      if ( err == ESP_OK )
 *      {
 *          ESP_LOGI( TAG, "\tVerified: %s", is_verified ? "true" : "false" );
 *      }
 *  }
 * @endcode
 * 
 * @param[in] message Pointer to the message to sign.
 * @param[out] signature The ECDSA signature.
 * @param[out] signature_length The length of the signature.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL  : Failed to get sign using the secure element
 */
/* @[declare_core2foraws_crypto_sha256_sign] */
esp_err_t core2foraws_crypto_sha256_sign( const unsigned char *message, uint8_t *signature, size_t *signature_length );
/* @[declare_core2foraws_crypto_sha256_sign] */

/**
 * @brief Verifies if the ECDSA signature is valid for the provided 
 * message.
 * 
 * The maximum length of the signature is defined in the macro @ref
 * CRYPTO_MAX_SIGNATURE_SIZE.
 *  
 * @note It might take a little time to wake the secure element and
 * generate a signature.
 * 
 * **Example:**
 * 
 * Create a variable named `sig` to store the ECDSA signature of a 
 * character hash stored in the variable `hash`. Then verify the 
 * signature.
 * @code{c}
 *  #include <stdint.h>
 *  #include <stdbool.h>
 *  #include <esp_log.h>
 *  #include "core2foraws.h" 
 * 
 *  static const char *TAG = "MAIN_ATECC608_EXAMPLE";
 * 
 *  static unsigned char hash[ 32 ] = 
 *  {
 *      0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
 *      0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
 *  };
 * 
 *  void app_main( void )
 *  {
 *      ESP_LOGI( TAG, "\tStarting...");
 * 
 *      core2foraws_init();
 * 
 *      unsigned char sig[ CRYPTO_MAX_SIGNATURE_SIZE ];
 *      size_t sig_len = 0;
 *      esp_err_t err = core2foraws_crypto_sha256_sign( hash, sig, &sig_len );
 *      if ( err == ESP_OK)
 *      {
 *          ESP_LOGI( TAG, "\tSignature: %u", ( unsigned int )sig );
 *      }
 *      
 *      bool is_verified = false;
 *      err = core2foraws_crypto_sha256_verify( hash, sig, sig_len, &is_verified );
 *      if ( err == ESP_OK )
 *      {
 *          ESP_LOGI( TAG, "\tVerified: %s", is_verified ? "true" : "false" );
 *      }
 *  }
 * @endcode
 * 
 * @param[in] message The message to sign.
 * @param[in] signature The ECDSA signature.
 * @param[in] signature_length The length of the signature.
 * @param[out] verified The boolean if the signature validates the message.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL  : Failed to get verify certificate using the secure element
 */
/* @[declare_core2foraws_crypto_sha256_verify] */
esp_err_t core2foraws_crypto_sha256_verify( const unsigned char *message, const uint8_t *signature, const size_t signature_length, bool *verified );
/* @[declare_core2foraws_crypto_sha256_verify] */

#ifdef __cplusplus
}
#endif
#endif
