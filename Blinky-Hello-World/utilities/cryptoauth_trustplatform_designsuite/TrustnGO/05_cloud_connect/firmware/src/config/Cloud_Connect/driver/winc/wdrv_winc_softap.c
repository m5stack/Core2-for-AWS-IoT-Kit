/*******************************************************************************
  WINC Driver Soft-AP Implementation

  File Name:
    wdrv_winc_softap.c

  Summary:
    WINC wireless driver Soft-AP implementation.

  Description:
    Provides an interface to create and manage a Soft-AP.
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

#include <stdint.h>
#include <string.h>

#include "wdrv_winc.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc_softap.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Soft-AP Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_APStart
    (
        DRV_HANDLE handle,
        const WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
        const WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
        const WDRV_WINC_HTTP_PROV_CONTEXT *const pHTTPProvCtx,
        const WDRV_WINC_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
    )

  Summary:
    Starts an instance of Soft-AP.

  Description:
    Using the defined BSS and authentication contexts with an optional HTTP
      provisioning context (socket mode only) this function creates and starts
      a Soft-AP instance.

  Remarks:
    See wdrv_winc_softap.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_APStart
(
    DRV_HANDLE handle,
    const WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
    const WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
    const WDRV_WINC_HTTP_PROV_CONTEXT *const pHTTPProvCtx,
    const WDRV_WINC_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
)
{
    WDRV_WINC_DCPT *pDcpt = (WDRV_WINC_DCPT *)handle;
#ifdef WDRV_WINC_DEVICE_SOFT_AP_EXT
    tstrM2MAPModeConfig apCfg;
    tstrM2MAPConfig *pAPCfg = &apCfg.strApConfig;
#else
    tstrM2MAPConfig apCfg;
    tstrM2MAPConfig *pAPCfg = &apCfg;
#endif

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pBSSCtx))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

#ifndef WDRV_WINC_NETWORK_MODE_SOCKET
    /* For Ethernet mode ensure Ethernet buffer is set. */
    if (false == pDcpt->isEthBufSet)
    {
        return WDRV_WINC_STATUS_NO_ETH_BUFFER;
    }
#endif

    /* Validate BSS context. */
    if (false == WDRV_WINC_BSSCtxIsValid(pBSSCtx, false))
    {
        return WDRV_WINC_STATUS_INVALID_CONTEXT;
    }

    /* Validate authentication type if context is present. */
    if ((NULL != pAuthCtx) && (WDRV_WINC_AUTH_TYPE_OPEN != pAuthCtx->authType)
                           && (WDRV_WINC_AUTH_TYPE_WEP != pAuthCtx->authType)
#ifdef WDRV_WINC_DEVICE_WPA_SOFT_AP
                           && (WDRV_WINC_AUTH_TYPE_WPA_PSK != pAuthCtx->authType)
#endif
                            )
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Copy SSID and channel. */
    memcpy(&pAPCfg->au8SSID, pBSSCtx->ssid.name, pBSSCtx->ssid.length);
    pAPCfg->au8SSID[pBSSCtx->ssid.length] = '\0';
    pAPCfg->u8ListenChannel = pBSSCtx->channel;

    if ((NULL == pAuthCtx) || (WDRV_WINC_AUTH_TYPE_OPEN == pAuthCtx->authType))
    {
        /* If authentication context is not specified or Open authentication is
           request then configure for Open authentication. */
        pAPCfg->u8SecType = M2M_WIFI_SEC_OPEN;

        /* Clear key parameters in configuration. */
        pAPCfg->u8KeyIndx = 0;
        pAPCfg->u8KeySz   = 0;
        pAPCfg->au8WepKey[0] = '\0';
    }
#ifdef WDRV_WINC_DEVICE_WPA_SOFT_AP
    else if (WDRV_WINC_AUTH_TYPE_WPA_PSK == pAuthCtx->authType)
    {
        /* If WPA2 authentication is requested copy key. */
        pAPCfg->u8SecType = M2M_WIFI_SEC_WPA_PSK;

        pAPCfg->u8KeySz = pAuthCtx->authInfo.WPAPerPSK.size;
        memcpy(pAPCfg->au8Key, pAuthCtx->authInfo.WPAPerPSK.key, pAPCfg->u8KeySz);
    }
