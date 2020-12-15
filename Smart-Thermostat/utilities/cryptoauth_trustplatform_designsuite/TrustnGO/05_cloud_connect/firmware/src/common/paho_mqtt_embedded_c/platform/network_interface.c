/**
 *
 * \file
 *
 * \brief Platform network interface
 *
 * Copyright (c) 2014-2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include "MQTTReturnCodes.h"
#include "network_interface.h"
#include "cloud_wifi_task.h"

/**
 * \brief Reads data from the WINC1500 module.
 *
 * \param network[in]               The Eclipse Paho MQTT network information
 * \param read_buffer[in]           The buffer
 * \param length[in]                The buffer length
 * \param timeout_ms[in]            The timeout
 *
 * \return    The MQTT status
 */
int mqtt_packet_read(Network *network, unsigned char *read_buffer, int length, int timeout_ms)
 {
    return cloud_wifi_read_data(read_buffer, length, timeout_ms);
 }

/**
 * \brief Writes data to the WINC1500 module.
 *
 * \param network[in]               The Eclipse Paho MQTT network information
 * \param send_buffer[in]           The buffer
 * \param length[in]                The buffer length
 * \param timeout_ms[in]            The timeout
 *
 * \return    The MQTT status
 */
int mqtt_packet_write(Network *network, unsigned char *send_buffer, int length, int timeout_ms)
{
    return cloud_wifi_send_data(send_buffer, length, timeout_ms);
}
