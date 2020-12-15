/*******************************************************************************
  WINC Wireless Driver Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc.h

  Summary:
    WINC Wireless Driver Header File

  Description:
    The WINC device driver provides an interface to manage the WINC1500 and
    WINC3400 devices. This file contains the main driver descriptor structure
    and ancillary functions and definitions.

    Other API's are provided in other header files, specifically:
      assoc       - Current association.
      bssfind     - BSS scan functionality.
      custie      - Custom IE management for Soft-AP.
      nvm         - Off line access the WINC SPI flash.
      powersave   - Power save control.
      socket      - TCP/IP sockets.
      softap      - Soft-AP mode.
      sta         - Infrastructure stations mode.
      systtime    - System time.
      wps         - WPS control.

      authctx     - Authentication context management.
      bssctx      - BSS context management.
      httpprovctx - HTTP provisioning context management.

      host_file   - File download via WINCs flash memory.

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

#ifndef _WDRV_WINC_H
#define _WDRV_WINC_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "configuration.h"
#include "definitions.h"
#include "wdrv_winc_bssfind.h"
#include "wdrv_winc_assoc.h"
#include "wdrv_winc_systime.h"
#include "wdrv_winc_softap.h"
#include "wdrv_winc_sta.h"
#include "wdrv_winc_wps.h"
#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
#include "wdrv_winc_ssl.h"
#endif
#ifdef WDRV_WINC_DEVICE_HOST_FILE_DOWNLOAD
#include "wdrv_winc_host_file.h"
#endif

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
    extern "C" {
#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  Firmware Version Information

  Summary:
    Defines the version information of the firmware.

  Description:
    This data type defines the version information of the firmware running on
    the WINC. This includes version, HIF protocol version and build date/times.

  Remarks:
    None.
*/

typedef struct
{
    /* Version number structure. */
    struct
    {
        /* Major version number. */
        uint16_t major;

        /* Major minor number. */
        uint16_t minor;

        /* Major patch number. */
        uint16_t patch;
    } version;

    /* HIF protocol version number structure. */
    struct
    {
        /* HIF major version number. */
        uint16_t major;

        /* HIF minor version number. */
        uint16_t minor;

        /* HIF block number. */
        uint16_t block;
    } hif;

    /* Build date/time structure. */
    struct
    {
        /* Build date string. */
        char date[12];

        /* Build time string. */
        char time[9];
    } build;
} WDRV_WINC_FIRMWARE_VERSION_INFO;

// *****************************************************************************
/*  Driver Version Information

  Summary:
    Defines the version information of the driver.

  Description:
    This data type defines the version information of the driver. This
    includes version and HIF protocol version.

  Remarks:
    None.
*/

typedef struct
{
    /* Version number structure. */
    struct
    {
        /* Major version number. */
        uint16_t major;

        /* Major minor number. */
        uint16_t minor;

        /* Major patch number. */
        uint16_t patch;
    } version;

    /* HIF protocol version number structure. */
    struct
    {
        /* HIF major version number. */
        uint16_t major;

        /* HIF minor version number. */
        uint16_t minor;

        /* HIF block number. */
        uint16_t block;
    } hif;
} WDRV_WINC_DRIVER_VERSION_INFO;

// *****************************************************************************
/*  WINC Operating Modes

  Summary:
    Defines the WINC operating mode.

  Description:
    This enumeration defines the possible operating modes of the WINC. These values
    are returned by WDRV_WINC_OperatingModeGet.

  Remarks:
    None.
*/

typedef enum
{
    /* An unknown or invalid operating mode. */
    WDRV_WINC_OP_MODE_UNKNOWN,

    /* Operating as an infrastructure station. */
    WDRV_WINC_OP_MODE_STA,

    /* Operating as a Soft-AP. */
    WDRV_WINC_OP_MODE_AP,

    /* Operating as a Soft-AP with built in support for provisioning. */
    WDRV_WINC_OP_MODE_PROV_AP
} WDRV_WINC_OP_MODE;

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
// *****************************************************************************
/*  ICMP Echo Status Codes

  Summary:
    Defines the ICMP echo status codes.

  Description:
    This enumeration defines the ICMP status supplied by the ICMP echo response
    callback.

  Remarks:
    This is only defined for use with the WINC in socket mode.
*/

