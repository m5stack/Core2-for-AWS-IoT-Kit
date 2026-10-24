#pragma once
/* Cryptoauthlib Configuration File */
#ifndef ATCA_CONFIG_H
#define ATCA_CONFIG_H

/* Include HALS */
#define ATCA_HAL_I2C
#define ATCA_USE_RTOS_TIMER 1
#define ATCA_MBEDTLS
/* cryptoauthlib 3.7.x accesses mbedTLS structs through MBEDTLS_PRIVATE(), which only mbedTLS 3.x (ESP-IDF 5.x) defines */
#include "mbedtls/version.h"
#if (MBEDTLS_VERSION_NUMBER < 0x03000000) && !defined(MBEDTLS_PRIVATE)
#define MBEDTLS_PRIVATE(member) member
#endif

/* mbedTLS < 2.24 (ESP-IDF 4.2) lacks two APIs used by cryptoauthlib 3.7.x */
#if (MBEDTLS_VERSION_NUMBER < 0x02180000)
#include <stddef.h>
#include "mbedtls/asn1.h"
#include "mbedtls/platform.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/x509_crt.h"

static inline void mbedtls_asn1_sequence_free(mbedtls_asn1_sequence *seq)
{
    while (seq != NULL) {
        mbedtls_asn1_sequence *next = seq->next;
        mbedtls_platform_zeroize(seq, sizeof(*seq));
        mbedtls_free(seq);
        seq = next;
    }
}

/* mbedtls_x509_crt_parse_der() always copies the buffer, matching make_copy == 1 */
#define mbedtls_x509_crt_parse_der_with_ext_cb(chain, buf, buflen, make_copy, cb, p_ctx) \
    mbedtls_x509_crt_parse_der(chain, buf, buflen)
#endif
//#define ATCA_CA_SUPPORT
/* Included device support */
#define ATCA_ATECC608_SUPPORT

#define ATCA_TNG_LEGACY_SUPPORT
#define ATCA_TFLEX_SUPPORT
#define ATCA_TNGTLS_SUPPORT
#define ATCA_TNGLORA_SUPPORT

/* Required by cryptoauthlib 3.7.x: the TNG headers evaluate these before the defaults are applied */
#define ATCAC_CERT_ADD_EN 1
#define ATCACERT_COMPCERT_EN 1

/* \brief How long to wait after an initial wake failure for the POST to
 *         complete.
 * If Power-on self test (POST) is enabled, the self test will run on waking
 * from sleep or during power-on, which delays the wake reply.
 */
#ifndef ATCA_POST_DELAY_MSEC
#define ATCA_POST_DELAY_MSEC 25
#endif

#define ATCA_PLATFORM_MALLOC malloc
#define ATCA_PLATFORM_FREE free

#define hal_delay_ms atca_delay_ms
#define ATCA_PRINTF
#endif // ATCA_CONFIG_H
