/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_crypto.h
 * @brief Core2 for AWS IoT Kit cryptographic hardware driver APIs
 */

#ifndef _CORE2FORAWS_CRYPTO_H_
#define _CORE2FORAWS_CRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <esp_err.h>
#include <cryptoauthlib.h>

/**
 * @brief The size of the public key string
 */
/* @[declare_core2foraws_crypto_pub_key_size] */
#define CRYPTO_PUB_KEY_SIZE 128U
/* @[declare_core2foraws_crypto_pub_key_size] */

/**
 * @brief The ATECC608 raw P-256 signature size.
 *
 * @note ATECC608 P-256 signatures are returned as a raw 64-byte R || S value.
 */
/* @[declare_core2foraws_crypto_max_signature_size] */
#define CRYPTO_MAX_SIGNATURE_SIZE ATCA_SIG_SIZE
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
 * @note This driver uses CryptoAuthLib directly and does not install mbedTLS
 * ALT hooks, so it can be initialized alongside ESP-TLS.
 * @note core2foraws_init() calls this function when the hardware feature is
 * enabled.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
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
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK              : Success
 *  - ESP_ERR_INVALID_ARG : @p serial_number is NULL
 *  - ESP_FAIL            : Failed to get the serial over the I2C bus
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
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK              : Success
 *  - ESP_ERR_INVALID_ARG : @p public_key is NULL
 *  - ESP_FAIL            : Failed to get or encode the device public key
 */
/* @[declare_core2foraws_crypto_pubkey_base64_get] */
esp_err_t core2foraws_crypto_pubkey_base64_get( char *public_key );
/* @[declare_core2foraws_crypto_pubkey_base64_get] */

/**
 * @brief Signs a SHA-256 digest with the pre-provisioned private key using
 * ECDSA.
 * 
 * @p message must point to a 32-byte SHA-256 digest. On success, @p signature
 * contains a raw, fixed-size P-256 `R || S` value and @p signature_length is
 * set to @ref CRYPTO_MAX_SIGNATURE_SIZE. After argument validation, a signing
 * failure leaves @p signature_length set to zero.
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
 *          ESP_LOGI( TAG, "\tSignature length: %u", ( unsigned int )sig_len );
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
 * @param[in] message Pointer to the 32-byte SHA-256 digest to sign.
 * @param[out] signature Buffer of at least @ref CRYPTO_MAX_SIGNATURE_SIZE
 * bytes for the raw ECDSA signature.
 * @param[out] signature_length Raw signature length on success, or zero if the
 * signing operation fails after argument validation.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK              : Success
 *  - ESP_ERR_INVALID_ARG : An input or output pointer is NULL
 *  - ESP_FAIL            : The secure element failed to sign the digest
 */
/* @[declare_core2foraws_crypto_sha256_sign] */
esp_err_t core2foraws_crypto_sha256_sign( const unsigned char *message, uint8_t *signature, size_t *signature_length );
/* @[declare_core2foraws_crypto_sha256_sign] */

/**
 * @brief Verifies a raw ECDSA signature for a SHA-256 digest.
 * 
 * @p message must point to a 32-byte SHA-256 digest. @p signature must contain
 * exactly @ref CRYPTO_MAX_SIGNATURE_SIZE bytes in raw `R || S` format. A
 * completed verification returns `ESP_OK`; inspect @p verified to distinguish
 * a valid signature from an invalid one.
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
 *          ESP_LOGI( TAG, "\tSignature length: %u", ( unsigned int )sig_len );
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
 * @param[in] message Pointer to the 32-byte SHA-256 digest to verify.
 * @param[in] signature Raw `R || S` ECDSA signature.
 * @param[in] signature_length Signature length; must equal
 * @ref CRYPTO_MAX_SIGNATURE_SIZE.
 * @param[out] verified Set to true only when the signature validates the
 * digest.
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK               : Verification completed; inspect @p verified
 *  - ESP_ERR_INVALID_ARG  : An input or output pointer is NULL
 *  - ESP_ERR_INVALID_SIZE : @p signature_length is not the required raw size
 *  - ESP_FAIL             : The secure element could not perform verification
 */
/* @[declare_core2foraws_crypto_sha256_verify] */
esp_err_t core2foraws_crypto_sha256_verify( const unsigned char *message, const uint8_t *signature, const size_t signature_length, bool *verified );
/* @[declare_core2foraws_crypto_sha256_verify] */

#ifdef __cplusplus
}
#endif
#endif
