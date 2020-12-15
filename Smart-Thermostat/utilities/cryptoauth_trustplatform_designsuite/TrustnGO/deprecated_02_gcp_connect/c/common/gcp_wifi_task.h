/**
 * \file
 * \brief GCP WIFI Functions
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

#ifndef GCP_WIFI_TASK_H
#define GCP_WIFI_TASK_H

#include <stdbool.h>

#include "gcp_status.h"
#include "bsp/include/nm_bsp.h"
//#include "oled1.h"
#include "socket/include/socket.h"
#include "cryptoauthlib.h"

#define min(x, y) ((x) > (y) ? (y) : (x))

extern ATCAIfaceCfg cfg_ateccx08a_gcp;

enum wifi_status
{
    WIFI_STATUS_UNKNOWN          = 0,
    WIFI_STATUS_MESSAGE_RECEIVED = 1,    
    WIFI_STATUS_MESSAGE_SENT     = 2,
    WIFI_STATUS_TIMEOUT          = 3,
    WIFI_STATUS_ERROR            = 4    
};

struct socket_connection
{
    SOCKET socket;
    uint32 address;
    uint16 port;
};

struct demo_button_state
{
	uint8_t  button_1;
	uint8_t  button_2;
	uint8_t  button_3;
};


void gcp_wifi_set_state(enum gcp_iot_state state);
enum gcp_iot_state gcp_wifi_get_state(void);

int gcp_wifi_read_data(uint8_t *read_buffer, uint32_t read_length, 
                       uint32_t timeout_ms);
int gcp_wifi_send_data(uint8_t *send_buffer, uint32_t send_length, 
                       uint32_t timeout_ms);
                       
void gcp_wifi_publish_message(struct demo_button_state state);

void gcp_wifi_task(void *params);

void client_timer_update(void);

#endif // GCP_WIFI_TASK_H