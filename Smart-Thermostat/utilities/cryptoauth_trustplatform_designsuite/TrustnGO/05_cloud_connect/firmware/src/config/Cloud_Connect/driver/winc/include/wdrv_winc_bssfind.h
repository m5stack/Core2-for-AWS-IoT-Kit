/*******************************************************************************
  WINC Driver BSS Find Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_bssfind.h

  Summary:
    WINC wireless driver BSS scanning header file.

  Description:
    This interface manages the operation of searching for a BSS.

    Searching operates on the find-first/find-next principal. When a find-first
      operation is requested the WINC will scan for available BSSs on the
      channels specified. The results will be provided via callback or polling
      and iterated over using a find-next operation.

    The callback function supplied to WDRV_WINC_BSSFindFirst may return true
      if the user wishes the next scan results to be automatically requested. By
      returning true the user can avoid needing to call WDRV_WINC_BSSFindNext,
      the callback will be called for each BSS found.
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

#ifndef _WDRV_WINC_BSSFIND_H
#define _WDRV_WINC_BSSFIND_H

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
// Section: WINC Driver BSS Find Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  Channel Mask.

  Summary:
    List of possible channel mask elements.

  Description:
    The channel mask consists of a single bit for each discrete channel. Channels
      maybe combined to form groups of channels, some of which are defined in
      this list.

  Remarks:
    None.
*/

typedef enum
{
    /* 2.4Ghz (2412 MHz) channel 1. */
    WDRV_WINC_CM_2_4G_CH1 = 0x0001,

    /* 2.4Ghz (2417 MHz) channel 2. */
    WDRV_WINC_CM_2_4G_CH2 = 0x0002,

    /* 2.4Ghz (2422 MHz) channel 3. */
    WDRV_WINC_CM_2_4G_CH3 = 0x0004,

    /* 2.4Ghz (2427 MHz) channel 4. */
    WDRV_WINC_CM_2_4G_CH4 = 0x0008,

    /* 2.4Ghz (2432 MHz) channel 5. */
    WDRV_WINC_CM_2_4G_CH5 = 0x0010,

    /* 2.4Ghz (2437 MHz) channel 6. */
    WDRV_WINC_CM_2_4G_CH6 = 0x0020,

    /* 2.4Ghz (2442 MHz) channel 7. */
    WDRV_WINC_CM_2_4G_CH7 = 0x0040,

    /* 2.4Ghz (2447 MHz) channel 8. */
    WDRV_WINC_CM_2_4G_CH8 = 0x0080,

    /* 2.4Ghz (2452 MHz) channel 9. */
    WDRV_WINC_CM_2_4G_CH9 = 0x0100,

    /* 2.4Ghz (2457 MHz) channel 10. */
    WDRV_WINC_CM_2_4G_CH10 = 0x0200,

    /* 2.4Ghz (2462 MHz) channel 11. */
    WDRV_WINC_CM_2_4G_CH11 = 0x0400,

    /* 2.4Ghz (2467 MHz) channel 12. */
    WDRV_WINC_CM_2_4G_CH12 = 0x0800,

    /* 2.4Ghz (2472 MHz) channel 13. */
    WDRV_WINC_CM_2_4G_CH13 = 0x1000,

    /* 2.4Ghz (2484 MHz) channel 14. */
    WDRV_WINC_CM_2_4G_CH14 = 0x2000,

    /* 2.4Ghz channels 1 through 14 */
    WDRV_WINC_CM_2_4G_ALL = 0x3fff,

    /* 2.4Ghz channels 1 through 11 */
    WDRV_WINC_CM_2_4G_NORTH_AMERICA = 0x07ff,

    /* 2.4Ghz channels 1 through 13 */
    WDRV_WINC_CM_2_4G_EUROPE = 0x1fff,

    /* 2.4Ghz channels 1 through 14 */
    WDRV_WINC_CM_2_4G_ASIA = 0x3fff
} WDRV_WINC_CHANNEL_MASK;

// *****************************************************************************
/*  BSS Information

  Summary:
    Structure containing information about a BSS.

  Description:
    This structure contains the BSSID and SSID of the BSS as well as the
      signal strength RSSI. The authentication type used by the BSS and the
      channel it is operating on are also provided.

  Remarks:
    None.
*/

typedef struct
{
    /* BSSID of BSS provider. */
    uint8_t bssid[6];

    /* SSID of BSS. */
    WDRV_WINC_SSID ssid;

    /* Signal strength RSSI of BSS. */
    int8_t rssi;

    /* Authentication type of BSS. */
    WDRV_WINC_AUTH_TYPE authType;

    /* Channel BSS is operating on. */
    uint8_t channel;
} WDRV_WINC_BSS_INFO;