typedef enum
{
    /* The operation completed successfully. */
    WDRV_WINC_ICMP_ECHO_STATUS_SUCCESS = 0,

    /* ICMP destination unreachable. */
    WDRV_WINC_ICMP_ECHO_STATUS_UNREACH = 1,

    /* The operation timed out. */
    WDRV_WINC_ICMP_ECHO_STATUS_TIMEOUT = 2
} WDRV_WINC_ICMP_ECHO_STATUS;
#endif

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
// *****************************************************************************
/* DHCP Address Event Handler Function Pointer

  Summary:
    Pointer to a DHCP address event handler function.

  Description:
    This data type defines a function event handler to receive notification
    of allocation of IP address via DHCP.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_WINC_Open.
    ipAddress   - IPv4 address.

  Returns:
    None.

  Remarks:
    Only supported with the socket mode WINC driver.

    See WDRV_WINC_IPSetUseDHCP and WDRV_WINC_IPDHCPServerConfigure.
*/

typedef void (*WDRV_WINC_DHCP_ADDRESS_EVENT_HANDLER)
(
    DRV_HANDLE handle,
    uint32_t ipAddress
);

// *****************************************************************************
/* ICMP Echo Response Event Handler Function Pointer

  Summary:
    Pointer to an ICMP echo response event handler function.

  Description:
    This data type defines a function event handler which is
    called in response to an ICMP echo request response event.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_WINC_Open.
    ipAddress   - IPv4 address.
    rtt         - The round-trip time.
    statusCode  - Status code.

  Returns:
    None.

  Remarks:
    Only supported with the socket mode WINC driver.

    The status code indicates if the ICMP echo request was successful or not.
    It may take the form of either:

        PING_ERR_SUCCESS        - The request was successful.
        PING_ERR_DEST_UNREACH   - The destination was unreachable.
        PING_ERR_TIMEOUT        - The request timed out.
*/

typedef void (*WDRV_WINC_ICMP_ECHO_RSP_EVENT_HANDLER)
(
    DRV_HANDLE handle,
    uint32_t ipAddress,
    uint32_t rtt,
    WDRV_WINC_ICMP_ECHO_STATUS statusCode
);
#else
// *****************************************************************************
/*  Ethernet Message Received Callback

  Summary:
    Pointer to a callback function to receive Ethernet frames from the WINC.

  Description:
    This data type defines a function callback for Ethernet frames.
    The Ethernet frame may be fragmented which is indicated by the isFragmented
    flag along with the fragNum fragment number.

  Parameters:
    handle          - Client handle obtained by a call to WDRV_WINC_Open.
    pEthMsg         - Pointer to the Ethernet frame.
    lengthEthMsg    - Length of the Ethernet frame.
    isFragmented    - A flag indicating if the Ethernet frame is fragmented.
    fragNum         - The fragment number of this Ethernet frame.

  Returns:
    None.

  Remarks:
    Only supported with the Ethernet mode WINC driver.

    See WDRV_WINC_EthernetRecvPacket.
*/

typedef void (*WDRV_WINC_ETH_MSG_RECV_CALLBACK)
(
    DRV_HANDLE handle,
    const uint8_t *const pEthMsg,
    uint16_t lengthEthMsg,
    bool isFragmented,
    uint8_t fragNum
);
#endif

// *****************************************************************************
/*  WINC Driver Descriptor

  Summary:
    The defines the WINC driver descriptor.

  Description:
    This data type defines the system level descriptor for the WINC driver.
    This structure is initialized by a call to WDRV_WINC_Initialize.

  Remarks:
    None.
*/

