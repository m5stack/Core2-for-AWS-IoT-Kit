/**
 * \file
 * \brief Cloud status functions
 *
 * \copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
 * SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
 * OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
 * FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
 * LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
 * THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
 * THIS SOFTWARE.
 */

#ifndef CLOUD_STATUS_H
#define CLOUD_STATUS_H

#include <stdint.h>

// Define
#define CLOUD_STATUS_MESSAGE_SIZE  (64)

/**
 * \brief The Cloud IOT state
 *
 * \note  The state machines depend on the order of the states.  Please do not
 *        change the order of the states without reviewing the state machines
 */
enum cloud_iot_state
{
    // Provisioning States
    CLOUD_STATE_UNKNOWN                   = 0,
    CLOUD_STATE_ATECCx08A_DETECT          = 1,
    CLOUD_STATE_ATECCx08A_PRECONFIGURE    = 2,
    CLOUD_STATE_ATECCx08A_INIT            = 3,
    CLOUD_STATE_ATECCx08A_CONFIGURE       = 4,
    CLOUD_STATE_ATECCx08A_PROVISIONED     = 5,
    CLOUD_STATE_ATECCx08A_PROVISION_RESET = 6,

    // CLOUD and WIFI States
    CLOUD_STATE_WINC1500_INIT             = 7,
    CLOUD_STATE_WIFI_CONFIGURE            = 8,
    CLOUD_STATE_WIFI_DISCONNECT           = 9,
    CLOUD_STATE_CLOUD_CONNECT               = 10,
    CLOUD_STATE_CLOUD_CONNECTING            = 11,
    CLOUD_STATE_CLOUD_CONNECTED             = 12,
    CLOUD_STATE_CLOUD_SUBSCRIPTION          = 13,
    CLOUD_STATE_CLOUD_REPORTING             = 14,
    CLOUD_STATE_CLOUD_DISCONNECT            = 15
};

/**
 * \brief The Cloud IoT status code
 */
enum cloud_iot_code
{
    CLOUD_STATUS_SUCCESS                    = 0,
    CLOUD_STATUS_GENERAL_ERROR              = 1,
    CLOUD_STATUS_UNKNOWN_COMMAND            = 2,
    CLOUD_STATUS_BAD_PARAMETER              = 3,

    CLOUD_STATUS_ATECCx08A_FAILURE          = 256,
    CLOUD_STATUS_ATECCx08A_UNPROVISIONED    = 257,
    CLOUD_STATUS_ATECCx08A_INIT_FAILURE     = 258,
    CLOUD_STATUS_ATECCx08A_CONFIG_FAILURE   = 259,
    CLOUD_STATUS_ATECCx08A_COMM_FAILURE     = 260,
    CLOUD_STATUS_WINC1500_FAILURE           = 261,
    CLOUD_STATUS_WINC1500_INIT_FAILURE      = 262,
    CLOUD_STATUS_WINC1500_COMM_FAILURE      = 263,
    CLOUD_STATUS_WINC1500_TIMEOUT_FAILURE   = 264,

    CLOUD_STATUS_WIFI_FAILURE               = 512,
    CLOUD_STATUS_WIFI_SSID_NOT_FOUND        = 513,
    CLOUD_STATUS_WIFI_CONNECT_FAILURE       = 514,
    CLOUD_STATUS_WIFI_DISCONNECT_FAILURE    = 515,
    CLOUD_STATUS_WIFI_AUTHENICATION_FAILURE = 516,
    CLOUD_STATUS_DHCP_FAILURE               = 517,
    CLOUD_STATUS_DNS_RESOLUTION_FAILURE     = 518,
    CLOUD_STATUS_NTP_FAILURE                = 519,

    CLOUD_STATUS_CLOUD_FAILURE                = 1024,
    CLOUD_STATUS_CLOUD_COMM_FAILURE           = 1025,
    CLOUD_STATUS_CLOUD_SUBSCRIPTION_FAILURE   = 1026,
    CLOUD_STATUS_CLOUD_REPORT_FAILURE         = 1027
};

/**
 * \brief The Cloud IoT status structure
 */
struct cloud_iot_status
{

    uint32_t cloud_state;
    uint32_t cloud_status;
    char     cloud_message[CLOUD_STATUS_MESSAGE_SIZE];
};

void cloud_iot_clear_status(void);

struct cloud_iot_status * cloud_iot_get_status(void);
void cloud_iot_set_status(uint32_t state, uint32_t status, const char *message);

#endif // CLOUD_STATUS_H