// *****************************************************************************
/*  BSS Discovery Notification Callback

  Summary:
    Callback to signal discovery of a BSS.

  Description:
    When the information about a discovered BSS is requested the driver will
      use this callback to provide the BSS information to the user.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_WINC_Open.
    index       - Index of BSS find results.
    ofTotal     - Total number of BSS find results.
    pBSSInfo    - Pointer to BSS information structure.

  Returns:
    If true is returned the WINC driver will automatically fetch the next BSS
    find results which will cause a later call to this callback. If false is
    returned the WINC driver will not fetch the next BSS find results, the user
    must call WDRV_WINC_BSSFindNext to retrieve them.

  Remarks:
    The callback function must return true or false. true indicates that the user
      wishes the next BSS information structure be provided. false indicates that
      the user will call WDRV_WINC_BSSFindNext to obtain the next BBS
      information structure.
*/

typedef bool (*WDRV_WINC_BSSFIND_NOTIFY_CALLBACK)
(
    DRV_HANDLE handle,
    uint8_t index,
    uint8_t ofTotal,
    WDRV_WINC_BSS_INFO *pBSSInfo
);

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver BSS Find Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSFindFirst
    (
        DRV_HANDLE handle,
        uint8_t channel,
        bool active,
        const WDRV_WINC_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Request a BSS scan is performed by the WINC.

  Description:
    A scan is requested on the specified channels. An optional callback can
      be provided to receive notification of the first BSS discovered.

  Precondition:
    WDRV_WINC_Initialize must have been called.
    WDRV_WINC_Open must have been called to obtain a valid handle.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_WINC_Open.
    channel          - Channel to scan, maybe WDRV_WINC_ALL_CHANNELS in
                         which case all enabled channels are scanned.
    active           - Use active vs passive scanning.
    pfNotifyCallback - Callback to receive notification of first BSS found.

  Returns:
    WDRV_WINC_STATUS_OK               - A scan was initiated.
    WDRV_WINC_STATUS_NOT_OPEN         - The driver instance is not open.
    WDRV_WINC_STATUS_REQUEST_ERROR    - The request to the WINC was rejected.
    WDRV_WINC_STATUS_INVALID_ARG      - The parameters were incorrect.
    WDRV_WINC_STATUS_SCAN_IN_PROGRESS - A scan is already in progress.

  Remarks:
    If channel is WDRV_WINC_ALL_CHANNELS then all enabled channels are
      scanned. The enabled channels can be configured using
      WDRV_WINC_BSSFindSetEnabledChannels. How the scan is performed can
      be configured using WDRV_WINC_BSSFindSetScanParameters.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindFirst
(
    DRV_HANDLE handle,
    uint8_t channel,
    bool active,
    const WDRV_WINC_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSFindNext
    (
        DRV_HANDLE handle,
        WDRV_WINC_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Request the next scan results be provided.

  Description:
    The information structure of the next BSS is requested from the WINC.

  Precondition:
    WDRV_WINC_Initialize must have been called.
    WDRV_WINC_Open must have been called to obtain a valid handle.
    WDRV_WINC_BSSFindFirst must have been called.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_WINC_Open.
    pfNotifyCallback - Callback to receive notification of next BSS found.

  Returns:
    WDRV_WINC_STATUS_OK               - The request was accepted.
    WDRV_WINC_STATUS_NOT_OPEN         - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG      - The parameters were incorrect.
    WDRV_WINC_STATUS_SCAN_IN_PROGRESS - A scan is already in progress.
    WDRV_WINC_STATUS_BSS_FIND_END     - No more results are available.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindNext
(
    DRV_HANDLE handle,
    WDRV_WINC_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSFindReset
    (
        DRV_HANDLE handle,
        WDRV_WINC_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Request the first scan results again

  Description:
    The information structure of the first BSS is requested from the WINC.

  Precondition:
    WDRV_WINC_Initialize must have been called.
    WDRV_WINC_Open must have been called to obtain a valid handle.
    WDRV_WINC_BSSFindFirst must have been called.

  Parameters:
    handle           - Client handle obtained by a call to WDRV_WINC_Open.
    pfNotifyCallback - Callback to receive notification of next BSS found.

  Returns:
    WDRV_WINC_STATUS_OK               - The request was accepted.
    WDRV_WINC_STATUS_NOT_OPEN         - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG      - The parameters were incorrect.
    WDRV_WINC_STATUS_SCAN_IN_PROGRESS - A scan is already in progress.
    WDRV_WINC_STATUS_BSS_FIND_END     - No more results are available.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindReset
(
    DRV_HANDLE handle,
    WDRV_WINC_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSFindGetInfo
    (
        DRV_HANDLE handle,
        WDRV_WINC_BSS_INFO *const pBSSInfo
    )

  Summary:
    Requests the information structure of the current BSS scan result.

  Description:
    After each call to either WDRV_WINC_BSSFindFirst or WDRV_WINC_BSSFindNext
      the driver receives a single BSS information structure which it stores.
      This function retrieves that structure.

  Precondition:
    WDRV_WINC_Initialize must have been called.
    WDRV_WINC_Open must have been called to obtain a valid handle.
    WDRV_WINC_BSSFindFirst must have been called.

  Parameters:
    handle   - Client handle obtained by a call to WDRV_WINC_Open.
    pBSSInfo - Pointer to structure to populate with the current BSS information.

  Returns:
    WDRV_WINC_STATUS_OK          - The request was accepted.
    WDRV_WINC_STATUS_NOT_OPEN    - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG - The parameters were incorrect.
    WDRV_WINC_STATUS_NO_BSS_INFO - There is no current BBS information available.

  Remarks:
    This function may be polled after calling WDRV_WINC_BSSFindFirst or
      WDRV_WINC_BSSFindNext until it returns WDRV_WINC_STATUS_OK.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindGetInfo
(
    DRV_HANDLE handle,
    WDRV_WINC_BSS_INFO *const pBSSInfo
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSFindSetScanParameters
    (
        DRV_HANDLE handle,
        uint8_t numSlots,
        uint8_t activeSlotTime,
        uint8_t passiveSlotTime,
        uint8_t numProbes
    )

  Summary:
    Configures the scan operation.

  Description:
    The scan consists of a number of slots per channel, each slot can has a
      configurable time period and number of probe requests to send.

  Precondition:
    WDRV_WINC_Initialize must have been called.
    WDRV_WINC_Open must have been called to obtain a valid handle.

  Parameters:
    handle          - Client handle obtained by a call to WDRV_WINC_Open.
    numSlots        - Number if slots (minimum is 2).
    activeSlotTime  - Time waiting for responses (10ms <= slotTime <= 250ms)
                        for active scans.
    passiveSlotTime - Time waiting for responses (10ms <= slotTime <= 1200ms)
                        for passive scans.
    numProbes       - Number of probes per slot.

  Returns:
    WDRV_WINC_STATUS_OK            - The request was accepted.
    WDRV_WINC_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG   - The parameters were incorrect.

  Remarks:
    If any parameter is zero then the configured value is unchanged.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindSetScanParameters
(
    DRV_HANDLE handle,
    uint8_t numSlots,
    uint8_t activeSlotTime,
    uint16_t passiveSlotTime,
    uint8_t numProbes
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSFindSetRSSIThreshold
    (
        DRV_HANDLE handle,
        int8_t rssiThreshold
    )

  Summary:
    Configures the scan RSSI threshold.

  Description:
    The RSSI threshold required for reconnection to an AP without scanning
    all channels first.

  Precondition:
    WDRV_WINC_Initialize must have been called.
    WDRV_WINC_Open must have been called to obtain a valid handle.

  Parameters:
    handle        - Client handle obtained by a call to WDRV_WINC_Open.
    rssiThreshold - Minimum RSSI for detection.

  Returns:
    WDRV_WINC_STATUS_OK            - The request was accepted.
    WDRV_WINC_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG   - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindSetRSSIThreshold
(
    DRV_HANDLE handle,
    int8_t rssiThreshold
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSFindSetEnabledChannels
    (
        DRV_HANDLE handle,
        WDRV_WINC_CHANNEL_MASK channelMask
    )

  Summary:
    Configures the channels which can be scanned.

  Description:
    To comply with regulatory domains certain channels must not be scanned.
      This function configures which channels are enabled to be used.

  Precondition:
    WDRV_WINC_Initialize must have been called.
    WDRV_WINC_Open must have been called to obtain a valid handle.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_WINC_Open.
    channelMask - A channel mask detailing all the enabled channels.

  Returns:
    WDRV_WINC_STATUS_OK            - The request was accepted.
    WDRV_WINC_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR - The WINC was unable to accept this
                                           request.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindSetEnabledChannels
(
    DRV_HANDLE handle,
    WDRV_WINC_CHANNEL_MASK channelMask
);

//*******************************************************************************
/*
  Function:
    uint8_t WDRV_WINC_BSSFindGetNumBSSResults(DRV_HANDLE handle)

  Summary:
    Returns the number of BSS scan results found.

  Description:
    Returns the number of BSS scan results found.

  Precondition:
    WDRV_WINC_Initialize must have been called.
    WDRV_WINC_Open must have been called to obtain a valid handle.
    WDRV_WINC_BSSFindFirst must have been called to start a scan.

  Parameters:
    handle - Client handle obtained by a call to WDRV_WINC_Open.

  Returns:
    Number of BSS scan results available. Zero indicates no results or an
      error occurred.

  Remarks:
    None.

*/

uint8_t WDRV_WINC_BSSFindGetNumBSSResults(DRV_HANDLE handle);

//*******************************************************************************
/*
  Function:
    bool WDRV_WINC_BSSFindInProgress(DRV_HANDLE handle)

  Summary:
    Indicates if a BSS scan is in progress.

  Description:
    Returns a flag indicating if a BSS scan operation is currently running.

  Precondition:
    WDRV_WINC_Initialize must have been called.
    WDRV_WINC_Open must have been called to obtain a valid handle.

  Parameters:
    handle - Client handle obtained by a call to WDRV_WINC_Open.

  Returns:
    Flag indicating if a scan is in progress. If an error occurs the result
      is false.

  Remarks:
    None.

*/

bool WDRV_WINC_BSSFindInProgress(DRV_HANDLE handle);

#endif /* _WDRV_WINC_BSSFIND_H */
