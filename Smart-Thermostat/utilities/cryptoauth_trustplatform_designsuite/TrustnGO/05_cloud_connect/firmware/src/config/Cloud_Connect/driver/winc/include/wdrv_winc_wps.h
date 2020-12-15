/*******************************************************************************
  WINC Driver WPS Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_wps.h

  Summary:
    WINC wireless driver WPS header file.

  Description:
    Provides an interface use WPS for BSS discovery.
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

#ifndef _WDRV_WINC_WPS_H
#define _WDRV_WINC_WPS_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_winc_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver WPS Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  WPS Discovery Callback

  Summary:
    Callback to report WPS discovery results.

  Description:
    Callback to report WPS discovery results.

  Remarks:
    None.
*/

typedef void (*WDRV_WINC_WPS_DISC_CALLBACK)
(
    DRV_HANDLE handle,
    WDRV_WINC_BSS_CONTEXT *pBSSCtx,
    WDRV_WINC_AUTH_CONTEXT *pAuthCtx
);

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver WPS Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_WPSEnrolleeDiscoveryStartPIN
    (
        DRV_HANDLE handle,
        uint32_t pin,
        const WDRV_WINC_WPS_DISC_CALLBACK pfWPSDiscoveryCallback
    )

  Summary:
    Start WPS discovery (PIN).

  Description:
    Starts the WPS discovery as an enrollee using the PIN method.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle                 - Client handle obtained by a call to WDRV_WINC_Open.
    pin                    - WPS PIN number.
    pfWPSDiscoveryCallback - Callback function to return discovery results to.

  Returns:
    WDRV_WINC_STATUS_OK            - Discovery request accepted.
    WDRV_WINC_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR - The request to the WINC was rejected.

  Remarks:
    The pin number is an 8-digit number where the least significant digit is
      a checksum as defined by WPS.

*/

WDRV_WINC_STATUS WDRV_WINC_WPSEnrolleeDiscoveryStartPIN
(
    DRV_HANDLE handle,
    uint32_t pin,
    const WDRV_WINC_WPS_DISC_CALLBACK pfWPSDiscoveryCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_WPSEnrolleeDiscoveryStartPBC
    (
        DRV_HANDLE handle,
        const WDRV_WINC_WPS_DISC_CALLBACK pfWPSDiscoveryCallback
    )

  Summary:
    Start WPS discovery (Push Button).

  Description:
    Starts the WPS discovery as an enrollee using the Push Button method.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle                 - Client handle obtained by a call to WDRV_WINC_Open.
    pfWPSDiscoveryCallback - Callback function to return discovery results to.

  Returns:
    WDRV_WINC_STATUS_OK            - Discovery request accepted.
    WDRV_WINC_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_WPSEnrolleeDiscoveryStartPBC
(
    DRV_HANDLE handle,
    const WDRV_WINC_WPS_DISC_CALLBACK pfWPSDiscoveryCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_WPSEnrolleeDiscoveryStop(DRV_HANDLE handle)

  Summary:
    Stop WPS discovery.

  Description:
    Stops the WPS discovery if it was started.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle                 - Client handle obtained by a call to WDRV_WINC_Open.

  Returns:
    WDRV_WINC_STATUS_OK            - Discovery request accepted.
    WDRV_WINC_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_WPSEnrolleeDiscoveryStop(DRV_HANDLE handle);

#endif /* _WDRV_WINC_WPS_H */
