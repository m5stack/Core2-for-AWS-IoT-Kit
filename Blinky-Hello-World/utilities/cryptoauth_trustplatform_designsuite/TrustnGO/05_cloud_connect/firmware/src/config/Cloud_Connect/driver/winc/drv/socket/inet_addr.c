/*******************************************************************************
  WINC1500 Wireless Driver

  File Name:
    inet_addr.c

  Summary:
    Implementation of standard inet_addr function.

  Description:
    Implementation of standard inet_addr function.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
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
#include <stdint.h>
#include "socket.h"

in_addr_t inet_addr(const char *cp)
{
    uint8_t i,l;
    uint16_t t;
    uint32_t ip;
    char c;

    ip = 0;

    for (i=0; i<4; i++)
    {
        t = 0;
        ip >>= 8;

        // Count non-delimiter or terminator characters

        for (l=0; l<4; l++)
        {
            c = cp[l];

            if (('.' == c) || ('\0' == c))
            {
                break;
            }
        }

        // There must be 1 to 3 characters

        if ((0 == l) || (4 == l))
        {
            return 0;
        }

        c = *cp++;

        // First digit can't be '0' unless it's the only one

        if ((l > 1) && (c == '0'))
        {
            return 0;
        }

        while(l--)
        {
            // Each digit must be decimal

            if ((c < '0') || (c > '9'))
            {
                return 0;
            }

            t = (t * 10) + (c - '0');

            c = *cp++;
        }

        // Total accumulated number must be less than 256

        if (t > 255)
        {
            return 0;
        }

        // Pack number into 32 bit IP address representation

        ip |= ((uint32_t)t << 24);

        // First three numbers must terminate with '.', last one with '\0's

        if ((('\0' == c) && (i != 3)) || (('\0' != c) && (i == 3)))
        {
            return 0;
        }
    }

    return ip;
}

//DOM-IGNORE-END
