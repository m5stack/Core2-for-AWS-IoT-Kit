/*******************************************************************************
  WINC Driver STA Implementation

  File Name:
    wdrv_winc_sta.c

  Summary:
    WINC wireless driver STA implementation.

  Description:
    WINC wireless driver STA implementation.
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
#include "wdrv_winc_sta.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver STA Implementation
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

  Remarks:
    See wdrv_winc_sta.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSConnect
(
    DRV_HANDLE handle,
    const WDRV_WINC_BSS_CONTEXT *const pBSSCtx,
    const WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
    const WDRV_WINC_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
)
{
    WDRV_WINC_DCPT *pDcpt = (WDRV_WINC_DCPT *)handle;
    int8_t result;
    uint8_t channel;
#ifdef WDRV_WINC_DEVICE_EXT_CONNECT_PARAMS
    tstrNetworkId networkID;
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

    /* Ensure the BSS context is valid. */
    if (false == WDRV_WINC_BSSCtxIsValid(pBSSCtx, false))
    {
        return WDRV_WINC_STATUS_INVALID_CONTEXT;
    }

    /* Ensure the authentication type is valid, if present. */
    if ((NULL != pAuthCtx) && (WDRV_WINC_AUTH_TYPE_OPEN != pAuthCtx->authType)
                           && (WDRV_WINC_AUTH_TYPE_WPA_PSK != pAuthCtx->authType)
                           && (WDRV_WINC_AUTH_TYPE_WEP != pAuthCtx->authType)
#ifdef WDRV_WINC_DEVICE_ENTERPRISE_CONNECT
                           && (WDRV_WINC_AUTH_TYPE_802_1X_MSCHAPV2 != pAuthCtx->authType)
                           && (WDRV_WINC_AUTH_TYPE_802_1X_TLS != pAuthCtx->authType)
#endif
                            )
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

#ifndef WDRV_WINC_NETWORK_MODE_SOCKET
    /* Ethernet mode must have the Ethernet buffer set. */
    if (false == pDcpt->isEthBufSet)
    {
        return WDRV_WINC_STATUS_NO_ETH_BUFFER;
    }
#endif

    /* Set the channel, translating the all channel identifier if needed. */
    channel = pBSSCtx->channel;

    if (WDRV_WINC_ALL_CHANNELS == channel)
    {
        channel = M2M_WIFI_CH_ALL;
    }

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    if ((false == pDcpt->useDHCP) && (0 != pDcpt->ipAddress))
    {
        /* If not using DHCP, turn off DHCP on the WINC and config the IP
           address and subnet, gateway and DNS server static addresses. */
        tstrM2MIPConfig pIPConfig;

        result = m2m_wifi_enable_dhcp(0);

        if (M2M_SUCCESS != result)
        {
            return WDRV_WINC_STATUS_CONNECT_FAIL;
        }

        pIPConfig.u32DNS        = pDcpt->dnsServerAddress;
        pIPConfig.u32Gateway    = pDcpt->gatewayAddress;
        pIPConfig.u32StaticIP   = pDcpt->ipAddress;
        pIPConfig.u32SubnetMask = pDcpt->netMask;

        result = m2m_wifi_set_static_ip(&pIPConfig);

        if (M2M_SUCCESS != result)
        {
            return WDRV_WINC_STATUS_CONNECT_FAIL;
        }

        pDcpt->useDHCP = false;
    }
    else
    {
        result = m2m_wifi_enable_dhcp(1);

        if (M2M_SUCCESS != result)
        {
            return WDRV_WINC_STATUS_CONNECT_FAIL;
        }

        pDcpt->useDHCP   = true;
        pDcpt->ipAddress = 0;
    }
#endif

#ifdef WDRV_WINC_DEVICE_EXT_CONNECT_PARAMS
    if (true == pBSSCtx->bssid.valid)
    {
        networkID.pu8Bssid = (uint8_t*)pBSSCtx->bssid.addr;
    }
    else
    {
        networkID.pu8Bssid = NULL;
    }

    networkID.pu8Ssid = (uint8_t*)pBSSCtx->ssid.name;
    networkID.u8SsidLen = pBSSCtx->ssid.length;
    networkID.enuChannel = pBSSCtx->channel;
