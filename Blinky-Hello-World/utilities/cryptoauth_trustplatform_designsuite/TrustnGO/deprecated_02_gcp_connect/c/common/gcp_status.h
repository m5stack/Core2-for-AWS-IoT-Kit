/**
 * \file
 * \brief GCP status functions
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

#ifndef GCP_STATUS_H
#define GCP_STATUS_H

#include <stdint.h>

// Define
#define GCP_STATUS_MESSAGE_SIZE  (64)

/**
 * \brief The GCP IoT Demo 
 *
 * \note  The state machines depend on the order of the states.  Please do not
 *        change the order of the states without reviewing the state machines
 */
enum gcp_iot_state
{
    // Provisioning States
    GCP_STATE_UNKNOWN                   = 0,
    GCP_STATE_ATECCx08A_DETECT          = 1,
    GCP_STATE_ATECCx08A_PRECONFIGURE    = 2,
    GCP_STATE_ATECCx08A_INIT            = 3,
    GCP_STATE_ATECCx08A_CONFIGURE       = 4,
    GCP_STATE_ATECCx08A_PROVISIONED     = 5,
    GCP_STATE_ATECCx08A_PROVISION_RESET = 6,
    
    // GCP and WIFI States
    GCP_STATE_WINC1500_INIT             = 7,
    GCP_STATE_WIFI_CONFIGURE            = 8,
    GCP_STATE_WIFI_DISCONNECT           = 9,
    GCP_STATE_GCP_CONNECT               = 10,
    GCP_STATE_GCP_CONNECTING            = 11,
    GCP_STATE_GCP_CONNECTED             = 12,
    GCP_STATE_GCP_SUBSCRIPTION          = 13,
    GCP_STATE_GCP_REPORTING             = 14,
    GCP_STATE_GCP_DISCONNECT            = 15
};

/**
 * \brief The GCP IoT Demo status code
 */
enum gcp_iot_code
{
    GCP_STATUS_SUCCESS                    = 0,
    GCP_STATUS_GENERAL_ERROR              = 1,
    GCP_STATUS_UNKNOWN_COMMAND            = 2,
    GCP_STATUS_BAD_PARAMETER              = 3,
    
    GCP_STATUS_ATECCx08A_FAILURE          = 256,
    GCP_STATUS_ATECCx08A_UNPROVISIONED    = 257,
    GCP_STATUS_ATECCx08A_INIT_FAILURE     = 258,
    GCP_STATUS_ATECCx08A_CONFIG_FAILURE   = 259,
    GCP_STATUS_ATECCx08A_COMM_FAILURE     = 260,    
    GCP_STATUS_WINC1500_FAILURE           = 261,
    GCP_STATUS_WINC1500_INIT_FAILURE      = 262,
    GCP_STATUS_WINC1500_COMM_FAILURE      = 263,
    GCP_STATUS_WINC1500_TIMEOUT_FAILURE   = 264,
    
    GCP_STATUS_WIFI_FAILURE               = 512,
    GCP_STATUS_WIFI_SSID_NOT_FOUND        = 513,
    GCP_STATUS_WIFI_CONNECT_FAILURE       = 514,
    GCP_STATUS_WIFI_DISCONNECT_FAILURE    = 515,
    GCP_STATUS_WIFI_AUTHENICATION_FAILURE = 516,
    GCP_STATUS_DHCP_FAILURE               = 517,
    GCP_STATUS_DNS_RESOLUTION_FAILURE     = 518,    
    GCP_STATUS_NTP_FAILURE                = 519,
    
    GCP_STATUS_GCP_FAILURE                = 1024,
    GCP_STATUS_GCP_COMM_FAILURE           = 1025,
    GCP_STATUS_GCP_SUBSCRIPTION_FAILURE   = 1026,
    GCP_STATUS_GCP_REPORT_FAILURE         = 1027
};

/**
 * \brief The GCP IoT Demo status structure
 */
struct gcp_iot_status {
    
    uint32_t gcp_state;
    uint32_t gcp_status;
    char     gcp_message[GCP_STATUS_MESSAGE_SIZE];
};

void gcp_iot_clear_status(void);

struct gcp_iot_status * gcp_iot_get_status(void);
void gcp_iot_set_status(uint32_t state, uint32_t status, const char *message);

#endif // GCP_STATUS_H