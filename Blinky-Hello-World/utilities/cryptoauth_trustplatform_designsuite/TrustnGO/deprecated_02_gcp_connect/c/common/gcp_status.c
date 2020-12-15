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

#include <string.h>

#include "gcp_status.h"

// Global variables
static struct gcp_iot_status g_gcp_iot_status;

/**
 * \brief Clears the global GCP IoT status information
 */
void gcp_iot_clear_status(void)
{
    g_gcp_iot_status.gcp_state  = GCP_STATE_UNKNOWN;
    g_gcp_iot_status.gcp_status = GCP_STATUS_SUCCESS;

    memset(&g_gcp_iot_status.gcp_message[0], 0, sizeof(g_gcp_iot_status.gcp_message));
}

/**
 * \brief Gets the global GCP IoT status information
 *
 * \note Do not free the pointer returned from this function
 */
struct gcp_iot_status * gcp_iot_get_status(void)
{
    return &g_gcp_iot_status;
}

/**
 * \brief Sets the global GCP IoT status information
 *
 * param[in] state                  The GCP IoT error state
 * param[in] status                 The GCP IoT error status
 * param[in] message                The GCP IoT error message
 */
void gcp_iot_set_status(uint32_t state, uint32_t status, const char *message)
{
    g_gcp_iot_status.gcp_state  = state;
    g_gcp_iot_status.gcp_status = status;

    memset(&g_gcp_iot_status.gcp_message[0], 0, 
           sizeof(g_gcp_iot_status.gcp_message));
    strncpy(&g_gcp_iot_status.gcp_message[0], &message[0], 
            sizeof(g_gcp_iot_status.gcp_message));
}
