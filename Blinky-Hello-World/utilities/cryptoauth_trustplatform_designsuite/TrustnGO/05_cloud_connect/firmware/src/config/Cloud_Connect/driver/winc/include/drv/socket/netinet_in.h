/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    netinet_in.h

  Summary:

  Description:
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

#ifndef _NETINET_IN_H
#define _NETINET_IN_H

#include <stdint.h>
#include <sys/types.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
    extern "C" {
#endif
// DOM-IGNORE-END

typedef uint32_t in_addr_t;

struct in_addr {
    /*!<
        Network Byte Order representation of the IPv4 address. For example,
        the address "192.168.0.10" is represented as 0x0A00A8C0.
    */
    in_addr_t s_addr;
};

struct sockaddr_in{
    uint16_t        sin_family;
    /*!<
        Specifies the address family(AF).
        Members of AF_INET address family are IPv4 addresses.
        Hence,the only supported value for this is AF_INET.
    */
    uint16_t        sin_port;
    /*!<
        Port number of the socket.
        Network sockets are identified by a pair of IP addresses and port number.
        Must be set in the Network Byte Order format , @ref _htons (e.g. _htons(80)).
        Can NOT have zero value.
    */
    struct in_addr  sin_addr;
    /*!<
        IP Address of the socket.
        The IP address is of type @ref in_addr structure.
        Can be set to "0" to accept any IP address for server operation.
    */
    uint8_t         sin_zero[8];
    /*!<
        Padding to make structure the same size as @ref sockaddr.
    */
};

const char *inet_ntop(int af, const void *src, char *dst, size_t size);
in_addr_t inet_addr(const char *cp);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _NETINET_IN_H */
