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

#include <string.h>

#include "aws_status.h"

// Global variables
static struct aws_iot_status g_aws_iot_status;

/**
 * \brief Clears the global AWS IoT status information
 */
void aws_iot_clear_status(void)
{
    g_aws_iot_status.aws_state  = AWS_STATE_UNKNOWN;
    g_aws_iot_status.aws_status = AWS_STATUS_SUCCESS;

    memset(&g_aws_iot_status.aws_message[0], 0, sizeof(g_aws_iot_status.aws_message));
}

/**
 * \brief Gets the global AWS IoT status information
 *
 * \note Do not free the pointer returned from this function
 */
struct aws_iot_status * aws_iot_get_status(void)
{
    return &g_aws_iot_status;
}

/**
 * \brief Sets the global AWS IoT status information
 *
 * param[in] state                  The AWS IoT error state
 * param[in] status                 The AWS IoT error status
 * param[in] message                The AWS IoT error message
 */
void aws_iot_set_status(uint32_t state, uint32_t status, const char *message)
{
    g_aws_iot_status.aws_state  = state;
    g_aws_iot_status.aws_status = status;

    memset(&g_aws_iot_status.aws_message[0], 0, 
           sizeof(g_aws_iot_status.aws_message));
    strncpy(&g_aws_iot_status.aws_message[0], &message[0], 
            sizeof(g_aws_iot_status.aws_message));
}
