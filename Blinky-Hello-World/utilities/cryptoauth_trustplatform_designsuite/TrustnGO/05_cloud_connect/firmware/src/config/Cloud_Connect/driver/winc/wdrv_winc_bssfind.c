/*******************************************************************************
  WINC Wireless Driver

  File Name:
    wdrv_winc_bssfind.c

  Summary:
    WINC wireless driver.

  Description:
    WINC wireless driver.
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

#include <stdint.h>
#include <string.h>

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "wdrv_winc.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc_bssfind.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver BSS Find Implementations
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

  Remarks:
    See wdrv_winc_bssfind.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindFirst
(
    DRV_HANDLE handle,
    uint8_t channel,
    bool active,
    const WDRV_WINC_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;
    tstrM2MScanOption scanOptions;
    int8_t result;

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

    /* Convert public API representation of all channels to
       internal representation for the M2M driver. */
    if (WDRV_WINC_ALL_CHANNELS == channel)
    {
        channel = M2M_WIFI_CH_ALL;
    }

    /* Check if the scan parameters have been updated from
       the defaults. */
    if (false == pDcpt->scanParamDefault)
    {
        scanOptions.u8NumOfSlot         = pDcpt->scanNumSlots;
        scanOptions.u8SlotTime          = pDcpt->scanActiveScanTime;
        scanOptions.u8ProbesPerSlot     = pDcpt->scanNumProbes;
        scanOptions.s8RssiThresh        = pDcpt->scanRSSIThreshold;
#ifdef WDRV_WINC_DEVICE_WINC3400
        scanOptions.u16PassiveScanTime  = pDcpt->scanPassiveScanTime;
#endif

        /* Scan parameters need to be updated in the WINC device. */
        if (M2M_ERR_FAIL == m2m_wifi_set_scan_options(&scanOptions))
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }
    }

    if (true == active)
    {
        /* Request active scan of selected channel (or all channels). */
        result = m2m_wifi_request_scan(channel);
    }
    else
    {
        /* Request passive scan of selected channel (or all channels). */
#if defined(WDRV_WINC_DEVICE_WINC1500)
        result = m2m_wifi_request_scan_passive(channel, pDcpt->scanPassiveScanTime);
#elif defined(WDRV_WINC_DEVICE_WINC3400)
        result = m2m_wifi_request_scan_passive(channel);
#endif
    }

    /* Handle response, from scan request. */
    switch (result)
    {
        case M2M_ERR_INVALID_ARG:
        {
            /* There was a problem with the request channel. */
            return WDRV_WINC_STATUS_INVALID_ARG;
        }

        case M2M_ERR_SCAN_IN_PROGRESS:
        {
            /* A scan is already in progress. */
            return WDRV_WINC_STATUS_SCAN_IN_PROGRESS;
        }

        case M2M_SUCCESS:
        default:
        {
            break;
        }
    }

    /* Update state to reflect a new scan has started. */
    pDcpt->scanInProgress     = true;
    pDcpt->scanIndex          = 0;
    pDcpt->isBSSScanInfoValid = false;
    pDcpt->scanNumScanResults = 0;
    pDcpt->pfBSSFindNotifyCB  = pfNotifyCallback;

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc_bssfind.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindNext
(
    DRV_HANDLE handle,
    WDRV_WINC_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
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

    /* Cannot request results while a scan is in progress. */
    if (true == pDcpt->scanInProgress)
    {
        return WDRV_WINC_STATUS_SCAN_IN_PROGRESS;
    }

    /* Check if the request would exceed the number of results
       available, signal find operation end if so. */
    pDcpt->scanIndex++;

    if (pDcpt->scanIndex >= m2m_wifi_get_num_ap_found())
    {
        pDcpt->scanIndex--;

        return WDRV_WINC_STATUS_BSS_FIND_END;
    }

    /* Request the next BBS results from the WINC device. */
    m2m_wifi_req_scan_result(pDcpt->scanIndex);

    /* Invalidate the BSS scan cache and store callback supplied. */
    pDcpt->isBSSScanInfoValid = false;
    pDcpt->pfBSSFindNotifyCB  = pfNotifyCallback;

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc_bssfind.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindReset
(
    DRV_HANDLE handle,
    WDRV_WINC_BSSFIND_NOTIFY_CALLBACK pfNotifyCallback
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

    /* Cannot reset the find operation while a scan is in progress. */
    if (true == pDcpt->scanInProgress)
    {
        return WDRV_WINC_STATUS_SCAN_IN_PROGRESS;
    }

    /* Check if there are any results available. */
    if (0 == m2m_wifi_get_num_ap_found())
    {
        return WDRV_WINC_STATUS_BSS_FIND_END;
    }

    /* Reset to first BSS results and request them from the WINC device. */
    pDcpt->scanIndex = 0;

    m2m_wifi_req_scan_result(pDcpt->scanIndex);

    /* Invalidate the BSS scan cache and store callback supplied. */
    pDcpt->isBSSScanInfoValid = false;
    pDcpt->pfBSSFindNotifyCB  = pfNotifyCallback;

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc_bssfind.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindGetInfo
(
    DRV_HANDLE handle,
    WDRV_WINC_BSS_INFO *const pBSSInfo
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pBSSInfo))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    // Check if the BSS scan cache is valid. */
    if (false == pDcpt->isBSSScanInfoValid)
    {
        return WDRV_WINC_STATUS_NO_BSS_INFO;
    }

    /* Copy BSS scan cache to user supplied buffer. */
    memcpy(pBSSInfo, &pDcpt->lastBSSScanInfo, sizeof(WDRV_WINC_BSS_INFO));

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc_bssfind.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindSetScanParameters
(
    DRV_HANDLE handle,
    uint8_t numSlots,
    uint8_t activeSlotTime,
    uint16_t passiveSlotTime,
    uint8_t numProbes
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

    /* Check each scan parameter and mark when they change from the defaults
       so that they can be loaded into the WINC. Passive Scan Time is handled
       differently on WINC1500 vs WINC3400. */

    /* Check for update to Active Scan Time. */
    if ((0 != activeSlotTime) && (pDcpt->scanActiveScanTime != activeSlotTime))
    {
        pDcpt->scanActiveScanTime = activeSlotTime;
        pDcpt->scanParamDefault = false;
    }

    /* Check for update to Passive Scan Time. */
    if ((0 != passiveSlotTime) && (pDcpt->scanPassiveScanTime != passiveSlotTime))
    {
        pDcpt->scanPassiveScanTime = passiveSlotTime;
#ifdef WDRV_WINC_DEVICE_WINC3400
        pDcpt->scanParamDefault = false;
#endif
    }

    /* Check for update to Number of Slots. */
    if ((0 != numSlots) && (pDcpt->scanNumSlots != numSlots))
    {
        pDcpt->scanNumSlots = numSlots;
        pDcpt->scanParamDefault = false;
    }

    /* Check for update to Number of Probes. */
    if ((0 != numProbes) && (pDcpt->scanNumProbes != numProbes))
    {
        pDcpt->scanNumProbes = numProbes;
        pDcpt->scanParamDefault = false;
    }

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc_bssfind.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindSetRSSIThreshold
(
    DRV_HANDLE handle,
    int8_t rssiThreshold
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

    /* Update RSSI Threshold and tag the defaults have changed to force new
       parameters to be loaded into WINC device. */
    pDcpt->scanRSSIThreshold = rssiThreshold;
    pDcpt->scanParamDefault  = false;

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc_bssfind.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSFindSetEnabledChannels
(
    DRV_HANDLE handle,
    WDRV_WINC_CHANNEL_MASK channelMask
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

    /* Update the selected channel mask. */
    if (M2M_ERR_FAIL == m2m_wifi_set_scan_region(channelMask))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    uint8_t WDRV_WINC_BSSFindGetNumBSSResults(DRV_HANDLE handle)

  Summary:
    Returns the number of BSS scan results found.

  Description:
    Returns the number of BSS scan results found.

  Remarks:
    See wdrv_winc_bssfind.h for usage information.

*/

uint8_t WDRV_WINC_BSSFindGetNumBSSResults(DRV_HANDLE handle)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid and the instance is open.
       Ensure a scan is not already in progress. */
    if ((NULL == pDcpt) || (false == pDcpt->isOpen)
        || (true == pDcpt->scanInProgress))
    {
        return 0;
    }

    /* Return the number of BSSs found. */
    return m2m_wifi_get_num_ap_found();
}

//*******************************************************************************
/*
  Function:
    bool WDRV_WINC_BSSFindInProgress(DRV_HANDLE handle)

  Summary:
    Indicates if a BSS scan is in progress.

  Description:
    Returns a flag indicating if a BSS scan operation is currently running.

  Remarks:
    See wdrv_winc_bssfind.h for usage information.

*/

bool WDRV_WINC_BSSFindInProgress(DRV_HANDLE handle)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid and the instance is open. */
    if ((NULL == pDcpt) || (false == pDcpt->isOpen))
    {
        return false;
    }

    return pDcpt->scanInProgress;
}
