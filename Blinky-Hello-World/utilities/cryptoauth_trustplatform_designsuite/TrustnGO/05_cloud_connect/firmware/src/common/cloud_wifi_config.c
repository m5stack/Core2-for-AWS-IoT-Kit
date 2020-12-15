/**
 * \file
 * \brief  Example Configuration
 *
 * \copyright (c) 2017 Microchip Technology Inc. and its subsidiaries.
 *            You may use this software and any derivatives exclusively with
 *            Microchip products.
 *
 * \page License
 *
 * (c) 2017 Microchip Technology Inc. and its subsidiaries. You may use this
 * software and any derivatives exclusively with Microchip products.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIPS TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
 * TERMS.
 */

#include "cloud_wifi_ecc_process.h"
#include "cloud_wifi_config.h"
#include "jwt/atca_jwt.h"
#include <stdio.h>
#include "definitions.h"
#include "cloud_wifi_ecc_process.h"

static char config_thing_id[130];

int config_set_thing_id()
{
    uint8_t serial_num[9];
    size_t hex_size;

    hex_size = sizeof(config_thing_id) - 1;
    ATCA_STATUS rv;

    rv = atcab_read_serial_number(serial_num);
    if (ATCA_SUCCESS != rv)
    {
        return rv;
    }

#if defined(CLOUD_CONFIG_GCP)
    config_thing_id[0] = 'd';
    rv = atcab_bin2hex_(serial_num, sizeof(serial_num), &config_thing_id[1], &hex_size, false, false, true);
#elif defined(CLOUD_CONFIG_AWS)
    rv = atcab_bin2hex_(serial_num, sizeof(serial_num), config_thing_id, &hex_size, false, false, true);
#elif defined(CLOUD_CONFIG_AZURE)
    rv = atcab_bin2hex_(serial_num, sizeof(serial_num), config_thing_id, &hex_size, false, false, true);
#endif

    if (ATCA_SUCCESS != rv)
    {
        return rv;
    }

    return ATCA_SUCCESS;
}

/** \brief Populate the buffer with the client id */
int config_get_client_id(char* buf, size_t buflen)
{
    if (buf && buflen)
    {
        int rv;

#if defined(CLOUD_CONFIG_GCP)
        rv = snprintf(buf, buflen, "projects/%s/locations/%s/registries/%s/devices/%s",
                      config_gcp_project_id, config_gcp_region_id, config_gcp_registry_id, config_thing_id);
#elif defined(CLOUD_CONFIG_AWS)
        rv = snprintf(buf, buflen, "%s", config_thing_id);
#elif defined(CLOUD_CONFIG_AZURE)
        rv = snprintf(buf, buflen, "sn%s", config_thing_id);
#endif

        if (0 < rv && rv < buflen)
        {
            buf[rv] = 0;
            return 0;
        }
    }
    return -1;
}

/* Populate the buffer with the username */
int config_get_client_username(char* buf, size_t buflen)
{
    if (buf && buflen)
    {
#ifdef CLOUD_CONFIG_AZURE
        int rv = snprintf(buf, buflen, "%s/sn%s", CLOUD_ENDPOINT, config_thing_id);
#else
        int rv = snprintf(buf, buflen, "unused");
#endif
        if (0 < rv && rv < buflen)
        {
            buf[rv] = 0;
            return 0;
        }
    }
    return -1;
}

/* Populate the buffer with the user's password */
int config_get_client_password(char* buf, size_t buflen)
{
    int rv = -1;

#ifdef CLOUD_CONFIG_GCP

    if (buf && buflen)
    {
        atca_jwt_t jwt;

        uint32_t ts = RTC_Timer32CounterGet();

        /* Build the JWT */
        rv = atca_jwt_init(&jwt, buf, buflen);
        if (ATCA_SUCCESS != rv)
        {
            return rv;
        }

        if (ATCA_SUCCESS != (rv = atca_jwt_add_claim_numeric(&jwt, "iat", ts)))
        {
            return rv;
        }

        if (ATCA_SUCCESS != (rv = atca_jwt_add_claim_numeric(&jwt, "exp", ts + 86400)))
        {
            return rv;
        }

        if (ATCA_SUCCESS != (rv = atca_jwt_add_claim_string(&jwt, "aud", config_gcp_project_id)))
        {
            return rv;
        }

        rv = atca_jwt_finalize(&jwt, 0);

    }
#else
    if (buf && buflen)
    {
        rv = snprintf(buf, buflen, "unused");

        if (0 < rv && rv < buflen)
        {
            buf[rv] = 0;
            return 0;
        }
    }
#endif

    return rv;

}

/* Get the topic id  where the client will be publishing messages */
int config_get_client_pub_topic(char* buf, size_t buflen)
{
    int rv;

    if (buf && buflen)
    {
#if defined(CLOUD_CONFIG_GCP)
        rv = snprintf(buf, buflen, "/devices/%s/events", config_thing_id);
#elif defined(CLOUD_CONFIG_AWS)
        rv = snprintf(buf, buflen, "$aws/things/%s/shadow/update", config_thing_id);
#elif defined(CLOUD_CONFIG_AZURE)
        rv = snprintf(buf, buflen, "devices/sn%s/messages/events/", config_thing_id);
#endif

        if (0 < rv && rv < buflen)
        {
            buf[rv] = 0;
            return 0;
        }
    }
    return -1;
}

/* Get the topic id  where the client will be publishing messages */
int config_get_client_sub_topic(char* buf, size_t buflen)
{
    int rv;

    if (buf && buflen)
    {
#if defined(CLOUD_CONFIG_GCP)
        rv = snprintf(buf, buflen, "/devices/%s/commands/#", config_thing_id);
#elif defined(CLOUD_CONFIG_AWS)
        rv = snprintf(buf, buflen, "$aws/things/%s/shadow/update/delta", config_thing_id);
#elif defined(CLOUD_CONFIG_AZURE)
        rv = snprintf(buf, buflen, "devices/sn%s/messages/devicebound/#", config_thing_id);
#endif

        if (0 < rv && rv < buflen)
        {
            buf[rv] = 0;
            return 0;
        }
    }
    return -1;
}