#endif
    else if (WDRV_WINC_AUTH_TYPE_WEP == pAuthCtx->authType)
    {
        /* If WEP authentication is requested validate the key index and size. */
        pAPCfg->u8SecType = M2M_WIFI_SEC_WEP;

        if ((pAuthCtx->authInfo.WEP.idx > 4) ||
            (   (WEP_40_KEY_STRING_SIZE != pAuthCtx->authInfo.WEP.size) &&
                (WEP_104_KEY_STRING_SIZE != pAuthCtx->authInfo.WEP.size)))
        {
            return WDRV_WINC_STATUS_INVALID_ARG;
        }

        /* Copy WEP key, index and size into configuration. */
        pAPCfg->u8KeyIndx = pAuthCtx->authInfo.WEP.idx;
        pAPCfg->u8KeySz   = pAuthCtx->authInfo.WEP.size;
        memcpy(&pAPCfg->au8WepKey, &pAuthCtx->authInfo.WEP.key, pAuthCtx->authInfo.WEP.size);
        pAPCfg->au8WepKey[pAuthCtx->authInfo.WEP.size] = '\0';
    }
    else
    {
        /* WPA and Enterprise are not supported. */
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Set DHCP server address, for Ethernet mode a value is still required
       even though not used. */
#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    pAPCfg->au8DHCPServerIP[0] = pDcpt->dhcpServerAddress & 0xff;
    pAPCfg->au8DHCPServerIP[1] = (pDcpt->dhcpServerAddress >> 8) & 0xff;
    pAPCfg->au8DHCPServerIP[2] = (pDcpt->dhcpServerAddress >> 16) & 0xff;
    pAPCfg->au8DHCPServerIP[3] = (pDcpt->dhcpServerAddress >> 24) & 0xff;

    pDcpt->haveIPAddress = true;
    pDcpt->ipAddress     = pDcpt->dhcpServerAddress;
#else
    pAPCfg->au8DHCPServerIP[0] = 192;
    pAPCfg->au8DHCPServerIP[1] = 168;
    pAPCfg->au8DHCPServerIP[2] = 1;
    pAPCfg->au8DHCPServerIP[3] = 1;
#endif
#ifdef WDRV_WINC_DEVICE_SOFT_AP_EXT
    apCfg.strApConfigExt.au8DNSServerIP[0] = pDcpt->dnsServerAddress & 0xff;
    apCfg.strApConfigExt.au8DNSServerIP[1] = (pDcpt->dnsServerAddress >> 8) & 0xff;
    apCfg.strApConfigExt.au8DNSServerIP[2] = (pDcpt->dnsServerAddress >> 16) & 0xff;
    apCfg.strApConfigExt.au8DNSServerIP[3] = (pDcpt->dnsServerAddress >> 24) & 0xff;

    apCfg.strApConfigExt.au8DefRouterIP[0] = pDcpt->gatewayAddress & 0xff;
    apCfg.strApConfigExt.au8DefRouterIP[1] = (pDcpt->gatewayAddress >> 8) & 0xff;
    apCfg.strApConfigExt.au8DefRouterIP[2] = (pDcpt->gatewayAddress >> 16) & 0xff;
    apCfg.strApConfigExt.au8DefRouterIP[3] = (pDcpt->gatewayAddress >> 24) & 0xff;

    apCfg.strApConfigExt.au8SubnetMask[0] = pDcpt->netMask & 0xff;
    apCfg.strApConfigExt.au8SubnetMask[1] = (pDcpt->netMask >> 8) & 0xff;
    apCfg.strApConfigExt.au8SubnetMask[2] = (pDcpt->netMask >> 16) & 0xff;
    apCfg.strApConfigExt.au8SubnetMask[3] = (pDcpt->netMask >> 24) & 0xff;
#endif

    pAPCfg->u8SsidHide = pBSSCtx->cloaked;

    pDcpt->pfConnectNotifyCB = pfNotifyCallback;

    if (NULL == pHTTPProvCtx)
    {
        /* For just a plain Soft-AP start the AP service. */
#ifdef WDRV_WINC_DEVICE_SOFT_AP_EXT
        if (M2M_SUCCESS != m2m_wifi_enable_ap_ext(&apCfg))
#else
        if (M2M_SUCCESS != m2m_wifi_enable_ap(&apCfg))
#endif
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }
    }
#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    else
    {
        /* For Socket mode only support starting a HTTP provisioning Soft-AP. */
#ifdef WDRV_WINC_DEVICE_SOFT_AP_EXT
        if (M2M_SUCCESS != m2m_wifi_start_provision_mode_ext(&apCfg,
                    (char*)pHTTPProvCtx->domainName, pHTTPProvCtx->wildcardURL))
#else
        if (M2M_SUCCESS != m2m_wifi_start_provision_mode(&apCfg,
                    (char*)pHTTPProvCtx->domainName, pHTTPProvCtx->wildcardURL))
#endif
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }

        pDcpt->pfProvConnectInfoCB = pHTTPProvCtx->pfProvConnectInfoCB;
        pDcpt->isProvisioning = true;
    }
#endif

    pDcpt->isAP = true;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_APStop(DRV_HANDLE handle)

  Summary:
    Stops an instance of Soft-AP.

  Description:
    Stops an instance of Soft-AP.

  Remarks:
    See wdrv_winc_softap.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_APStop(DRV_HANDLE handle)
{
    WDRV_WINC_DCPT *pDcpt = (WDRV_WINC_DCPT *)handle;

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

    /* Ensure operation mode is really Soft-AP. */
    if (false == pDcpt->isAP)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    if (true == pDcpt->isProvisioning)
    {
        /* Stop a provisioning Soft-AP. */
        if (M2M_SUCCESS != m2m_wifi_stop_provision_mode())
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }
    }
    else
    {
        /* Stop a plain Soft-AP. */
        if (M2M_SUCCESS != m2m_wifi_disable_ap())
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }
    }

    /* Clear AP state. */
    pDcpt->isAP           = false;
    pDcpt->isProvisioning = false;
    pDcpt->assocInfoValid = false;
#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    pDcpt->haveIPAddress  = false;
    pDcpt->ipAddress      = 0;
#endif    

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_APSetCustIE
    (
        DRV_HANDLE handle,
        const WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx
    )

  Summary:
    Configures the custom IE.

  Description:
    Soft-AP beacons may contain a application provided custom IE. This function
      associates an custom IE store context with the Soft-AP instance.

  Remarks:
    See wdrv_winc_softap.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_APSetCustIE
(
    DRV_HANDLE handle,
    const WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx
)
{
    WDRV_WINC_DCPT *pDcpt = (WDRV_WINC_DCPT *)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pCustIECtx))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure operation mode is really Soft-AP. */
    if (false == pDcpt->isAP)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Set the custom IE. */
    if (M2M_SUCCESS != m2m_wifi_set_cust_InfoElement((uint8_t*)&pCustIECtx->curLength))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}