#endif

    if ((NULL == pAuthCtx) || (WDRV_WINC_AUTH_TYPE_OPEN == pAuthCtx->authType))
    {
        /* Connect using Open authentication. */
#ifdef WDRV_WINC_DEVICE_EXT_CONNECT_PARAMS
        result = m2m_wifi_connect_open(WIFI_CRED_SAVE_ENCRYPTED, &networkID);
#else
        result = m2m_wifi_connect((char*)pBSSCtx->ssid.name, pBSSCtx->ssid.length,
                                    WDRV_WINC_AUTH_TYPE_OPEN, NULL, channel);
#endif
    }
    else if (WDRV_WINC_AUTH_TYPE_WPA_PSK == pAuthCtx->authType)
    {
        /* Connect using WPA authentication. */
#ifdef WDRV_WINC_DEVICE_EXT_CONNECT_PARAMS
        tstrAuthPsk pskParams;
        uint8_t pskLength;

        pskLength = pAuthCtx->authInfo.WPAPerPSK.size;

        if ((M2M_MAX_PSK_LEN-1) == pskLength)
        {
            pskParams.pu8Psk          = (uint8_t*)pAuthCtx->authInfo.WPAPerPSK.key;
            pskParams.pu8Passphrase   = NULL;
            pskParams.u8PassphraseLen = 0;
        }
        else
        {
            pskParams.pu8Psk          = NULL;
            pskParams.pu8Passphrase   = (uint8_t*)pAuthCtx->authInfo.WPAPerPSK.key;
            pskParams.u8PassphraseLen = pskLength;
        }

        result = m2m_wifi_connect_psk(WIFI_CRED_SAVE_ENCRYPTED, &networkID, &pskParams);
#else
        result = m2m_wifi_connect((char*)pBSSCtx->ssid.name, pBSSCtx->ssid.length,
                                    pAuthCtx->authType, (void*)&pAuthCtx->authInfo.WPAPerPSK.key, channel);
#endif
    }
    else if (WDRV_WINC_AUTH_TYPE_WEP == pAuthCtx->authType)
    {
        /* Connect using WEP authentication. */
#ifdef WDRV_WINC_DEVICE_EXT_CONNECT_PARAMS
        tstrAuthWep wepParams;

        wepParams.pu8WepKey = (uint8_t*)pAuthCtx->authInfo.WEP.key;
        wepParams.u8KeySz   = pAuthCtx->authInfo.WEP.size;
        wepParams.u8KeyIndx = pAuthCtx->authInfo.WEP.idx;

        result = m2m_wifi_connect_wep(WIFI_CRED_SAVE_ENCRYPTED, &networkID, &wepParams);
#else
        tstrM2mWifiWepParams wepParams;

        /* Copy WEP key, index and size into M2M structure. */
        wepParams.u8KeyIndx = pAuthCtx->authInfo.WEP.idx;
        wepParams.u8KeySz   = pAuthCtx->authInfo.WEP.size + 1;
        memcpy(wepParams.au8WepKey, pAuthCtx->authInfo.WEP.key, pAuthCtx->authInfo.WEP.size);
        wepParams.au8WepKey[pAuthCtx->authInfo.WEP.size] = '\0';

        result = m2m_wifi_connect((char*)pBSSCtx->ssid.name, pBSSCtx->ssid.length,
                                    pAuthCtx->authType, (void*)&wepParams, channel);
#endif
    }
