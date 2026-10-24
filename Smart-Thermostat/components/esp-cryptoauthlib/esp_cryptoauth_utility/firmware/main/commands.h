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
#ifndef __SECURE_ELEMENT_COMMANDS_H_
#define __SECURE_ELEMENT_COMMANDS_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef enum device_status {
    BEGIN = 0,
    ATECC_INIT_FAIL,
    ATECC_INIT_SUCCESS,
    KEY_PAIR_GEN_FAIL,
    KEY_PAIR_GEN_SUCCESS,
    CSR_GEN_FAIL,
    CSR_GEN_SUCCESS,
    PUBKEY_GEN_FAIL,
    PUBKEY_GEN_SUCCESS,
    GET_CERT_DEF_SUCCESS,
    GET_CERT_DEF_FAIL,
    PROGRAM_CERT_BEGIN,
    PROGRAM_CERT_FAIL,
    PROGRAM_CERT_SUCCESS,
    TNGTLS_ROOT_CERT_FAIL,
    TNGTLS_ROOT_CERT_SUCCESS,
    TNGTLS_SIGNER_CERT_FAIL,
    TNGTLS_SIGNER_CERT_SUCCESS,
    TNGTLS_DEVICE_CERT_FAIL,
    TNGTLS_DEVICE_CERT_SUCCESS,
} device_status_t;

esp_err_t register_command_handler();
#ifdef __cplusplus
}
#endif

#endif /* ! __SECURE_ELEMENT_COMMANDS_H_ */
