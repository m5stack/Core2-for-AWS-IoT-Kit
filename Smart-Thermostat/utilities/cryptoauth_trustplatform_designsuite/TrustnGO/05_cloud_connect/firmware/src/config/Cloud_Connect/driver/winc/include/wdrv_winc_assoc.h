/*******************************************************************************
  WINC Driver Association Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_assoc.h

  Summary:
    WINC wireless driver association header file.

  Description:
    This interface provides information about the current association with a
    peer device.
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

#ifndef _WDRV_WINC_ASSOC_H
#define _WDRV_WINC_ASSOC_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_winc_common.h"
#include "wdrv_winc_authctx.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Association Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  Combined IPv4/Ethernet Address

  Summary:
    A structure containing both an IPv4 and Ethernet address.

  Description:
    An IPv4 stored as a 32 bit number and an Ethernet address.

  Remarks:
    The top 8 bits of the IPv4 32 bit representation corresponds to
      the last byte of the IPv4 address, i.e. 192.168.0.1 = 0x0100A8C0
*/

typedef struct
{
    /* IPv4 address. */
    uint32_t ipAddress;

    /* Ethernet address. */
    uint8_t  macAddress[6];
} WDRV_WINC_NETWORK_ADDRESS;

// *****************************************************************************
/*  Association Information Callback.

  Summary:
    Pointer to a callback to provide information about the current association.

  Description:
    This data type defines a function callback to provide details of the SSID,
      network address, authentication type and initial signal strength (RSSI) of the
      current association.

  Parameters:
    handle          - Client handle obtained by a call to WDRV_WINC_Open.
    pSSID           - Pointer to current SSID.
    pPeerAddress    - Pointer to the peer devices address.
    authType        - Authentication type currently being used.
    rssi            - The current RSSI of the association.

  Returns:
    None.

  Remarks:
    See WDRV_WINC_AssociationSSIDGet, WDRV_WINC_AssociationPeerAddressGet and
    WDRV_WINC_AssociationAuthTypeGet.
*/

typedef void (*WDRV_WINC_ASSOC_CALLBACK)
(
    DRV_HANDLE handle,
    const WDRV_WINC_SSID *const pSSID,
    const WDRV_WINC_NETWORK_ADDRESS *const pPeerAddress,
    WDRV_WINC_AUTH_TYPE authType,
    int8_t rssi
);

// *****************************************************************************
/*  Association RSSI Callback.

  Summary:
    A callback to provide the current RSSI of the current association.

  Description:
    This callback provides details of the signal strength (RSSI) of the current
      association.

  Parameters:
    handle  - Client handle obtained by a call to WDRV_WINC_Open.
    rssi    - The current RSSI of the association.

  Returns:
    None.

  Remarks:
    None.
*/

typedef void (*WDRV_WINC_ASSOC_RSSI_CALLBACK)(DRV_HANDLE handle, int8_t rssi);

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Association Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_AssociationSSIDGet
    (
        DRV_HANDLE handle,
        WDRV_WINC_SSID *const pSSID,
        WDRV_WINC_ASSOC_CALLBACK const pfAssociationInfoCB
    )

  Summary:
    Retrieve the current association SSID.

  Description:
    Attempts to retrieve the SSID of the current association.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.
    A peer device needs to be connected and associated.

  Parameters:
    handle              - Client handle obtained by a call to WDRV_WINC_Open.
    pSSID               - Pointer to buffer to receive the SSID if available.
    pfAssociationInfoCB - Pointer to callback function to be used when SSID
                            is available.

  Returns:
    WDRV_WINC_STATUS_OK             - pSSID will contain the SSID.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected
                                            or there is no current association.
    WDRV_WINC_STATUS_RETRY_REQUEST  - The SSID is not available but it will be
                                            requested from the WINC.

  Remarks:
    If the SSID is not currently known to the driver (stored within the
      WINC) a request will be sent to the WINC and the return status
      will be WDRV_WINC_STATUS_RETRY_REQUEST. The callback function
      pfAssociationInfoCB can be provided which will be called when the WINC
      provides the association information to the driver. Alternatively the caller
      may poll this function until the return status is WDRV_WINC_STATUS_OK
      to obtain the SSID in pSSID.

*/