#ifdef WDRV_WINC_DEVICE_ENTERPRISE_CONNECT
    else if (WDRV_WINC_AUTH_TYPE_802_1X_MSCHAPV2 == pAuthCtx->authType)
    {
        /* Connect using WPA(2) Enterprise authentication using MS-CHAPv2. */
#ifdef WDRV_WINC_DEVICE_EXT_CONNECT_PARAMS
        tstrAuth1xMschap2 ent1XParams;

        if (true == pAuthCtx->authInfo.WPAEntMSCHAPv2.domainIsFirst)
        {
            /* format must be domain\username. */
            ent1XParams.bPrependDomain = true;
            ent1XParams.pu8Domain = (uint8_t*)pAuthCtx->authInfo.WPAEntMSCHAPv2.domainUserName;
            ent1XParams.pu8UserName = (uint8_t*)(strchr(pAuthCtx->authInfo.WPAEntMSCHAPv2.domainUserName, '\\') + 1);
            ent1XParams.u16DomainLen = ent1XParams.pu8UserName - ent1XParams.pu8Domain;
            ent1XParams.u16UserNameLen = strlen((const char*)ent1XParams.pu8UserName);
        }
        else
        {
            /* format must be either username or username@domain. */
            ent1XParams.bPrependDomain = false;
            ent1XParams.pu8Domain = (uint8_t*)strchr(pAuthCtx->authInfo.WPAEntMSCHAPv2.domainUserName, '@');
            ent1XParams.pu8UserName = (uint8_t*)pAuthCtx->authInfo.WPAEntMSCHAPv2.domainUserName;

            if (NULL == ent1XParams.pu8Domain)
            {
                ent1XParams.u16DomainLen = 0;
                ent1XParams.u16UserNameLen = strlen((const char*)ent1XParams.pu8UserName);
            }
            else
            {
                ent1XParams.u16DomainLen = strlen((const char*)ent1XParams.pu8Domain);
                ent1XParams.u16UserNameLen = ent1XParams.pu8Domain - ent1XParams.pu8UserName;
            }
        }

        ent1XParams.pu8Password = (uint8_t*)pAuthCtx->authInfo.WPAEntMSCHAPv2.password;
        ent1XParams.u16PasswordLen = pAuthCtx->authInfo.WPAEntMSCHAPv2.passwordLength;

        ent1XParams.bUnencryptedUserName = pAuthCtx->authInfo.WPAEntMSCHAPv2.visibleUserName;

        result = m2m_wifi_connect_1x_mschap2(WIFI_CRED_SAVE_ENCRYPTED, &networkID, &ent1XParams);
#else
        result = WDRV_WINC_STATUS_CONNECT_FAIL;
#endif
    }
    else if (WDRV_WINC_AUTH_TYPE_802_1X_TLS == pAuthCtx->authType)
    {
        /* Connect using WPA(2) Enterprise authentication using TLS. */
#ifdef WDRV_WINC_DEVICE_EXT_CONNECT_PARAMS
        tstrAuth1xTls ent1XParams;

        if (true == pAuthCtx->authInfo.WPAEntTLS.domainIsFirst)
        {
            /* format must be domain\username. */
            ent1XParams.bPrependDomain = true;
            ent1XParams.pu8Domain = (uint8_t*)pAuthCtx->authInfo.WPAEntTLS.domainUserName;
            ent1XParams.pu8UserName = (uint8_t*)(strchr(pAuthCtx->authInfo.WPAEntTLS.domainUserName, '\\') + 1);
            ent1XParams.u16DomainLen = ent1XParams.pu8UserName - ent1XParams.pu8Domain;
            ent1XParams.u16UserNameLen = strlen((const char*)ent1XParams.pu8UserName);
        }
        else
        {
            /* format must be either username or username@domain. */
            ent1XParams.bPrependDomain = false;
            ent1XParams.pu8Domain = (uint8_t*)strchr(pAuthCtx->authInfo.WPAEntTLS.domainUserName, '@');
            ent1XParams.pu8UserName = (uint8_t*)pAuthCtx->authInfo.WPAEntTLS.domainUserName;

            if (NULL == ent1XParams.pu8Domain)
            {
                ent1XParams.u16DomainLen = 0;
                ent1XParams.u16UserNameLen = strlen((const char*)ent1XParams.pu8UserName);
            }
            else
            {
                ent1XParams.u16DomainLen = strlen((const char*)ent1XParams.pu8Domain);
                ent1XParams.u16UserNameLen = ent1XParams.pu8Domain - ent1XParams.pu8UserName;
            }
        }

        ent1XParams.pu8PrivateKey_Mod = (uint8_t*)pAuthCtx->authInfo.WPAEntTLS.pRSAPrivateKeyModulus;
        ent1XParams.pu8PrivateKey_Exp = (uint8_t*)pAuthCtx->authInfo.WPAEntTLS.pRSAPrivateKeyExponent;
        ent1XParams.pu8Certificate    = (uint8_t*)pAuthCtx->authInfo.WPAEntTLS.pRSACertificate;
        ent1XParams.u16PrivateKeyLen  = pAuthCtx->authInfo.WPAEntTLS.RSAPrivateKeyModulusLength;
        ent1XParams.u16CertificateLen = pAuthCtx->authInfo.WPAEntTLS.RSACertificateLength;

        ent1XParams.bUnencryptedUserName = pAuthCtx->authInfo.WPAEntTLS.visibleUserName;

        result = m2m_wifi_connect_1x_tls(WIFI_CRED_SAVE_ENCRYPTED, &networkID, &ent1XParams);
#else
        result = WDRV_WINC_STATUS_CONNECT_FAIL;
#endif // #ifdef WDRV_WINC_DEVICE_EXT_CONNECT_PARAMS
    }
