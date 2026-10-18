/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "cryptoauthlib.h"
#include "../../lib/crypto/core2foraws_crypto.c"

static ATCA_STATUS init_status, serial_status, pubkey_status, sign_status, verify_status;
static unsigned int init_calls;
static bool lock_timeout;

SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *storage) { return storage; }
BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t timeout)
{
    assert(mutex != NULL && timeout == 101);
    if (lock_timeout) return 0;
    mutex->depth++;
    return pdTRUE;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex)
{
    assert(mutex != NULL && mutex->depth > 0);
    mutex->depth--;
    return pdTRUE;
}
void vTaskDelay(TickType_t ticks) { assert(ticks == 1); }
esp_err_t core2foraws_common_error(esp_err_t error)
{
    return error == ATCA_SUCCESS ? ESP_OK : ESP_FAIL;
}

ATCA_STATUS atcab_init(ATCAIfaceCfg *cfg)
{
    assert(_crypto_mutex != NULL && _crypto_mutex->depth == 1);
    assert(cfg->atcai2c.address == 0x6A && cfg->atcai2c.baud == 100000);
    init_calls++;
    return init_status;
}
ATCA_STATUS atcab_read_serial_number(uint8_t *serial_number)
{
    assert(_crypto_mutex != NULL && _crypto_mutex->depth == 1);
    if (serial_status != ATCA_SUCCESS) return serial_status;
    const uint8_t mock_sn[ATCA_SERIAL_NUM_SIZE] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xEE
    };
    memcpy(serial_number, mock_sn, sizeof(mock_sn));
    return ATCA_SUCCESS;
}
ATCA_STATUS atcab_get_pubkey(uint16_t key_id, uint8_t *public_key)
{
    assert(key_id == 0 && _crypto_mutex != NULL && _crypto_mutex->depth == 1);
    if (pubkey_status != ATCA_SUCCESS) return pubkey_status;
    for (size_t i = 0; i < ATCA_PUB_KEY_SIZE; i++) public_key[i] = (uint8_t)(i + 1);
    return ATCA_SUCCESS;
}
ATCA_STATUS atcab_base64encode(const uint8_t *byte_array, size_t array_len,
                               char *encoded, size_t *encoded_len)
{
    assert(byte_array != NULL && array_len == 91 && encoded != NULL && encoded_len != NULL);
    const char *mock_b64 = "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE";
    size_t len = strlen(mock_b64);
    memcpy(encoded, mock_b64, len);
    *encoded_len = len;
    return ATCA_SUCCESS;
}
ATCA_STATUS atcab_sign(uint16_t key_id, const uint8_t *msg, uint8_t *signature)
{
    assert(key_id == 0 && msg != NULL && signature != NULL);
    assert(_crypto_mutex != NULL && _crypto_mutex->depth == 1);
    if (sign_status != ATCA_SUCCESS) return sign_status;
    memset(signature, 0x5A, ATCA_SIG_SIZE);
    return ATCA_SUCCESS;
}
ATCA_STATUS atcab_verify_extern(const uint8_t *message, const uint8_t *signature,
                                const uint8_t *public_key, bool *is_verified)
{
    assert(message != NULL && signature != NULL && public_key != NULL && is_verified != NULL);
    assert(_crypto_mutex != NULL && _crypto_mutex->depth == 1);
    if (verify_status != ATCA_SUCCESS) return verify_status;
    *is_verified = (signature[0] == 0x5A);
    return ATCA_SUCCESS;
}

int main(void)
{
    char serial[CRYPTO_SERIAL_STR_SIZE] = { 0 };
    char pubkey[CRYPTO_PUB_KEY_SIZE] = { 0 };
    unsigned char digest[32] = { 0 };
    uint8_t sig[CRYPTO_MAX_SIGNATURE_SIZE] = { 0 };
    size_t sig_len = 99;
    bool verified = true;

    /* Argument validation before init */
    assert(core2foraws_crypto_serial_get(NULL) == ESP_ERR_INVALID_ARG);
    assert(core2foraws_crypto_pubkey_base64_get(NULL) == ESP_ERR_INVALID_ARG);
    assert(core2foraws_crypto_sha256_sign(NULL, sig, &sig_len) == ESP_ERR_INVALID_ARG);
    assert(core2foraws_crypto_sha256_verify(digest, sig, 63, &verified) == ESP_ERR_INVALID_SIZE && !verified);

    /* Uninitialized calls return ESP_ERR_INVALID_STATE */
    assert(core2foraws_crypto_serial_get(serial) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_crypto_pubkey_base64_get(pubkey) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_crypto_sha256_sign(digest, sig, &sig_len) == ESP_ERR_INVALID_STATE && sig_len == 0);
    assert(core2foraws_crypto_sha256_verify(digest, sig, CRYPTO_MAX_SIGNATURE_SIZE, &verified) == ESP_ERR_INVALID_STATE);

    /* Failed init leaves subsystem uninitialized for retry */
    init_status = ATCA_COMM_FAIL;
    assert(core2foraws_crypto_init() == ESP_FAIL && init_calls == 1);
    assert(core2foraws_crypto_serial_get(serial) == ESP_ERR_INVALID_STATE);

    /* Successful init is idempotent and pins 0x6A @ 100 kHz */
    init_status = ATCA_SUCCESS;
    assert(core2foraws_crypto_init() == ESP_OK && init_calls == 2);
    assert(core2foraws_crypto_init() == ESP_OK && init_calls == 2);

    /* Serial number formatting */
    assert(core2foraws_crypto_serial_get(serial) == ESP_OK);
    assert(strcmp(serial, "0123456789ABCDEFEE") == 0);

    /* Public key base64 formatting */
    assert(core2foraws_crypto_pubkey_base64_get(pubkey) == ESP_OK);
    assert(strcmp(pubkey, "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE") == 0);

    /* Sign and verify under lock */
    assert(core2foraws_crypto_sha256_sign(digest, sig, &sig_len) == ESP_OK && sig_len == 64);
    assert(core2foraws_crypto_sha256_verify(digest, sig, sig_len, &verified) == ESP_OK && verified);

    /* Lock timeout returns ESP_ERR_TIMEOUT */
    lock_timeout = true;
    assert(core2foraws_crypto_serial_get(serial) == ESP_ERR_TIMEOUT);
    lock_timeout = false;

    assert(_crypto_mutex->depth == 0);
    puts("Crypto initialization guard, address pinning, locking and sign/verify tests passed");
    return 0;
}
