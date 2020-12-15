/*******************************************************************************
  Device compatibility Header File

  Company:
    Microchip Technology Inc.

  File Name:
    atsamd21e18a_compat.h

  Summary:
    This file includes deprecated macro names that are retained for the purpose
    of compatibility with Harmony 3 Middleware. Users are discouraged from using
    macros defined in this file. Recommend to use macros defined in the device
    header file instead.

  Description:
    None

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
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

#ifndef ATSAMD21E18A_COMPAT_H

#ifndef DEVICE_DESC_BANK_NUMBER
#define DEVICE_DESC_BANK_NUMBER USB_DEVICE_DESC_BANK_NUMBER
#endif //DEVICE_DESC_BANK_NUMBER

#ifndef HOST_DESC_BANK_NUMBER
#define HOST_DESC_BANK_NUMBER USB_HOST_DESC_BANK_NUMBER
#endif //HOST_DESC_BANK_NUMBER

#ifndef DEVICE_ENDPOINT_NUMBER
#define DEVICE_ENDPOINT_NUMBER USB_DEVICE_ENDPOINT_NUMBER
#endif //DEVICE_ENDPOINT_NUMBER

#ifndef HOST_PIPE_NUMBER
#define HOST_PIPE_NUMBER USB_HOST_PIPE_NUMBER
#endif //HOST_PIPE_NUMBER

#endif //ATSAMD21E18A_COMPAT_H