#endif // #ifdef WDRV_WINC_DEVICE_ENTERPRISE_CONNECT
    else
    {
        /* Unknown authentication method. */
        return WDRV_WINC_STATUS_CONNECT_FAIL;
    }

    if (M2M_SUCCESS != result)
    {
        return WDRV_WINC_STATUS_CONNECT_FAIL;
    }

    pDcpt->pfConnectNotifyCB   = pfNotifyCallback;
    pDcpt->isConnected         = false;
#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    pDcpt->haveIPAddress       = false;
#endif

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc_sta.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSReconnect
(
    DRV_HANDLE handle,
    WDRV_WINC_BSSCON_NOTIFY_CALLBACK pfNotifyCallback
)
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

    /* Ensure WINC is not connected. */
    if (true == pDcpt->isConnected)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

#ifndef WDRV_WINC_NETWORK_MODE_SOCKET
    /* For Ethernet mode ensure Ethernet buffer is allocated. */
    if (false == pDcpt->isEthBufSet)
    {
        return WDRV_WINC_STATUS_NO_ETH_BUFFER;
    }
#endif

    /* Attempt reconnection. */
    if (M2M_SUCCESS != m2m_wifi_default_connect())
    {
        return WDRV_WINC_STATUS_CONNECT_FAIL;
    }

    pDcpt->pfConnectNotifyCB   = pfNotifyCallback;
    pDcpt->isConnected         = false;
#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    pDcpt->haveIPAddress       = false;
#endif

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_BSSDisconnect(DRV_HANDLE handle)

  Summary:
    Disconnects from a BSS.

  Description:
    Disconnects from an existing BSS.

  Remarks:
    See wdrv_winc_sta.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_BSSDisconnect(DRV_HANDLE handle)
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

    /* Ensure WINC is not connected. */
    if (false == pDcpt->isConnected)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Disconnect WINC. */
    if (M2M_SUCCESS != m2m_wifi_disconnect())
    {
        return WDRV_WINC_STATUS_DISCONNECT_FAIL;
    }

    pDcpt->isConnected    = false;
#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    pDcpt->haveIPAddress  = false;
#endif
    pDcpt->assocInfoValid = false;

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc_sta.h for usage information.

*/

#ifdef WDRV_WINC_DEVICE_BSS_ROAMING
WDRV_WINC_STATUS WDRV_WINC_BSSRoamingConfigure
(
    DRV_HANDLE handle,
    WDRV_WINC_BSS_ROAMING_CFG roamingCfg
)
{
    WDRV_WINC_DCPT *pDcpt = (WDRV_WINC_DCPT *)handle;
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

    /* Ensure WINC is not connected. */
    if (false == pDcpt->isConnected)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    result = M2M_ERR_FAIL;

    switch (roamingCfg)
    {
        case WDRV_WINC_BSS_ROAMING_CFG_ON:
        {
            /* Turn on roaming without issuing a DHCP renew. */
            result = m2m_wifi_enable_roaming(false);
            break;
        }

        case WDRV_WINC_BSS_ROAMING_CFG_ON_IP_RENEW:
        {
            /* Turn on roaming, issue a DHCP renew. */
            result = m2m_wifi_enable_roaming(true);
            break;
        }

        case WDRV_WINC_BSS_ROAMING_CFG_OFF:
        default:
        {
            /* Turn off roaming. */
            result = m2m_wifi_disable_roaming();
            break;
        }
    }

    if (M2M_SUCCESS != result)
    {
        return WDRV_WINC_STATUS_CONNECT_FAIL;
    }

    return WDRV_WINC_STATUS_OK;
}
#endif