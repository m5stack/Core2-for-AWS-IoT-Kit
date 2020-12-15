/**
 * \file
 * \brief AWS status functions
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

#ifndef AWS_STATUS_H
#define AWS_STATUS_H

#include <stdint.h>

// Define
#define AWS_STATUS_MESSAGE_SIZE  (64)

/**
 * \brief The AWS IoT Zero Touch Demo state
 *
 * \note  The state machines depend on the order of the states.  Please do not
 *        change the order of the states without reviewing the state machines
 */
enum aws_iot_state
{
    // Provisioning States
    AWS_STATE_UNKNOWN                   = 0,
    AWS_STATE_ATECCx08A_DETECT          = 1,
    AWS_STATE_ATECCx08A_PRECONFIGURE    = 2,
    AWS_STATE_ATECCx08A_INIT            = 3,
    AWS_STATE_ATECCx08A_CONFIGURE       = 4,
    AWS_STATE_ATECCx08A_PROVISIONED     = 5,
    AWS_STATE_ATECCx08A_PROVISION_RESET = 6,
    
    // AWS and WIFI States
    AWS_STATE_WINC1500_INIT             = 7,
    AWS_STATE_WIFI_CONFIGURE            = 8,
    AWS_STATE_WIFI_DISCONNECT           = 9,
    AWS_STATE_AWS_CONNECT               = 10,
    AWS_STATE_AWS_CONNECTING            = 11,
    AWS_STATE_AWS_CONNECTED             = 12,
    AWS_STATE_AWS_SUBSCRIPTION          = 13,
    AWS_STATE_AWS_REPORTING             = 14,
    AWS_STATE_AWS_DISCONNECT            = 15
};

/**
 * \brief The AWS IoT Zero Touch Demo status code
 */
enum aws_iot_code
{
    AWS_STATUS_SUCCESS                    = 0,
    AWS_STATUS_GENERAL_ERROR              = 1,
    AWS_STATUS_UNKNOWN_COMMAND            = 2,
    AWS_STATUS_BAD_PARAMETER              = 3,
    
    AWS_STATUS_ATECCx08A_FAILURE          = 256,
    AWS_STATUS_ATECCx08A_UNPROVISIONED    = 257,
    AWS_STATUS_ATECCx08A_INIT_FAILURE     = 258,
    AWS_STATUS_ATECCx08A_CONFIG_FAILURE   = 259,
    AWS_STATUS_ATECCx08A_COMM_FAILURE     = 260,    
    AWS_STATUS_WINC1500_FAILURE           = 261,
    AWS_STATUS_WINC1500_INIT_FAILURE      = 262,
    AWS_STATUS_WINC1500_COMM_FAILURE      = 263,
    AWS_STATUS_WINC1500_TIMEOUT_FAILURE   = 264,
    
    AWS_STATUS_WIFI_FAILURE               = 512,
    AWS_STATUS_WIFI_SSID_NOT_FOUND        = 513,
    AWS_STATUS_WIFI_CONNECT_FAILURE       = 514,
    AWS_STATUS_WIFI_DISCONNECT_FAILURE    = 515,
    AWS_STATUS_WIFI_AUTHENICATION_FAILURE = 516,
    AWS_STATUS_DHCP_FAILURE               = 517,
    AWS_STATUS_DNS_RESOLUTION_FAILURE     = 518,    
    AWS_STATUS_NTP_FAILURE                = 519,
    
    AWS_STATUS_AWS_FAILURE                = 1024,
    AWS_STATUS_AWS_COMM_FAILURE           = 1025,
    AWS_STATUS_AWS_SUBSCRIPTION_FAILURE   = 1026,
    AWS_STATUS_AWS_REPORT_FAILURE         = 1027
};

/**
 * \brief The AWS IoT Zero Touch Demo status structure
 */
struct aws_iot_status {
    
    uint32_t aws_state;
    uint32_t aws_status;
    char     aws_message[AWS_STATUS_MESSAGE_SIZE];
};

void aws_iot_clear_status(void);

struct aws_iot_status * aws_iot_get_status(void);
void aws_iot_set_status(uint32_t state, uint32_t status, const char *message);

#endif // AWS_STATUS_H