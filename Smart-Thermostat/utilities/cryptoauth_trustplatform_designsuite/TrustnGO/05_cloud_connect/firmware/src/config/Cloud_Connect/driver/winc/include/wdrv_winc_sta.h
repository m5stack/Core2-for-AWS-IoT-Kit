/*******************************************************************************
  WINC Driver STA Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_sta.h

  Summary:
    WINC wireless driver STA header file.

  Description:
    WINC wireless driver STA header file.
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

#ifndef _WDRV_WINC_STA_H
#define _WDRV_WINC_STA_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_winc_common.h"
#include "wdrv_winc_bssctx.h"
#include "wdrv_winc_authctx.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************

#ifdef WDRV_WINC_DEVICE_BSS_ROAMING
// *****************************************************************************
/*  BSS Roaming Configuration

  Summary:
    Defines the BSS roaming configuration.

  Description:
    This enumeration defines the BSS roaming configuration.

  Remarks:
    None.
*/
typedef enum
{
    /* BSS Roaming is turned off. */
    WDRV_WINC_BSS_ROAMING_CFG_OFF,

    /* BSS Roaming is turned on, no IP renew occurs. */
    WDRV_WINC_BSS_ROAMING_CFG_ON,

    /* BSS Roaming is turned on, DHCP renew is request upon reconnection. */
    WDRV_WINC_BSS_ROAMING_CFG_ON_IP_RENEW
} WDRV_WINC_BSS_ROAMING_CFG;
#endif

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver STA Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSConnect
    (
        DRV_HANDLE handle,
        const WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
        const WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
        const WDRV_WINC_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Connects to a BSS in infrastructure station mode.

  Description:
    Using the defined BSS and authentication contexts this function requests
      the WINC connect to the BSS as an infrastructure station.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.
    A BSS context must have been created and initialized.
    An authentication context must have been created and initialized.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_WINC_Open.
    pBSSCtx          - Pointer to BSS context.
    pAuthCtx         - Pointer to authentication context.
    pfNotifyCallback - Pointer to notification callback function.

  Returns:
    WDRV_WINC_STATUS_OK              - The request has been accepted.
    WDRV_WINC_STATUS_NOT_OPEN        - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR   - The request to the WINC was rejected.
    WDRV_WINC_STATUS_INVALID_CONTEXT - The BSS context is not valid.
    WDRV_WINC_STATUS_CONNECT_FAIL    - The connection has failed.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSConnect
(
    DRV_HANDLE handle,
    const WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
    const WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
    const WDRV_WINC_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSReconnect
    (
        DRV_HANDLE handle,
        WDRV_WINC_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Reconnects to a BSS using stored credentials.

  Description:
    Reconnects to the previous BSS using credentials stored from last time.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.
    WDRV_WINC_BSSConnect must have previously been called to establish
      connection.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_WINC_Open.
    pfNotifyCallback - Pointer to notification callback function.

 Returns:
    WDRV_WINC_STATUS_OK              - The request has been accepted.
    WDRV_WINC_STATUS_NOT_OPEN        - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR   - The request to the WINC was rejected.
    WDRV_WINC_STATUS_CONNECT_FAIL    - The connection has failed.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSReconnect
(
    DRV_HANDLE handle,
    WDRV_WINC_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSDisconnect(DRV_HANDLE handle)

  Summary:
    Disconnects from a BSS.

  Description:
    Disconnects from an existing BSS.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle - Client handle obtained by a call to WDRV_WINC_Open.

  Returns:
    WDRV_WINC_STATUS_OK              - The request has been accepted.
    WDRV_WINC_STATUS_NOT_OPEN        - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_WINC_STATUS_DISCONNECT_FAIL - The disconnection has failed.
    WDRV_WINC_STATUS_REQUEST_ERROR   - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSDisconnect(DRV_HANDLE handle);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSRoamingConfigure
    (
        DRV_HANDLE handle,
        WDRV_WINC_BSS_ROAMING_CFG roamingCfg
    )

  Summary:
    Configures BSS roaming support.

  Description:
    Enables or disables BSS roaming support. If enabled the WINC can perform
      a DHCP renew of the current IP address if configured to do so, otherwise
      it will assume the existing IP address is still valid.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle     - Client handle obtained by a call to WDRV_WINC_Open.
    roamingCfg - Roaming configuration, see WDRV_WINC_BSS_ROAMING_CFG.

  Returns:
    WDRV_WINC_STATUS_OK              - The request has been accepted.
    WDRV_WINC_STATUS_NOT_OPEN        - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_WINC_STATUS_CONNECT_FAIL    - The disconnection has failed.
    WDRV_WINC_STATUS_REQUEST_ERROR   - The request to the WINC was rejected.

  Remarks:
    None.

*/

#ifdef WDRV_WINC_DEVICE_BSS_ROAMING
WDRV_WINC_STATUS WDRV_WINC_BSSRoamingConfigure
(
    DRV_HANDLE handle,
    WDRV_WINC_BSS_ROAMING_CFG roamingCfg
);
#endif

#endif /* _WDRV_WINC_STA_H */
