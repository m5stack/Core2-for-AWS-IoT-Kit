/*
 * Copyright 2021 Espressif Systems (Shanghai) CO LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

/*
 * Certificate Type
 */
typedef enum cert_type {
    CERT_TYPE_DEVICE = 0,
    CERT_TYPE_SIGNER,
} cert_type_t;

esp_err_t init_atecc608a(char *device_type,uint8_t i2c_sda_pin, uint8_t i2c_scl_pin, int *err_code);
esp_err_t atecc_print_info(uint8_t *serial_no, int *err_ret);
esp_err_t atecc_keygen(int key_slot, unsigned char *pub_key_buf, int pub_key_buf_len, int *err_code);
esp_err_t atecc_csr_gen(unsigned char *csr_buf, size_t csr_buf_len, int *err_code);
esp_err_t get_cert_def(unsigned char *cert_def_array, size_t data_len, cert_type_t cert_type);
esp_err_t atecc_input_cert(unsigned char *cert_buf, size_t cert_len, cert_type_t cert_type, bool lock, int *err_code, uint32_t expected_crc);
esp_err_t atecc_gen_pubkey(int key_slot, unsigned char *pubkey_buf, int pub_key_buf_len, int *err_code);
esp_err_t atecc_get_tngtls_root_cert(unsigned char *cert_buf, size_t *cert_len, int *err_code);
esp_err_t atecc_get_tngtls_signer_cert(unsigned char *cert_buf, size_t *cert_len, int *err_code);
esp_err_t atecc_get_tngtls_device_cert(unsigned char *cert_buf, size_t *cert_len, int *err_code);

#ifdef __cplusplus
}
#endif
