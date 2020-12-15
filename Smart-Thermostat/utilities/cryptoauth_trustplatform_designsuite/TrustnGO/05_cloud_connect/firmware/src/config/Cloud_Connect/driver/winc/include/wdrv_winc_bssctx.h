/*******************************************************************************
  WINC Driver BSS Context Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_bssctx.h

  Summary:
    WINC wireless driver BSS context header file.

  Description:
    This interface manages the BSS contexts which 'wrap' the state
      associated with BSSs.
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

#ifndef _WDRV_WINC_BSSCTX_H
#define _WDRV_WINC_BSSCTX_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_winc_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver BSS Context Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  BSS Context

  Summary:
    Context structure containing information about BSSs.

  Description:
    The context contains the SSID, channel and visibility of a BSS.

  Remarks:
    None.
*/

typedef struct
{
    /* SSID of BSS. */
    WDRV_WINC_SSID ssid;

#ifdef WDRV_WINC_DEVICE_EXT_CONNECT_PARAMS
    /* BSSID. */
    WDRV_WINC_MAC_ADDR bssid;
#endif
    /* Current channel of BSS. */
    uint8_t channel;

    /* Flag indicating if SSID should be cloaked. */
    bool cloaked;
} WDRV_WINC_BSS_CONTEXT;

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver BSS Context Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    bool WDRV_WINC_BSSCtxIsValid
    (
        const WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
        bool ssidValid
    )

  Summary:
    Tests if a BSS context is valid.

  Description:
    Tests the elements of the BSS context to judge if their values are legal.

  Precondition:
    None.

  Parameters:
    pBSSCtx   - Pointer to a BSS context.
    ssidValid - Flag indicating if the SSID within the context must be valid.

  Returns:
    true or false indicating if context is valid.

  Remarks:
    A valid SSID is one which has a non-zero length. The check is optional as
      it is legal for the SSID field to be zero length.

*/

bool WDRV_WINC_BSSCtxIsValid
(
    const WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
    bool ssidValid
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSCtxSetDefaults
    (
        WDRV_WINC_BSS_CONTEXT *const pBSSCtx
    )

  Summary:
    Configures a BSS context into a default legal state.

  Description:
    Ensures that each element of the structure is configured into a legal state.

  Precondition:
    None.

  Parameters:
    pBSSCtx - Pointer to a BSS context.

  Returns:
    WDRV_WINC_STATUS_OK             - The context has been configured.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSCtxSetDefaults
(
    WDRV_WINC_BSS_CONTEXT *const pBSSCtx
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSCtxSetSSID
    (
        WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
        uint8_t *const pSSID,
        uint8_t ssidLength
    )

  Summary:
    Configures the SSID of the BSS context.

  Description:
    The SSID string and length provided are copied into the BSS context.

  Precondition:
    None.

  Parameters:
    pBSSCtx    - Pointer to a BSS context.
    pSSID      - Pointer to buffer containing the new SSID.
    ssidLength - The length of the SSID held in the pSSID buffer.

  Returns:
    WDRV_WINC_STATUS_OK              - The context has been configured.
    WDRV_WINC_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_WINC_STATUS_INVALID_CONTEXT - The BSS context is not valid.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSCtxSetSSID
(
    WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
    uint8_t *const pSSID,
    uint8_t ssidLength
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSCtxSetBSSID
    (
        WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
        uint8_t *const pBSSID
    )

  Summary:
    Configures the BSSID of the BSS context.

  Description:
    The BSSID string is copied into the BSS context.

  Precondition:
    None.

  Parameters:
    pBSSCtx    - Pointer to a BSS context.
    pBSSID     - Pointer to buffer containing the new BSSID.

  Returns:
    WDRV_WINC_STATUS_OK              - The context has been configured.
    WDRV_WINC_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_WINC_STATUS_INVALID_CONTEXT - The BSS context is not valid.

  Remarks:
    None.

*/

#ifdef WDRV_WINC_DEVICE_EXT_CONNECT_PARAMS
WDRV_WINC_STATUS WDRV_WINC_BSSCtxSetBSSID
(
    WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
    uint8_t *const pBSSID
);
#endif

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSCtxSetChannel
    (
        WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
        uint8_t channel
    )

  Summary:
    Configures the channel of the BSS context.

  Description:
    The supplied channel value is copied into the BSS context.

  Precondition:
    None.

  Parameters:
    pBSSCtx - Pointer to a BSS context.
    channel - Channel number.

  Returns:
    WDRV_WINC_STATUS_OK              - The context has been configured.
    WDRV_WINC_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_WINC_STATUS_INVALID_CONTEXT - The BSS context is not valid.

  Remarks:
    channel may be WDRV_WINC_ALL_CHANNELS to represent no fixed channel.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSCtxSetChannel
(
    WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
    uint8_t channel
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSCtxSetSSIDVisibility
    (
        WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
        bool visible
    )

  Summary:
    Configures the visibility of the BSS context.

  Description:
    Specific to Soft-AP mode this flag defines if the BSS context will create a
      visible presence on air.

  Precondition:
    None.

  Parameters:
    pBSSCtx - Pointer to a BSS context.
    visible - Flag indicating if this BSS will be visible or not.

  Returns:
    WDRV_WINC_STATUS_OK              - The context has been configured.
    WDRV_WINC_STATUS_INVALID_ARG     - The parameters were incorrect.
    WDRV_WINC_STATUS_INVALID_CONTEXT - The BSS context is not valid.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSCtxSetSSIDVisibility
(
    WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
    bool visible
);

#endif /* _WDRV_WINC_BSSCTX_H */