WDRV_WINC_STATUS WDRV_WINC_AssociationSSIDGet
(
    DRV_HANDLE handle,
    WDRV_WINC_SSID *const pSSID,
    WDRV_WINC_ASSOC_CALLBACK const pfAssociationInfoCB
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_AssociationPeerAddressGet
    (
        DRV_HANDLE handle,
        WDRV_WINC_NETWORK_ADDRESS *const pPeerAddress,
        WDRV_WINC_ASSOC_CALLBACK const pfAssociationInfoCB
    )

  Summary:
    Retrieve the current association peer device network address.

  Description:
    Attempts to retrieve the network address of the peer device in the
      current association.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.
    A peer device needs to be connected and associated.

  Parameters:
    handle              - Client handle obtained by a call to WDRV_WINC_Open.
    pPeerAddress        - Pointer to structure to receive the network address
                            if available.
    pfAssociationInfoCB - Pointer to callback function to be used when network
                            address is available.

  Returns:
    WDRV_WINC_STATUS_OK             - pPeerAddress will contain the network address.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected
                                            or there is no current association.
    WDRV_WINC_STATUS_RETRY_REQUEST  - The network address is not available
                                            but it will be requested from the WINC.

  Remarks:
    If the network address is not currently known to the driver (stored within the
      WINC) a request will be sent to the WINC and the return status
      will be WDRV_WINC_STATUS_RETRY_REQUEST. The callback function
      pfAssociationInfoCB can be provided which will be called when the WINC
      provides the association information to the driver. Alternatively the caller
      may poll this function until the return status is WDRV_WINC_STATUS_OK
      to obtain the network address in pPeerAddress.

*/

WDRV_WINC_STATUS WDRV_WINC_AssociationPeerAddressGet
(
    DRV_HANDLE handle,
    WDRV_WINC_NETWORK_ADDRESS *const pPeerAddress,
    WDRV_WINC_ASSOC_CALLBACK const pfAssociationInfoCB
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_AssociationAuthTypeGet
    (
        DRV_HANDLE handle,
        WDRV_WINC_AUTH_TYPE *const pAuthType,
        WDRV_WINC_ASSOC_CALLBACK const pfAssociationInfoCB
    )

  Summary:
    Retrieve the current association authentication type.

  Description:
    Attempts to retrieve the authentication type of the current association.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.
    A peer device needs to be connected and associated.

  Parameters:
    handle              - Client handle obtained by a call to WDRV_WINC_Open.
    pAuthType           - Pointer to element to receive the authentication type
                            if available.
    pfAssociationInfoCB - Pointer to callback function to be used when
                            authentication type is available.

  Returns:
    WDRV_WINC_STATUS_OK             - pAuthType will contain the
                                            authentication type.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected
                                            or there is no current association.
    WDRV_WINC_STATUS_RETRY_REQUEST  - The authentication type is not available
                                            but it will be requested from the WINC.

  Remarks:
    If the authentication type is not currently known to the driver (stored within the
      WINC) a request will be sent to the WINC and the return status
      will be WDRV_WINC_STATUS_RETRY_REQUEST. The callback function
      pfAssociationInfoCB can be provided which will be called when the WINC
      provides the association information to the driver. Alternatively the caller
      may poll this function until the return status is WDRV_WINC_STATUS_OK
      to obtain the authentication type in pAuthType.

*/

WDRV_WINC_STATUS WDRV_WINC_AssociationAuthTypeGet
(
    DRV_HANDLE handle,
    WDRV_WINC_AUTH_TYPE *const pAuthType,
    WDRV_WINC_ASSOC_CALLBACK const pfAssociationInfoCB
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_AssociationRSSIGet
    (
        DRV_HANDLE handle,
        int8_t *const pRSSI,
        WDRV_WINC_ASSOC_RSSI_CALLBACK const pfAssociationRSSICB
    )

  Summary:
    Retrieve the current association RSSI.

  Description:
    Attempts to retrieve the RSSI of the current association.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.
    A peer device needs to be connected and associated.

  Parameters:
    handle              - Client handle obtained by a call to WDRV_WINC_Open.
    pRSSI               - Pointer to variable to receive RSSI if available.
    pfAssociationRSSICB - Pointer to callback function to be used when
                            RSSI value is available.

  Returns:
    WDRV_WINC_STATUS_OK             - pRSSI will contain the RSSI.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected
                                            or there is no current association.
    WDRV_WINC_STATUS_RETRY_REQUEST  - The RSSI is not available but it will
                                            be requested from the WINC.

  Remarks:
    If the RSSI is not currently known to the driver (stored within the
      WINC) a request will be sent to the WINC and the return status
      will be WDRV_WINC_STATUS_RETRY_REQUEST. The callback function
      pfAssociationRSSICB can be provided which will be called when the WINC
      provides the RSSI information to the driver. Alternatively the caller
      may poll this function until the return status is WDRV_WINC_STATUS_OK
      to obtain the RSSI in pRSSI.

*/

WDRV_WINC_STATUS WDRV_WINC_AssociationRSSIGet
(
    DRV_HANDLE handle,
    int8_t *const pRSSI,
    WDRV_WINC_ASSOC_RSSI_CALLBACK const pfAssociationRSSICB
);

#endif /* _WDRV_WINC_ASSOC_H */
