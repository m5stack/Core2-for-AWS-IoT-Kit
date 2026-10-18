/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int ATCA_STATUS;
#define ATCA_SUCCESS          0
#define ATCA_COMM_FAIL        0xF0
#define ATCA_SERIAL_NUM_SIZE  9
#define ATCA_PUB_KEY_SIZE     64
#define ATCA_SIG_SIZE         64

typedef struct
{
    struct
    {
        uint8_t address;
        uint32_t baud;
    } atcai2c;
} ATCAIfaceCfg;

static const ATCAIfaceCfg cfg_ateccx08a_i2c_default = {
    .atcai2c = { .address = 0xC0, .baud = 400000 }
};

ATCA_STATUS atcab_init(ATCAIfaceCfg *cfg);
ATCA_STATUS atcab_read_serial_number(uint8_t *serial_number);
ATCA_STATUS atcab_get_pubkey(uint16_t key_id, uint8_t *public_key);
ATCA_STATUS atcab_base64encode(const uint8_t *byte_array, size_t array_len,
                               char *encoded, size_t *encoded_len);
ATCA_STATUS atcab_sign(uint16_t key_id, const uint8_t *msg, uint8_t *signature);
ATCA_STATUS atcab_verify_extern(const uint8_t *message, const uint8_t *signature,
                                const uint8_t *public_key, bool *is_verified);
