/*******************************************************************************
  WINC Wireless Driver

  File Name:
    wdrv_winc_wps.c

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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "wdrv_winc.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc_wps.h"

static bool _ValidateChecksum(uint32_t pin)
{
    uint32_t accum = 0;

    accum += 3 * ((pin / 10000000) % 10);
    accum += 1 * ((pin / 1000000) % 10);
    accum += 3 * ((pin / 100000) % 10);
    accum += 1 * ((pin / 10000) % 10);
    accum += 3 * ((pin / 1000) % 10);
    accum += 1 * ((pin / 100) % 10);
    accum += 3 * ((pin / 10) % 10);
    accum += 1 * ((pin / 1) % 10);

    return (0 == (accum % 10));
}

/****************************************************************************
 * Function:        WDRV_WINC_WPSEnrolleeDiscoveryStartPIN
 * Summary: Start WPS discovery (PIN).
 *****************************************************************************/
WDRV_WINC_STATUS WDRV_WINC_WPSEnrolleeDiscoveryStartPIN(DRV_HANDLE handle, uint32_t pin, const WDRV_WINC_WPS_DISC_CALLBACK pfWPSDiscoveryCallback)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const )handle;
    char pinStr[9];

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

    if (false == _ValidateChecksum(pin))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    sprintf(pinStr, "%08" PRIu32, pin);

    pDcpt->pfWPSDiscoveryCB = pfWPSDiscoveryCallback;

    if (M2M_SUCCESS != m2m_wifi_wps(WPS_PIN_TRIGGER, pinStr))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}

/****************************************************************************
 * Function:        WDRV_WINC_WPSEnrolleeDiscoveryStartPBC
 * Summary: Start WPS discovery (Push Button).
 *****************************************************************************/
WDRV_WINC_STATUS WDRV_WINC_WPSEnrolleeDiscoveryStartPBC(DRV_HANDLE handle, const WDRV_WINC_WPS_DISC_CALLBACK pfWPSDiscoveryCallback)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const )handle;

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

    pDcpt->pfWPSDiscoveryCB = pfWPSDiscoveryCallback;

    if (M2M_SUCCESS != m2m_wifi_wps(WPS_PBC_TRIGGER, NULL))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}

/****************************************************************************
 * Function:        WDRV_WINC_WPSEnrolleeDiscoveryStop
 * Summary: Stop WPS discovery.
 *****************************************************************************/
WDRV_WINC_STATUS WDRV_WINC_WPSEnrolleeDiscoveryStop(DRV_HANDLE handle)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const )handle;

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

    if (M2M_SUCCESS != m2m_wifi_wps_disable())
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}

//DOM-IGNORE-END
