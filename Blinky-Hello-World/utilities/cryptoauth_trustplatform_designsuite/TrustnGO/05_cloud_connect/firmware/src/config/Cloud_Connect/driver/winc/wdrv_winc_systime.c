/*******************************************************************************
  WINC Wireless Driver

  File Name:
    wdrv_winc_systime.c

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
#include <time.h>

#include "wdrv_winc.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc_systime.h"

/****************************************************************************
 * Function:        WDRV_WINC_LocalTimeToUTC
 * Summary: Convert local system time to UTC time.
 *****************************************************************************/
uint32_t WDRV_WINC_LocalTimeToUTC(const tstrSystemTime *const pSysTime)
{
    time_t timeUTC;
    struct tm t;

    if (NULL == pSysTime)
    {
        return 0;
    }

    t.tm_sec  = pSysTime->u8Second;
    t.tm_min  = pSysTime->u8Minute;
    t.tm_hour = pSysTime->u8Hour;
    t.tm_mday = pSysTime->u8Day;
    t.tm_mon  = pSysTime->u8Month-1;
    t.tm_year = pSysTime->u16Year-1900;

    timeUTC = mktime(&t);

    if (-1 == timeUTC)
        return 0;

    return (uint32_t)timeUTC;
}

/****************************************************************************
 * Function:        WDRV_WINC_UTCToLocalTime
 * Summary: Convert UTC time to local system time.
 *****************************************************************************/
bool WDRV_WINC_UTCToLocalTime(uint32_t timeUTC, tstrSystemTime *const pSysTime)
{
    struct tm *ptm;
    time_t utc;

    if (NULL == pSysTime)
    {
        return false;
    }

    utc = timeUTC;

    ptm = gmtime(&utc);

    pSysTime->u8Second = ptm->tm_sec;
    pSysTime->u8Minute = ptm->tm_min;
    pSysTime->u8Hour   = ptm->tm_hour;
    pSysTime->u8Day    = ptm->tm_mday;
    pSysTime->u8Month  = ptm->tm_mon+1;
    pSysTime->u16Year  = ptm->tm_year+1900;

    return true;
}

/****************************************************************************
 * Function:        WDRV_WINC_SystemTimeSNTPClientEnable
 * Summary: Enable the SNTP client to set system time.
 *****************************************************************************/
WDRV_WINC_STATUS WDRV_WINC_SystemTimeSNTPClientEnable(DRV_HANDLE handle,
                                const char *pServerName, bool allowDHCPOverride)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const )handle;
#if defined(WDRV_WINC_DEVICE_CONF_NTP_SERVER) && defined(WDRV_WINC_NETWORK_MODE_SOCKET)
    tenuSNTPUseDHCP useDHCP;
#endif

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

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    if (M2M_SUCCESS != m2m_wifi_enable_sntp(1))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

#ifdef WDRV_WINC_DEVICE_CONF_NTP_SERVER
    if (true == allowDHCPOverride)
    {
        useDHCP = SNTP_ENABLE_DHCP;
    }
    else
    {
        useDHCP = SNTP_DISABLE_DHCP;
    }

    if (NULL != pServerName)
    {
        if (M2M_SUCCESS != m2m_wifi_configure_sntp((uint8_t*)pServerName,
                                                strlen(pServerName), useDHCP))
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }
    }
    else
    {
        uint8_t noNTPSever[1];

        if (M2M_SUCCESS != m2m_wifi_configure_sntp(noNTPSever, 0, useDHCP))
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }
    }
#endif

    return WDRV_WINC_STATUS_OK;
#else
    return WDRV_WINC_STATUS_REQUEST_ERROR;
#endif
}

/****************************************************************************
 * Function:        WDRV_WINC_SystemTimeSNTPClientDisable
 * Summary: Disable the SNTP client.
 *****************************************************************************/
WDRV_WINC_STATUS WDRV_WINC_SystemTimeSNTPClientDisable(DRV_HANDLE handle)
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

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    if (M2M_SUCCESS != m2m_wifi_enable_sntp(0))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
#else
    return WDRV_WINC_STATUS_REQUEST_ERROR;
#endif
}

/****************************************************************************
 * Function:        WDRV_WINC_SystemTimeSetCurrent
 * Summary: Sets the current system time on the WINC.
 *****************************************************************************/
WDRV_WINC_STATUS WDRV_WINC_SystemTimeSetCurrent(DRV_HANDLE handle, uint32_t curTime)
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

    if (M2M_SUCCESS != m2m_wifi_set_system_time(curTime + 2208988800UL))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}

/****************************************************************************
 * Function:        WDRV_WINC_SystemTimeGetCurrent
 * Summary: Requests the current system time from the WINC.
 *****************************************************************************/
WDRV_WINC_STATUS WDRV_WINC_SystemTimeGetCurrent(DRV_HANDLE handle,
                            const WDRV_WINC_SYSTIME_CURRENT_CALLBACK pfGetCurrentCallback)
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

    pDcpt->pfSystemTimeGetCurrentCB = pfGetCurrentCallback;

    if (M2M_SUCCESS != m2m_wifi_get_system_time())
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}

//DOM-IGNORE-END