typedef struct
{
    /* Flag indicating if the driver has been initialized. */
    bool isInit;

    /* Driver status which can be queried via WDRV_WINC_Status. */
    SYS_STATUS sysStat;

    /* Intent used to open driver. */
    DRV_IO_INTENT intent;

    /* Flag indicating if this instance of the driver has been opened by
        a call to WDRV_WINC_Open. */
    bool isOpen;

    /* Flag indicating if this instance is operating as s station or soft-AP. */
    bool isAP;

    /* Flag indicating if this instance is a provisioning Soft-AP. */
    bool isProvisioning;

    /* Flag indicating if a connection has been established. */
    bool isConnected;

    /* Flag indicating if a BSS scan is currently in progress. */
    bool scanInProgress;

    /* Current index of the BSS scan results in lastBSSScanInfo if
        isBSSScanInfoValid is true. */
    uint8_t scanIndex;

    /* The number of scan results available. */
    uint8_t scanNumScanResults;

    /* Flag indicating if the contents of lastBSSScanInfo are valid. */
    bool isBSSScanInfoValid;

    /* Single result from last BSS scan, only valid if isBSSScanInfoValid is true. */
    WDRV_WINC_BSS_INFO lastBSSScanInfo;

    /* Flag indicating if the scan parameters have been modified. */
    bool scanParamDefault;

    /* The number of scan slots per channel. */
    uint8_t scanNumSlots;

    /* The length of each scan slot in milliseconds. */
    uint8_t scanActiveScanTime;

    /* Number of probe requests to be sent each scan slot. */
    uint8_t scanNumProbes;

    /* The Received Signal Strength Indicator threshold required for (fast) reconnection. */
    int8_t scanRSSIThreshold;

    /* The length of each scan slot in milliseconds. */
    uint16_t scanPassiveScanTime;

    /* Number of DTIM intervals to sleep during power save. */
    uint16_t powerSaveDTIMInterval;

    /* Current Ethernet address assigned to the WINC. */
    uint8_t ethAddress[6];

    /* Last RSSI value received from the WINC. */
    int8_t rssi;

    /* User supplied handle stored to allow callbacks to be associated with
        a user context. */
    uintptr_t userHandle;

    /* Flag indicating if the association information is currently valid. */
    bool assocInfoValid;

    /* SSID associated with the current connection. */
    WDRV_WINC_SSID assocSSID;

    /* Ethernet address of connected peer device. */
    WDRV_WINC_NETWORK_ADDRESS assocPeerAddress;

    /* Authentication type of the connection association. */
    WDRV_WINC_AUTH_TYPE assocAuthType;

    /* Callback to use for BSS find operations. */
    WDRV_WINC_BSSFIND_NOTIFY_CALLBACK pfBSSFindNotifyCB;

    /* Callback to use for notifying changes in connection state. */
    WDRV_WINC_BSSCON_NOTIFY_CALLBACK pfConnectNotifyCB;

    /* Callback to use for events relating to the WINC system time. */
    WDRV_WINC_SYSTIME_CURRENT_CALLBACK pfSystemTimeGetCurrentCB;

    /* Callback to use for retrieving association information from the WINC. */
    WDRV_WINC_ASSOC_CALLBACK pfAssociationInfoCB;

    /* Callback to use for retrieving association RSSI information from the WINC. */
    WDRV_WINC_ASSOC_RSSI_CALLBACK pfAssociationRSSICB;

    /* Callback to use for WPS discovery. */
    WDRV_WINC_WPS_DISC_CALLBACK pfWPSDiscoveryCB;

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    /* Flag indicating if a firmware update download is currently in progress. */
    bool updateInProgress;

    /* Flag indicating if the WINC has a valid IPv4 address. */
    bool haveIPAddress;

    /* Flag indicating if the WINC should use DHCP to obtain a IPv4 address. */
    bool useDHCP;

    /* Current IPv4 address of the WINC, if haveIPAddress is true. */
    uint32_t ipAddress;

    /* Current IPv4 netmask to use. */
    uint32_t netMask;

    /* Current DNS server IPv4 address to use. */
    uint32_t dnsServerAddress;

    /* Current default gateway IPv4 address to use. */
    uint32_t gatewayAddress;

    /* Current IPv4 address to use for the DHCP server. Effectively also the
        address of the WINC Soft-AP. */
    uint32_t dhcpServerAddress;

    /* Callback to use for DHCP events. */
    WDRV_WINC_DHCP_ADDRESS_EVENT_HANDLER pfDHCPAddressEventCB;

    /* Callback to use for ICHO echo responses. */
    WDRV_WINC_ICMP_ECHO_RSP_EVENT_HANDLER pfICMPEchoResponseCB;

    /* Callback to use for provisioning events. */
    WDRV_WINC_HTTPPROV_INFO_CALLBACK pfProvConnectInfoCB;

    /* Callback to use for events relating to firmware update downloads. */
    WDRV_WINC_STATUS_CALLBACK pfOTADownloadStatusCB;

    /* Callback to use for events relating to firmware switching. */
    WDRV_WINC_STATUS_CALLBACK pfSwitchFirmwareStatusCB;

    /* Callback to use for SSL cipher suite confirmation. */
    WDRV_WINC_SSL_CIPHERSUITELIST_CALLBACK pfSSLCipherSuiteListCB;
    
    /* Callback to use for SSL cipher suite confirmation. */
    WDRV_WINC_SSL_REQ_ECC_CALLBACK pfSSLReqECCCB;
#else
    /* Flag indicating that the receive Ethernet buffer has been initialized. */
    bool isEthBufSet;

    /* Ethernet fragment number of the next Ethernet frame to receive. */
    uint8_t ethFragNum;

    /* Callback to use to pass received Ethernet messages to the user. */
    WDRV_WINC_ETH_MSG_RECV_CALLBACK pfEthernetMsgRecvCB;
#endif
#ifdef WDRV_WINC_ENABLE_BLE
    /* Flag indicating if BLE is active. */
    bool bleActive;
#endif
#ifdef WDRV_WINC_DEVICE_HOST_FILE_DOWNLOAD
    WDRV_WINC_HOST_FILE_DCPT hostFileDcpt;

    WDRV_WINC_HOST_FILE_STATUS_CALLBACK pfHostFileCB;
#endif


    /* Semaphore for ISR to signal foreground. */
    OSAL_SEM_HANDLE_TYPE isrSemaphore;
    
    /* Mutex for event process. */
    OSAL_MUTEX_HANDLE_TYPE eventProcessMutex;
 
} WDRV_WINC_DCPT;

