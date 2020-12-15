/*******************************************************************************
  WINC1500 Wireless Driver

  File Name:
    inet_ntop.c

  Summary:
    Implementation of standard inet_ntop function.

  Description:
    Implementation of standard inet_ntop function.
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
#include <stddef.h>
#include <sys/types.h>
#include "socket.h"

const char *inet_ntop(int af, const void *src, char *dst, size_t size)
{
    uint8_t i, v, t, c, n;
    char *rp = dst;
    uint32_t ip = ((struct in_addr*)src)->s_addr;

    if ((NULL == src) || (NULL == dst) || (size < 16))
    {
        return NULL;
    }

    for (i=0; i<4; i++)
    {
        t = ip;
        v = 100;

        // Check for zero

        if (t > 0)
        {
            n = 0;

            do
            {
                c = '0';
                while (t >= v)
                {
                    c++;
                    t -= v;
                }
                v /= 10;

                if (('0' != c) || (n > 0))
                {
                    *dst++ = c;

                    n++;
                }
            }
            while (v > 0);
        }
        else
        {
            *dst++ = '0';
        }

        if (3 == i)
        {
            *dst++ = '\0';
        }
        else
        {
            *dst++ = '.';
        }

        ip >>= 8;
    }

    return rp;
}

//DOM-IGNORE-END
