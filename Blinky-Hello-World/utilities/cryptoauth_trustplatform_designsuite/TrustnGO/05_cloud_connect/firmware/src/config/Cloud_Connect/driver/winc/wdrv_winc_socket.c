
/*******************************************************************************
  WINC Driver Socket Mode Implementation

  File Name:
    wdrv_winc_socket.c

  Summary:
    WINC wireless driver (Socket mode) implementation.

  Description:
    This interface provides extra functionality required for socket mode operation.
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
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include "wdrv_winc.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc_socket.h"
#include "m2m_ota.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Socket Mode Callback Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    void _WDRV_WINC_ICMPEchoResponseCallback
    (
        uint32_t ipAddress,
        uint32_t rtt,
        uint8_t errorCode
    )

  Summary:
    ICMP echo response event callback.

  Description:

  Precondition:
    None.

  Parameters:
    ipAddress - IP address of echo request target.
    rtt       - Round trip time in milliseconds.
    errorCode - Error code if echo request failed.

  Returns:
    None.

  Remarks:
    None.

*/

static void _WDRV_WINC_ICMPEchoResponseCallback
(
    uint32_t ipAddress,
    uint32_t rtt,
    uint8_t errorCode
)
{
    /* Open driver to obtain handle. */
    DRV_HANDLE handle = WDRV_WINC_Open(0, 0);

    /* Ensure handle is valid. */
    if (DRV_HANDLE_INVALID != handle)
    {
        WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

        if (NULL != pDcpt->pfICMPEchoResponseCB)
        {
            /* Call ICMP echo response callback if present. */
            pDcpt->pfICMPEchoResponseCB(handle, ipAddress, rtt, errorCode);
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Socket Mode Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_IPAddressSet
    (
        DRV_HANDLE handle,
        uint32_t ipAddress,
        uint32_t netMask
    )

  Summary:
    Configures an IPv4 address.

  Description:
    Disables DHCP on the WINC and assigns a static IPv4 address. The address
      isn't applied to the WINC until a connection is formed.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_IPAddressSet
(
    DRV_HANDLE handle,
    uint32_t ipAddress,
    uint32_t netMask
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure WINC is not connected. */
    if (true == pDcpt->isConnected)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Set IP address details. */
    pDcpt->ipAddress = ipAddress;
    pDcpt->netMask   = netMask;
    pDcpt->useDHCP   = false;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    uint32_t WDRV_WINC_IPAddressGet(DRV_HANDLE handle)

  Summary:
    Returns the current IPv4 address.

  Description:
    Returns the current IPv4 address if configured.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

uint32_t WDRV_WINC_IPAddressGet(DRV_HANDLE handle)
{
    const WDRV_WINC_DCPT *const pDcpt = (const WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return 0;
    }

    /* Ensure driver is open and has obtained an IP address. */
    if ((false == pDcpt->isOpen) || (false == pDcpt->haveIPAddress))
    {
        return 0;
    }

    return pDcpt->ipAddress;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_IPDNSServerAddressSet
    (
        DRV_HANDLE handle,
        uint32_t dnsServerAddress
    )

  Summary:
    Configures the DNS server address.

  Description:
    Disables DHCP on the WINC and assigns a static DNS address. The address
      isn't applied to the WINC until a connection is formed.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_IPDNSServerAddressSet
(
    DRV_HANDLE handle,
    uint32_t dnsServerAddress
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure WINC is not connected. */
    if (true == pDcpt->isConnected)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Set DNS server address and turn off DHCP. */
    pDcpt->dnsServerAddress = dnsServerAddress;
    pDcpt->useDHCP = false;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_IPDefaultGatewaySet
    (
        DRV_HANDLE handle,
        uint32_t gatewayAddress
    )

  Summary:
    Configures the default gateway used by the WINC.

  Description:
    Disables DHCP on the WINC and assigns a static default gateway address.
      The address isn't applied to the WINC until a connection is formed.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_IPDefaultGatewaySet
(
    DRV_HANDLE handle,
    uint32_t gatewayAddress
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure WINC is not connected. */
    if (true == pDcpt->isConnected)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Set default gateway address and turn off DHCP. */
    pDcpt->gatewayAddress = gatewayAddress;
    pDcpt->useDHCP = false;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_IPUseDHCPSet
    (
        DRV_HANDLE handle,
        const WDRV_WINC_DHCP_ADDRESS_EVENT_HANDLER pfDHCPAddressEventCallback
    )

  Summary:
    Enables DHCP.

  Description:
    Enables the use of DHCP by the WINC. This removes any static IP
      configuration. The use of DHCP isn't applied to the WINC until a
      connection is formed.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_IPUseDHCPSet
(
    DRV_HANDLE handle,
    const WDRV_WINC_DHCP_ADDRESS_EVENT_HANDLER pfDHCPAddressEventCallback
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure WINC is not connected. */
    if (true == pDcpt->isConnected)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Enable DHCP and set callback. */
    pDcpt->useDHCP = true;
    pDcpt->pfDHCPAddressEventCB = pfDHCPAddressEventCallback;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    bool WDRV_WINC_IPUseDHCPGet(DRV_HANDLE handle)

  Summary:
    Determine if DHCP is to be used.

  Description:
     Determines if the WINC will use DHCP for IP address configuration.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

bool WDRV_WINC_IPUseDHCPGet(DRV_HANDLE handle)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return false;
    }

    return ((true == pDcpt->useDHCP) || (0 == pDcpt->ipAddress));
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_IPDHCPServerConfigure
    (
        DRV_HANDLE handle,
        uint32_t ipAddress,
        WDRV_WINC_DHCP_ADDRESS_EVENT_HANDLER const pfDHCPAddressEventCallback
    )

  Summary:
    Configures the Soft-AP DHCP server.

  Description:
    Configures the IPv4 address and callback for the DHCP server.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_IPDHCPServerConfigure
(
    DRV_HANDLE handle,
    uint32_t ipAddress,
    uint32_t netMask,
    const WDRV_WINC_DHCP_ADDRESS_EVENT_HANDLER pfDHCPAddressEventCallback
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Set DHCP server address and event callback. */
    pDcpt->dhcpServerAddress = ipAddress;
    pDcpt->pfDHCPAddressEventCB = pfDHCPAddressEventCallback;
#ifdef WDRV_WINC_DEVICE_SOFT_AP_EXT
    pDcpt->netMask = netMask;
#endif

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    bool WDRV_WINC_IPLinkActive(DRV_HANDLE handle)

  Summary:
    Indicates state of IP link.

  Description:
    Returns a flag indicating if the IP link is active or not.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

bool WDRV_WINC_IPLinkActive(DRV_HANDLE handle)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return false;
    }

    return pDcpt->haveIPAddress;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_ICMPEchoRequest
    (
        DRV_HANDLE handle,
        uint32_t ipAddress,
        uint8_t ttl,
        const WDRV_WINC_ICMP_ECHO_RSP_EVENT_HANDLER pfICMPEchoResponseCB
    )

  Summary:
    Sends an ICMP echo request.

  Description:
    Sends an ICMP echo request packet to the IP specified.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_ICMPEchoRequest
(
    DRV_HANDLE handle,
    uint32_t ipAddress,
    uint8_t ttl,
    const WDRV_WINC_ICMP_ECHO_RSP_EVENT_HANDLER pfICMPEchoResponseCB
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    if ((NULL == pDcpt) || (0 == ipAddress))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Send ICMP echo request. */
    if (M2M_SUCCESS != m2m_ping_req(ipAddress, ttl, &_WDRV_WINC_ICMPEchoResponseCallback))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    pDcpt->pfICMPEchoResponseCB = pfICMPEchoResponseCB;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_OTAUpdateFromURL
    (
        DRV_HANDLE handle,
        char *pURL,
        WDRV_WINC_STATUS_CALLBACK pfUpdateStatusCB
    )

  Summary:
    Perform OTA from URL provided.

  Description:
    Using the URL provided the WINC will attempt to download and store the
      updated firmware image.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_OTAUpdateFromURL
(
    DRV_HANDLE handle,
    char *pURL,
    const WDRV_WINC_STATUS_CALLBACK pfUpdateStatusCB
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pURL))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure an update isn't in progress. */
    if (true == pDcpt->updateInProgress)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Start an OTA update. */
    if (M2M_SUCCESS != m2m_ota_start_update((uint8_t*)pURL))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Set in progress flag and callback. */
    pDcpt->updateInProgress      = true;
    pDcpt->pfOTADownloadStatusCB = pfUpdateStatusCB;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_OTAUpdateAbort(DRV_HANDLE handle)

  Summary:
    Abort an OTA update.

  Description:
    Requests a current OTA update operation be aborted.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_OTAUpdateAbort(DRV_HANDLE handle)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure an update isn't in progress. */
    if (false == pDcpt->updateInProgress)
    {
        return WDRV_WINC_STATUS_OK;
    }

    /* Abort the OTA. */
    if (M2M_SUCCESS != m2m_ota_abort())
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_SwitchActiveFirmwareImage
    (
        DRV_HANDLE handle,
        const WDRV_WINC_STATUS_CALLBACK pfSwitchFirmwareStatusCB
    )

  Summary:
    Switch active firmware image to inactive partition.

  Description:
    If there is an inactive firmware partition, either from an OTA update of from
      a previously disabled firmware image this function switches the WINC to
      use it. A reset of the WINC is required before the image becomes active.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_SwitchActiveFirmwareImage
(
    DRV_HANDLE handle,
    const WDRV_WINC_STATUS_CALLBACK pfSwitchFirmwareStatusCB
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Switch active firmware partitions. */
    if (M2M_SUCCESS != m2m_ota_switch_firmware())
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Store callback. */
    pDcpt->pfSwitchFirmwareStatusCB = pfSwitchFirmwareStatusCB;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_SocketRegisterEventCallback
    (
        DRV_HANDLE handle,
        tpfAppSocketCb pfAppSocketCb
    )

  Summary:
    Register an event callback for socket events.

  Description:
    Socket events are dispatched to the application via this callback.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_SocketRegisterEventCallback
(
    DRV_HANDLE handle,
    tpfAppSocketCb pfAppSocketCb
)
{
    if (0 == handle)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    registerSocketEventCallback(pfAppSocketCb);

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_SocketRegisterResolverCallback
    (
        DRV_HANDLE handle,
        tpfAppResolveCb pfAppResolveCb
    )

  Summary:
    Register an event callback for DNS resolver events.

  Description:
    DNS resolver events are dispatched to the application via this callback.

  Remarks:
    See wdrv_winc_socket.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_SocketRegisterResolverCallback
(
    DRV_HANDLE handle,
    tpfAppResolveCb pfAppResolveCb
)
{
    if (0 == handle)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    registerSocketResolveCallback(pfAppResolveCb);

    return WDRV_WINC_STATUS_OK;
}