// *****************************************************************************
// *****************************************************************************
// Section: WINC Debugging Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_DebugRegisterCallback
    (
        WDRV_WINC_DEBUG_PRINT_CALLBACK const pfDebugPrintCallback
    )

  Summary:
    Register callback for debug serial stream.

  Description:
    The debug serial stream provides a printf-like stream of messages from within
    the WINC driver. The caller can provide a function to be called when
    output is available.

  Precondition:
    None.

  Parameters:
    pfDebugPrintCallback - Function pointer to printf-like function.

  Returns:
    None.

  Remarks:
    None.

 */

void WDRV_WINC_DebugRegisterCallback(WDRV_WINC_DEBUG_PRINT_CALLBACK const pfDebugPrintCallback);

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Client Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    DRV_HANDLE WDRV_WINC_Open(const SYS_MODULE_INDEX index, const DRV_IO_INTENT intent)

  Summary:
    Opens an instance of the WINC driver.

  Description:
    Opens an instance of the WINC driver and returns a handle which must be
    used when calling other driver functions.

  Precondition:
    WDRV_WINC_Initialize should have been called.

  Parameters:
    index   - Identifier for the driver instance to be opened.
    intent  - Zero or more of the values from the enumeration
                DRV_IO_INTENT ORed together to indicate the intended use
                of the driver

  Returns:
    - Valid handle - if the open function succeeded
    - DRV_HANDLE_INVALID - if an error occurs

  Remarks:
    The index and intent parameters are not used in the current implementation
    and are maintained only for compatibility with the generic driver Open
    function signature.

*/

DRV_HANDLE WDRV_WINC_Open(const SYS_MODULE_INDEX index, const DRV_IO_INTENT intent);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_Close(DRV_HANDLE handle)

  Summary:
    Closes an instance of the WINC driver.

  Description:
    This is the function that closes an instance of the MAC.
    All per client data is released and the handle can no longer be used
    after this function is called.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle  - Client handle obtained by a call to WDRV_WINC_Open.

  Returns:
    None.

  Remarks:
    None.

*/

void WDRV_WINC_Close(DRV_HANDLE handle);

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver General Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_MulticastMACFilterAdd
    (
        DRV_HANDLE handle,
        const uint8_t *pEthAddress
    )

  Summary:
    Adds an Ethernet address to the multicast filter.

  Description:
    To receive multicast packets the multicast Ethernet address must be added
    to the receive filter on the WINC.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_WINC_Open.
    pEthAddress - Pointer to a valid multicast Ethernet address.

  Returns:
    WDRV_WINC_STATUS_OK             - A request to add the address was made
                                            to the WINC.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_MulticastMACFilterAdd
(
    DRV_HANDLE handle,
    const uint8_t *pEthAddress
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_MulticastMACFilterRemove
    (
        DRV_HANDLE handle,
        const uint8_t *pEthAddress
    )

  Summary:
    Removes an Ethernet address from the multicast filter.

  Description:
    To stop receiving multicast packets the multicast Ethernet address must be
    removed from the receive filter on the WINC.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_WINC_Open.
    pEthAddress - Pointer to a valid multicast Ethernet address.

  Returns:
    WDRV_WINC_STATUS_OK             - A request to remove the address was made
                                            to the WINC.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_MulticastMACFilterRemove
(
    DRV_HANDLE handle,
    const uint8_t *pEthAddress
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_EthernetAddressGet
    (
        DRV_HANDLE handle,
        uint8_t *pEthAddress
    )

  Summary:
    Returns the current Ethernet address assigned to the WINC.

  Description:
    Returns the current Ethernet address assigned to the WINC.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_WINC_Open.
    pEthAddress - Pointer to a buffer to receive the Ethernet address.

  Returns:
    WDRV_WINC_STATUS_OK             - The current Ethernet address was
                                            returned in pEthAddress[0..5].
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_EthernetAddressGet
(
    DRV_HANDLE handle,
    uint8_t *pEthAddress
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_OP_MODE WDRV_WINC_OperatingModeGet(DRV_HANDLE handle)

  Summary:
    Returns the current operating mode of the WINC.

  Description:
    Returns the current operating mode of the WINC.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle  - Client handle obtained by a call to WDRV_WINC_Open.

  Returns:
    WDRV_WINC_OP_MODE_UNKNOWN   - Unable to supply the operating mode
    WDRV_WINC_OP_MODE_STA       - The WINC is operating as a STA
    WDRV_WINC_OP_MODE_AP        - The WINC is operating as a Soft-AP
    WDRV_WINC_OP_MODE_PROV_AP   - The WINC is operating as a Soft-AP
                                        for the purposes of provisioning.

  Remarks:
    None.

*/

WDRV_WINC_OP_MODE WDRV_WINC_OperatingModeGet(DRV_HANDLE handle);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_UserHandleSet(DRV_HANDLE handle, uintptr_t userHandle)

  Summary:
    Associates a caller supplied handle with the driver instance.

  Description:
    The caller is able to associate a user supplied handle with this instance of
    the WINC driver which can be later retrieved when any callbacks are called
    by the driver by calling WDRV_WINC_UserHandleGet.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_WINC_Open.
    userHandle  - A generic user handle/pointer to stored with the driver.

  Returns:
    WDRV_WINC_STATUS_OK             - The handle has been stored.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_UserHandleSet(DRV_HANDLE handle, uintptr_t userHandle);

//*******************************************************************************
/*
  Function:
    uintptr_t WDRV_WINC_UserHandleGet(DRV_HANDLE handle)

  Summary:
    Returns the current user supplied handle associated with the driver.

  Description:
    This function returns the handle previously associated with the driver
    instance by a call to WDRV_WINC_UserHandleSet.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle  - Client handle obtained by a call to WDRV_WINC_Open.

  Returns:
    - The current handle associated with the driver.
    - NULL if no handle associated.

  Remarks:
    None.

*/

uintptr_t WDRV_WINC_UserHandleGet(DRV_HANDLE handle);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_InfoDriverVersionGet
    (
        DRV_HANDLE handle,
        WDRV_WINC_DRIVER_VERSION_INFO *const pDriverVersion
    )

  Summary:
    Returns the drivers version information.

  Description:
    Returns information on the drivers version and HIF protocol support.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle         - Client handle obtained by a call to WDRV_WINC_Open.
    pDriverVersion - Pointer to structure to receive version information.

  Returns:
    WDRV_WINC_STATUS_OK             - The information has been returned.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_InfoDriverVersionGet
(
    DRV_HANDLE handle,
    WDRV_WINC_DRIVER_VERSION_INFO *const pDriverVersion
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_InfoDeviceFirmwareVersionGet
    (
        DRV_HANDLE handle,
        bool active,
        WDRV_WINC_FIRMWARE_VERSION_INFO *const pFirmwareVersion
    )

  Summary:
    Returns the WINC firmware version information.

  Description:
    Returns information on the WINC firmware version, HIF protocol support
      and build date/time.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_WINC_Open.
    active           - Flag indicating if the active of inactive image is queried.
    pFirmwareVersion - Pointer to structure to receive version information.

  Returns:
    WDRV_WINC_STATUS_OK             - The information has been returned.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_InfoDeviceFirmwareVersionGet
(
    DRV_HANDLE handle,
    bool active,
    WDRV_WINC_FIRMWARE_VERSION_INFO *const pFirmwareVersion
);

//*******************************************************************************
/*
  Function:
    uint32_t WDRV_WINC_InfoDeviceIDGet(DRV_HANDLE handle)

  Summary:
    Returns the device ID.

  Description:
    Returns the device ID assigned to the WINC.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle - Client handle obtained by a call to WDRV_WINC_Open.

  Returns:
    Device ID or zero if an error occurs.

  Remarks:
    None.

*/

uint32_t WDRV_WINC_InfoDeviceIDGet(DRV_HANDLE handle);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_InfoDeviceMACAddressGet
    (
        DRV_HANDLE handle,
        bool permanent,
        uint8_t *const pMACAddress
    )

  Summary:
    Retrieves the MAC address of the WINC.

  Description:
    Retrieves either the permanent MAC address stored in OTP memory or the current
      working MAC address.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_WINC_Open.
    permanent   - Flag indicating if the permanent or working MAC address is returned.
    pMACAddress - Pointer to buffer to receive the MAC address.

  Returns:
    WDRV_WINC_STATUS_OK             - The information has been returned.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_InfoDeviceMACAddressGet
(
    DRV_HANDLE handle,
    bool permanent,
    uint8_t *const pMACAddress
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_InfoDeviceNameSet
    (
        DRV_HANDLE handle,
        const char* pDeviceName
    )

  Summary:
    Sets the WiFi Direct Device Name.

  Description:
    Sets the WiFi Direct Device Name.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_WINC_Open.
    pDeviceName - Pointer to device name.

  Returns:
    WDRV_WINC_STATUS_OK             - The information has been returned.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_InfoDeviceNameSet
(
    DRV_HANDLE handle,
    const char* pDeviceName
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_GainTableIndexSet(DRV_HANDLE handle, uint8_t index)

  Summary:
    Sets the gain table index.

  Description:
    Sets the gain table index.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle  - Client handle obtained by a call to WDRV_WINC_Open.
    index   - Gain table index to use.

  Returns:
    WDRV_WINC_STATUS_OK             - The information has been returned.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

#ifdef WDRV_WINC_DEVICE_MULTI_GAIN_TABLE
WDRV_WINC_STATUS WDRV_WINC_GainTableIndexSet(DRV_HANDLE handle, uint8_t index);
#endif

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _WDRV_WINC_H */
