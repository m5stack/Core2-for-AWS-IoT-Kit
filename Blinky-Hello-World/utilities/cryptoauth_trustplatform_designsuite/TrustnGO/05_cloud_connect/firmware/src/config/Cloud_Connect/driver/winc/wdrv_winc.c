/*******************************************************************************
  WINC Wireless Driver

  File Name:
    wdrv_winc.c

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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>

#include "wdrv_winc.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc_gpio.h"
#include "wdrv_winc_spi.h"
#include "m2m_wifi.h"
#ifdef WDRV_WINC_DEVICE_WINC3400
#include "m2m_flash.h"
#endif
#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
#include "m2m_ota.h"
#include "socket.h"
#include "m2m_ssl.h"
#include "wdrv_winc_ssl.h"
#endif
#ifdef WDRV_WINC_ENABLE_BLE
#include "platform.h"
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

/* This is user configurable function pointer for printf style output from driver. */
WDRV_WINC_DEBUG_PRINT_CALLBACK pfWINCDebugPrintCb;

/* This is the driver instance structure. */
static WDRV_WINC_DCPT wincDescriptor =
{
    .isInit = false,
    .sysStat = SYS_STATUS_UNINITIALIZED,
};

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Callback Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    void _WDRV_WINC_WifiCallback(uint8_t msgType, const void *const pMsgContent)

  Summary:
    WiFi event callback.

  Description:
    Callback to handle:
      M2M_WIFI_RESP_SCAN_DONE
      M2M_WIFI_RESP_SCAN_RESULT
      M2M_WIFI_RESP_CON_STATE_CHANGED
      M2M_WIFI_REQ_DHCP_CONF
      M2M_WIFI_RESP_GET_SYS_TIME
      M2M_WIFI_RESP_PROVISION_INFO
      M2M_WIFI_RESP_CONN_INFO
      M2M_WIFI_RESP_CURRENT_RSSI
      M2M_WIFI_REQ_WPS

  Precondition:
    None.

  Parameters:
    msgType     - Message type.
    pMsgContent - Pointer to message specific data.

  Returns:
    None.

  Remarks:
    None.

*/

static void _WDRV_WINC_WifiCallback(uint8_t msgType, const void *const pMsgContent)
{
    WDRV_WINC_DCPT *const pDcpt = &wincDescriptor;

    switch (msgType)
    {
        /* Requested BSS scan has completed. */
        case M2M_WIFI_RESP_SCAN_DONE:
        {
            const tstrM2mScanDone *const pScanDoneInfo =
                    (const tstrM2mScanDone *const)pMsgContent;

            /* Check that a scan was requested. */
            if (false == pDcpt->scanInProgress)
            {
                break;
            }

            /* Clear scan in progress flag. */
            pDcpt->scanInProgress = false;

            if (M2M_SUCCESS == pScanDoneInfo->s8ScanState)
            {
                /* Scan completed successfully. */
                pDcpt->scanNumScanResults = pScanDoneInfo->u8NumofCh;

                if (pScanDoneInfo->u8NumofCh > 0)
                {
                    /* If there are BSS results available, request the first one. */
                    m2m_wifi_req_scan_result(pDcpt->scanIndex);
                }
                else if (NULL != pDcpt->pfBSSFindNotifyCB)
                {
                    /* If no results are available then signal callback. */
                    pDcpt->pfBSSFindNotifyCB((DRV_HANDLE)pDcpt, 0, 0, NULL);
                }
                else
                {
                    /* No results and no callback so do nothing. */
                }
            }
            else
            {
                /* Ensure no results are available if scan fails. */
                pDcpt->scanNumScanResults = 0;
            }

            break;
        }

        /* A set of BSS scan results are available for access. */
        case M2M_WIFI_RESP_SCAN_RESULT:
        {
            const tstrM2mWifiscanResult *const pScanAPInfo =
                    (const tstrM2mWifiscanResult *const)pMsgContent;

            /* Copy BSS scan results from message buffer into driver local cache. */
            memcpy(pDcpt->lastBSSScanInfo.bssid, pScanAPInfo->au8BSSID, 6);
            memcpy(pDcpt->lastBSSScanInfo.ssid.name, pScanAPInfo->au8SSID, 32);

            pDcpt->lastBSSScanInfo.ssid.length = strlen((const char*)pScanAPInfo->au8SSID);
            pDcpt->lastBSSScanInfo.rssi        = pScanAPInfo->s8rssi;
            pDcpt->lastBSSScanInfo.authType    = pScanAPInfo->u8AuthType;
            pDcpt->lastBSSScanInfo.channel     = pScanAPInfo->u8ch;

            /* Mark BSS scan cache as valid. */
            pDcpt->isBSSScanInfoValid = true;

            if (NULL != pDcpt->pfBSSFindNotifyCB)
            {
                bool findResult;

                /* Notify the user application of the scan results. */
                findResult = pDcpt->pfBSSFindNotifyCB((DRV_HANDLE)pDcpt,
                        pDcpt->scanIndex+1, pDcpt->scanNumScanResults, &pDcpt->lastBSSScanInfo);

                /* Check if the callback requested the next set of results. */
                if (true == findResult)
                {
                    /* Request the next BSS results, or end operation if no
                       more are available. */
                    pDcpt->scanIndex++;

                    if (pDcpt->scanIndex < m2m_wifi_get_num_ap_found())
                    {
                        m2m_wifi_req_scan_result(pDcpt->scanIndex);
                        pDcpt->isBSSScanInfoValid = false;
                    }
                    else pDcpt->pfBSSFindNotifyCB = NULL;
                }
                else pDcpt->pfBSSFindNotifyCB = NULL;

                /* The user callback is cleared on error or end of scan. */
            }

            break;
        }

        /* The WiFi connection has changed state. */
        case M2M_WIFI_RESP_CON_STATE_CHANGED:
        {
            const tstrM2mWifiStateChanged *const pConnectState =
                    (const tstrM2mWifiStateChanged *const)pMsgContent;

            if (M2M_WIFI_CONNECTED == pConnectState->u8CurrState)
            {
                /* If WiFi has connected update local state. If DHCP is not in
                   use then also signal an IP address is assigned. */
                pDcpt->isConnected = true;

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
                if ((false == pDcpt->useDHCP) && (0 != pDcpt->ipAddress))
                {
                    pDcpt->haveIPAddress = true;
                }
#endif
            }
            else if (M2M_WIFI_DISCONNECTED == pConnectState->u8CurrState)
            {
                /* If WiFi has disconnect update local state and invalidate any
                   association data held locally in the driver. */
                pDcpt->isConnected    = false;
                pDcpt->assocInfoValid = false;
            }
#ifdef WDRV_WINC_DEVICE_BSS_ROAMING
            else if (M2M_WIFI_ROAMED == pConnectState->u8CurrState)
            {
            }
#endif
            else
            {
                /* No other state should be signalled. */
            }

            if (NULL != pDcpt->pfConnectNotifyCB)
            {
                /* Update user application via callback if set. */
                pDcpt->pfConnectNotifyCB((DRV_HANDLE)pDcpt,
                        pConnectState->u8CurrState, pConnectState->u8ErrCode);
            }

            break;
        }

        case M2M_WIFI_RESP_DEFAULT_CONNECT:
        /* The default connect attempt failed */
        {
            const tstrM2MDefaultConnResp *const pDefaultConnectRsp =
                    (const tstrM2MDefaultConnResp *const)pMsgContent;

            if (NULL != pDcpt->pfConnectNotifyCB)
            {
                WDRV_WINC_CONN_ERROR errorCode;

                switch (pDefaultConnectRsp->s8ErrorCode)
                {
                    case M2M_DEFAULT_CONN_INPROGRESS:
                    {
                        errorCode = WDRV_WINC_CONN_ERROR_INPROGRESS;
                        break;
                    }

                    case M2M_DEFAULT_CONN_FAIL:
                    {
                        errorCode = WDRV_WINC_CONN_ERROR_AUTH;
                        break;
                    }

                    case M2M_DEFAULT_CONN_SCAN_MISMATCH:
                    {
                        errorCode = WDRV_WINC_CONN_ERROR_SCAN;
                        break;
                    }

                    case M2M_DEFAULT_CONN_EMPTY_LIST:
                    {
                        errorCode = WDRV_WINC_CONN_ERROR_NOCRED;
                        break;
                    }

                    default:
                    {
                        errorCode = WDRV_WINC_CONN_ERROR_UNKNOWN;
                        break;
                    }
                }

                /* Update user application via callback if set. */
                pDcpt->pfConnectNotifyCB((DRV_HANDLE)pDcpt,
                        M2M_WIFI_DISCONNECTED, errorCode);
            }

            break;
        }

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
        /* The IP address of the connection has been updated due to DHCP. */
        case M2M_WIFI_REQ_DHCP_CONF:
        {
            const uint8_t *const pIP = (const uint8_t *const)pMsgContent;

            if (false == pDcpt->isAP)
            {
                /* For a STA signal that an IP address has now been assigned. */
                pDcpt->haveIPAddress = true;
            }

            pDcpt->ipAddress = ( (uint32_t)pIP[3] << 24) |
                               ( (uint32_t)pIP[2] << 16) |
                               ( (uint32_t)pIP[1] << 8 ) |
                               ( (uint32_t)pIP[0]);

            if (NULL != pDcpt->pfDHCPAddressEventCB)
            {
                /* Signal IP address to user application via callback. */
                pDcpt->pfDHCPAddressEventCB((DRV_HANDLE)pDcpt, pDcpt->ipAddress);
            }

            break;
        }
#endif

        /* The system time has been updated. */
        case M2M_WIFI_RESP_GET_SYS_TIME:
        {
            const tstrSystemTime *const pSysTime =
                    (const tstrSystemTime *const)pMsgContent;
            uint32_t timeUTC;

            /* Convert from internal M2M time structure to UTC. */
            timeUTC = WDRV_WINC_LocalTimeToUTC(pSysTime);

            if (NULL != pDcpt->pfSystemTimeGetCurrentCB)
            {
                /* Pass time to user application if callback was supplied. */
                pDcpt->pfSystemTimeGetCurrentCB((DRV_HANDLE)pDcpt, timeUTC);
            }

            break;
        }

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
        /* Provisioning information has been supplied to the WINC via the HTTP
           provision application running on the device. */
        case M2M_WIFI_RESP_PROVISION_INFO:
        {
            /* The information is only useful for the user application. */
            if (NULL != pDcpt->pfProvConnectInfoCB)
            {
                WDRV_WINC_SSID         targetSSID;
                WDRV_WINC_AUTH_CONTEXT authCtx;

                const tstrM2MProvisionInfo *const pProvInfo =
                        (const tstrM2MProvisionInfo *const)pMsgContent;

                /* Check the SSID length. */
                targetSSID.length = strlen((const char *)pProvInfo->au8SSID);
                if (targetSSID.length > 32)
                {
                    break;
                }

                /* Copy SSID to local structure, clear unused space. */
                memset(&targetSSID.name, 0, 32);
                memcpy(&targetSSID.name, pProvInfo->au8SSID, targetSSID.length);

                /* Copy authentication information to local context. Currently
                   only WPA and OPEN are supported. */
                authCtx.authType = pProvInfo->u8SecType;

                if (WDRV_WINC_AUTH_TYPE_WPA_PSK == pProvInfo->u8SecType)
                {
                    authCtx.authInfo.WPAPerPSK.size = (uint8_t)strlen((const char*)pProvInfo->au8Password);
                    memset(&authCtx.authInfo.WPAPerPSK.key, 0, M2M_MAX_PSK_LEN-1);
                    memcpy(&authCtx.authInfo.WPAPerPSK.key, &pProvInfo->au8Password, authCtx.authInfo.WPAPerPSK.size);
                }

                /* Pass information to user application via the callback, the
                   callback is cleared after this operation has completed. */
                pDcpt->pfProvConnectInfoCB((DRV_HANDLE)pDcpt, &targetSSID,
                        &authCtx, (M2M_SUCCESS == pProvInfo->u8Status) ? true : false);

                pDcpt->pfProvConnectInfoCB = NULL;
            }

            break;
        }
#endif

        /* Information about the current association has been received. */
        case M2M_WIFI_RESP_CONN_INFO:
        {
            const tstrM2MConnInfo *const pConnInfo =
                    (const tstrM2MConnInfo *const)pMsgContent;

            /* Copy and check the SSID length. */
            pDcpt->assocSSID.length = strlen(pConnInfo->acSSID);
            if (pDcpt->assocSSID.length > 32)
            {
                break;
            }

            /* Copy the SSID, ensuring unused space is cleared. */
            memset(&pDcpt->assocSSID.name, 0, 32);
            memcpy(&pDcpt->assocSSID.name, pConnInfo->acSSID, pDcpt->assocSSID.length);

            /* Copy the authentication type. */
            pDcpt->assocAuthType = pConnInfo->u8SecType;

            /* Copy the peer IP and MAC addresses. */
            pDcpt->assocPeerAddress.ipAddress = ( (uint32_t)pConnInfo->au8IPAddr[3] << 24) |
                                                ( (uint32_t)pConnInfo->au8IPAddr[2] << 16) |
                                                ( (uint32_t)pConnInfo->au8IPAddr[1] << 8)  |
                                                ( (uint32_t)pConnInfo->au8IPAddr[0]);

            memcpy(&pDcpt->assocPeerAddress.macAddress, pConnInfo->au8MACAddress, 6);

            /* Mark local store of association as valid. */
            pDcpt->assocInfoValid = true;

            if (NULL != pDcpt->pfAssociationInfoCB)
            {
                /* Pass association information to user application via callback. */
                pDcpt->pfAssociationInfoCB((DRV_HANDLE)pDcpt, &pDcpt->assocSSID,
                        &pDcpt->assocPeerAddress, pDcpt->assocAuthType, pConnInfo->s8RSSI);
            }
            break;
        }

        /* The current RSSI value has been received. */
        case M2M_WIFI_RESP_CURRENT_RSSI:
        {
            const int8_t *const pRSSI = (const int8_t *const)pMsgContent;

            /* Store locally. */
            pDcpt->rssi = *pRSSI;

            if (NULL != pDcpt->pfAssociationRSSICB)
            {
                /* Pass RSSI value to user application if callback supplied.
                   the callback is cleared after this operation has completed. */
                pDcpt->pfAssociationRSSICB((DRV_HANDLE)pDcpt, pDcpt->rssi);

                pDcpt->pfAssociationRSSICB = NULL;
            }
            break;
        }

        /* The WPS information has been received. */
        case M2M_WIFI_REQ_WPS:
        {
            const tstrM2MWPSInfo *const pWPSInfo =
                    (const tstrM2MWPSInfo *const)pMsgContent;

            /* The information is only useful for the user application. */
            if (NULL != pDcpt->pfWPSDiscoveryCB)
            {
                WDRV_WINC_BSS_CONTEXT bssCtx;
                WDRV_WINC_AUTH_CONTEXT authCtx;

                /* Initialise the BSS context with the received BSS information. */
                WDRV_WINC_BSSCtxSetDefaults(&bssCtx);
                WDRV_WINC_BSSCtxSetSSID(&bssCtx,
                                        (uint8_t *const)pWPSInfo->au8SSID,
                                        strlen((const char *)pWPSInfo->au8SSID));
                WDRV_WINC_BSSCtxSetChannel(&bssCtx, pWPSInfo->u8Ch);

                /* Initialise the authentication context. */
                switch (pWPSInfo->u8AuthType)
                {
                    case M2M_WIFI_SEC_OPEN:
                    {
                        WDRV_WINC_AuthCtxSetOpen(&authCtx);
                        break;
                    }

                    case M2M_WIFI_SEC_WPA_PSK:
                    {
                        WDRV_WINC_AuthCtxSetWPA(&authCtx,
                                                (uint8_t *const)pWPSInfo->au8PSK,
                                                strlen((const char *)pWPSInfo->au8PSK));
                        break;
                    }

                    default:
                    {
                        pDcpt->pfWPSDiscoveryCB((DRV_HANDLE)pDcpt, NULL, NULL);
                        return;
                    }
                }

                // Pass the BSS and authentication contexts to the user application. */
                pDcpt->pfWPSDiscoveryCB((DRV_HANDLE)pDcpt, &bssCtx, &authCtx);
            }

            break;
        }

#ifdef WDRV_WINC_ENABLE_BLE
        /* A BLE message has been received, pass it on to the BLE application. */
        case M2M_WIFI_RESP_BLE_API_RECV:
        {
            if (true == pDcpt->bleActive)
            {
                const tstrM2mBleApiMsg *const pBLEMsg = (const tstrM2mBleApiMsg *const)pMsgContent;

                platform_interface_callback((uint8_t*)pBLEMsg->data, pBLEMsg->u16Len);
            }
            break;
        }
#endif

        default:
        {
            break;
        }
    }
}

#ifndef WDRV_WINC_NETWORK_MODE_SOCKET
//*******************************************************************************
/*
  Function:
    void _WDRV_WINC_EthernetCallback
    (
        uint8_t msgType,
        const void *const pMsgContent,
        const void *const pCtrlBuf
    )

  Summary:
    Ethernet callback.

  Description:
    Callback to handle:
      M2M_WIFI_RESP_ETHERNET_RX_PACKET

  Precondition:
    None.

  Parameters:
    msgType     - Message type.
    pMsgContent - Pointer to message specific data.
    pCtrlBuf    - Pointer to control/information structure.

  Returns:
    None.

  Remarks:
    None.

*/

static void _WDRV_WINC_EthernetCallback
(
    uint8_t msgType,
    const void *const pMsgContent,
    const void *const pCtrlBuf
)
{
    WDRV_WINC_DCPT *const pDcpt = &wincDescriptor;

    /* Ensure on received Ethernet packets are handled. */
    if (M2M_WIFI_RESP_ETHERNET_RX_PACKET == msgType)
    {
        const tstrM2mIpCtrlBuf *const pEthCtrlBuf =
                (const tstrM2mIpCtrlBuf *const)pCtrlBuf;
        bool isFragmented;

        /* Only process if the callback has been set. */
        if (NULL != pDcpt->pfEthernetMsgRecvCB)
        {
            /* Check for message fragmentation. */
            if ((0 == pEthCtrlBuf->u16RemainingDataSize) && (1 == pDcpt->ethFragNum))
            {
                isFragmented = false;
            }
            else
            {
                isFragmented = true;
            }

            /* Pass Ethernet frame to user application. */
            pDcpt->pfEthernetMsgRecvCB((DRV_HANDLE)pDcpt, pMsgContent,
                    pEthCtrlBuf->u16DataSize, isFragmented, pDcpt->ethFragNum);

            /* Check if the last fragment and reset state for next packet. */
            if (0 == pEthCtrlBuf->u16RemainingDataSize)
            {
                pDcpt->ethFragNum = 1;
            }
            else
            {
                pDcpt->ethFragNum++;
            }
        }
    }
}
#endif

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
//*******************************************************************************
/*
  Function:
    void _WDRV_WINC_OTAUpdateCallback(uint8_t type, uint8_t status);

  Summary:
    OTA update callback.

  Description:
    Callback to handle:
      DL_STATUS
      SW_STATUS
      RB_STATUS

  Precondition:
    None.

  Parameters:
    type   - Status type.
    status - Status value.

  Returns:
    None.

  Remarks:
    None.

*/

static void _WDRV_WINC_OTAUpdateCallback(uint8_t type, uint8_t status)
{
    WDRV_WINC_DCPT *const pDcpt = &wincDescriptor;

    switch (type)
    {
        case DL_STATUS:
        {
            /* If callback supplied update with download status. */
            if (NULL != pDcpt->pfOTADownloadStatusCB)
            {
                pDcpt->pfOTADownloadStatusCB((DRV_HANDLE)pDcpt, status);
            }

            pDcpt->updateInProgress      = false;
            pDcpt->pfOTADownloadStatusCB = NULL;
            break;
        }

        case SW_STATUS:
        case RB_STATUS:
        {
            /* If callback supplied update with switch status. */
            if (NULL != pDcpt->pfSwitchFirmwareStatusCB)
            {
                pDcpt->pfSwitchFirmwareStatusCB((DRV_HANDLE)pDcpt, status);
            }

            pDcpt->pfSwitchFirmwareStatusCB = NULL;
            break;
        }

        default:
        {
            break;
        }
    }
}

//*******************************************************************************
/*
  Function:
    void _WDRV_WINC_SSLCallback(uint8_t msgType, void *pMsgContent)

  Summary:
    SSL event callback.

  Description:
    Callback to handle:
      M2M_SSL_REQ_ECC
      M2M_SSL_RESP_SET_CS_LIST
      M2M_SSL_RESP_WRITE_OWN_CERTS

  Precondition:
    None.

  Parameters:
    msgType     - Message type.
    pMsgContent - Pointer to message specific data.

  Returns:
    None.

  Remarks:
    None.

*/

static void _WDRV_WINC_SSLCallback(uint8_t msgType, void *pMsgContent)
{
    WDRV_WINC_DCPT *const pDcpt = &wincDescriptor;

    switch (msgType)
    {
        /* Handle set cipher suite list response. */
        case M2M_SSL_RESP_SET_CS_LIST:
        {
            /* List is only of interest to user application via callback. */
            if (NULL != pDcpt->pfSSLCipherSuiteListCB)
            {
                const tstrSslSetActiveCsList *const pSSLCSList = (const tstrSslSetActiveCsList *const)pMsgContent;

                WDRV_WINC_CIPHER_SUITE_CONTEXT cipherSuiteCtx;

                cipherSuiteCtx.ciperSuites = pSSLCSList->u32CsBMP;

                pDcpt->pfSSLCipherSuiteListCB((DRV_HANDLE)pDcpt, &cipherSuiteCtx);
            }
            break;
        }
        
        case M2M_SSL_REQ_ECC:
		{
            WDRV_WINC_ECC_REQ_INFO info;
            if (NULL != pDcpt->pfSSLReqECCCB)
            {
                tstrEccReqInfo  *eccReqInfo;
                eccReqInfo  = (tstrEccReqInfo *)pMsgContent;
                
                info.reqCmd = eccReqInfo->u16REQ;
                info.seqNo = eccReqInfo->u32SeqNo;
                info.status = eccReqInfo->u16Status;
                info.userData = eccReqInfo->u32UserData;
                
                switch (info.reqCmd)
                {
                    case ECC_REQ_CLIENT_ECDH:
                    case ECC_REQ_GEN_KEY:
                    case ECC_REQ_SERVER_ECDH:
                    {
                        WDRV_WINC_ECDH_REQ_INFO ecdhReqInfo;
                        memcpy(&ecdhReqInfo, &eccReqInfo->strEcdhREQ, sizeof(tstrEcdhReqInfo));
                        pDcpt->pfSSLReqECCCB((DRV_HANDLE)pDcpt, info, &ecdhReqInfo);
                        break;
                    }        

                    case ECC_REQ_SIGN_VERIFY:
                    {
                        WDRV_WINC_ECDSA_VERIFY_REQ_INFO ecdsaVerifyReqInfo;
                        memcpy(&ecdsaVerifyReqInfo, &eccReqInfo->strEcdsaVerifyREQ, sizeof(tstrEcdsaVerifyReqInfo));
                        pDcpt->pfSSLReqECCCB((DRV_HANDLE)pDcpt, info, &ecdsaVerifyReqInfo);
                        break;
                    }

                    case ECC_REQ_SIGN_GEN:
                    {
                        WDRV_WINC_ECDSA_SIGN_REQ_INFO ecdsaSignReqInfo;
                        memcpy(&ecdsaSignReqInfo, &eccReqInfo->strEcdsaSignREQ, sizeof(tstrEcdsaSignReqInfo));
                        pDcpt->pfSSLReqECCCB((DRV_HANDLE)pDcpt, info, &ecdsaSignReqInfo);
                        break;
                        
                    }

		    default:
                    {
                        break;
                    }
                }
                                        
            }
            break;
        }

        default:
        {
            break;
        }
    }
}
#endif

#ifdef WDRV_WINC_DEVICE_HOST_FILE_DOWNLOAD
//*******************************************************************************
/*
  Function:
    void _WDRV_WINC_HostFileGetCallback
    (
        uint8_t status,
        uint8_t handle,
        uint32_t size
    )

  Summary:
    Callback called in response to WDRV_WINC_HostFileDownload.

  Description:
    When WDRV_WINC_HostFileDownload is called to request a file be downloaded
      this function is called when the response is known.

  Precondition:
    WDRV_WINC_HostFileDownload has been called to request a file download.

  Parameters:
    status  - Status of download operation, see tenuOtaUpdateStatus
    handle  - WINC file handle ID if file was downloaded.
    size    - Size of file, in bytes, if file was downloaded.

  Returns:
    None.

  Remarks:
    None.

*/

static void _WDRV_WINC_HostFileGetCallback
(
    uint8_t status,
    uint8_t handle,
    uint32_t size
)
{
    WDRV_WINC_DCPT *const pDcpt = &wincDescriptor;

    if (OTA_STATUS_SUCCESS == status)
    {
        pDcpt->hostFileDcpt.handle = handle;
        pDcpt->hostFileDcpt.size   = size;
    }
    else
    {
        pDcpt->hostFileDcpt.handle = HFD_INVALID_HANDLER;
        pDcpt->hostFileDcpt.size   = 0;
    }

    pDcpt->hostFileDcpt.offset = 0;
    pDcpt->hostFileDcpt.remain = 0;
    pDcpt->hostFileDcpt.pBuffer = NULL;

    if (NULL != pDcpt->pfHostFileCB)
    {
        pDcpt->pfHostFileCB((DRV_HANDLE)pDcpt,
                (WDRV_WINC_HOST_FILE_HANDLE)&pDcpt->hostFileDcpt,
                WDRV_WINC_HOST_FILE_OP_DOWNLOAD, status);
    }
}

//*******************************************************************************
/*
  Function:
    void _WDRV_WINC_HostFileReadHIFCallback
    (
        uint8_t status,
        void *pBuffer,
        uint32_t size
    )

  Summary:
    Callback called in response to WDRV_WINC_HostFileRead.

  Description:
    When WDRV_WINC_HostFileRead is called to request a file be read
      this function is called when the response is known.

  Precondition:
    WDRV_WINC_HostFileRead has been called to request a file read.

  Parameters:
    status  - Status of read operation, see tenuOtaUpdateStatus
    handle  - Pointer to the data read.
    size    - Size of data read.

  Returns:
    None.

  Remarks:
    None.

*/

static void _WDRV_WINC_HostFileReadHIFCallback
(
    uint8_t status,
    void *pBuffer,
    uint32_t size
)
{
    WDRV_WINC_DCPT *const pDcpt = &wincDescriptor;

    if (OTA_STATUS_SUCCESS == status)
    {
        if ((NULL != pDcpt->hostFileDcpt.pBuffer) && (size <= pDcpt->hostFileDcpt.remain))
        {
            if (pDcpt->hostFileDcpt.remain < size)
            {
                size = pDcpt->hostFileDcpt.remain;
            }

            memcpy(pDcpt->hostFileDcpt.pBuffer, pBuffer, size);

            pDcpt->hostFileDcpt.remain  -= size;
            pDcpt->hostFileDcpt.pBuffer += size;
            pDcpt->hostFileDcpt.offset  += size;

            if (pDcpt->hostFileDcpt.remain > 0)
            {
                WDRV_WINC_HostFileRead((DRV_HANDLE)pDcpt,
                    (WDRV_WINC_HOST_FILE_HANDLE)&pDcpt->hostFileDcpt,
                    NULL, 0, 0, pDcpt->pfHostFileCB);
            }
        }
        else
        {
            status = OTA_STATUS_FAIL;
        }
    }

    if ((OTA_STATUS_SUCCESS != status) || (0 == pDcpt->hostFileDcpt.remain))
    {
        if (NULL != pDcpt->pfHostFileCB)
        {
            pDcpt->pfHostFileCB((DRV_HANDLE)pDcpt,
                    (WDRV_WINC_HOST_FILE_HANDLE)&pDcpt->hostFileDcpt,
                    WDRV_WINC_HOST_FILE_OP_READ, status);
        }
    }
}

//*******************************************************************************
/*
  Function:
    void _WDRV_WINC_HostFileEraseCallback(uint8_t status)

  Summary:
    Callback called in response to WDRV_WINC_HostFileErase.

  Description:
    When WDRV_WINC_HostFileErase is called to request a file be erased
      this function is called when the response is known.

  Precondition:
    WDRV_WINC_HostFileErase has been called to request a file erase.

  Parameters:
    status  - Status of erase operation, see tenuOtaUpdateStatus

  Returns:
    None.

  Remarks:
    None.

*/

static void _WDRV_WINC_HostFileEraseCallback(uint8_t status)
{
    WDRV_WINC_DCPT *const pDcpt = &wincDescriptor;

    if (NULL != pDcpt->pfHostFileCB)
    {
        pDcpt->pfHostFileCB((DRV_HANDLE)pDcpt,
                (WDRV_WINC_HOST_FILE_HANDLE)&pDcpt->hostFileDcpt,
                WDRV_WINC_HOST_FILE_OP_ERASE, status);

        pDcpt->hostFileDcpt.handle = HFD_INVALID_HANDLER;
        pDcpt->hostFileDcpt.size   = 0;
    }
}
#endif

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver System Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    SYS_MODULE_OBJ WDRV_WINC_Initialize
    (
        const SYS_MODULE_INDEX index,
        const SYS_MODULE_INIT *const init
    )

  Summary:
    System interface initialization of the WINC driver.

  Description:
    This is the function that initializes the WINC driver. It is called by
    the system.

  Remarks:
    See wdrv_winc_api.h for usage information.

*/

SYS_MODULE_OBJ WDRV_WINC_Initialize
(
    const SYS_MODULE_INDEX index,
    const SYS_MODULE_INIT *const init
)
{
    WDRV_WINC_DCPT *const pDcpt = &wincDescriptor;

    /* Set initial state. */
    pDcpt->isInit  = false;
    pDcpt->isOpen  = false;
    pDcpt->sysStat = SYS_STATUS_UNINITIALIZED;
    pDcpt->intent  = 0;

    pDcpt->userHandle = 0;

    pfWINCDebugPrintCb = NULL;

    return (SYS_MODULE_OBJ)pDcpt;
}

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_Deinitialize(SYS_MODULE_OBJ object)

  Summary:
    WINC driver deinitialization function.

  Description:
    This is the function that deinitializes the WINC.
    It is called by the system.

  Remarks:
    See wdrv_winc_api.h for usage information.

*/

void WDRV_WINC_Deinitialize(SYS_MODULE_OBJ object)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)object;

    if (SYS_STATUS_UNINITIALIZED != pDcpt->sysStat)
    {
        /* De-initialise SPI handling. */
        WDRV_WINC_SPIDeinitialize();
    }

    /* Clear internal state. */
    pDcpt->isInit  = false;
    pDcpt->isOpen  = false;
    pDcpt->sysStat = SYS_STATUS_UNINITIALIZED;
    pDcpt->intent  = 0;
}

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_Reinitialize
    (
        SYS_MODULE_OBJ object,
        const SYS_MODULE_INIT *const init
    )

  Summary:
    WINC driver reinitialization function.

  Description:
    This is the function that re-initializes the WINC.
    It is called by the system.

  Remarks:
    See wdrv_winc_api.h for usage information.

*/

void WDRV_WINC_Reinitialize
(
    SYS_MODULE_OBJ object,
    const SYS_MODULE_INIT *const init
)
{
}

//*******************************************************************************
/*
  Function:
    SYS_STATUS WDRV_WINC_Status(SYS_MODULE_OBJ object)

  Summary:
    Provides the current status of the WINC driver module.

  Description:
    This function provides the current status of the WINC driver module.

  Remarks:
    See wdrv_winc_api.h for usage information.

*/

SYS_STATUS WDRV_WINC_Status(SYS_MODULE_OBJ object)
{
    return ((WDRV_WINC_DCPT *)object)->sysStat;
}

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_Tasks(SYS_MODULE_OBJ object)

  Summary:
    Maintains the WINC drivers state machine.

  Description:
    This function is used to maintain the driver's internal state machine.

  Remarks:
    See wdrv_winc_api.h for usage information.

*/

void WDRV_WINC_Tasks(SYS_MODULE_OBJ object)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)object;

    switch (pDcpt->sysStat)
    {
        /* Uninitialised state. */
        case SYS_STATUS_UNINITIALIZED:
        {
            pDcpt->sysStat = SYS_STATUS_BUSY;

            WDRV_DBG_INFORM_PRINT("WINC: Initializing...\r\n");
            
            if (OSAL_RESULT_TRUE != OSAL_MUTEX_Create(&pDcpt->eventProcessMutex))
            {
                WDRV_DBG_ERROR_PRINT("eventProcessMutex create failed\r\n");
                pDcpt->sysStat = SYS_STATUS_ERROR;
                break;
            }

            /* Initialise SPI handling. */
            WDRV_WINC_SPIInitialize();

#ifndef WDRV_WINC_DEVICE_SPLIT_INIT
            pDcpt->isInit  = true;
            pDcpt->sysStat = SYS_STATUS_READY;
#else
            if (M2M_SUCCESS != m2m_wifi_init_hold())
            {
                WDRV_DBG_ERROR_PRINT("m2m_wifi_init_hold failed\r\n");
                pDcpt->sysStat = SYS_STATUS_ERROR;
            }
            else
            {
#ifdef WDRV_WINC_DEVICE_USE_FLASH_INIT
                tstrFlashState  strFlashState;
                m2m_flash_get_state(&strFlashState);
                WDRV_DBG_INFORM_PRINT("FlashAccess:%x:%d%d\r\n", strFlashState.u16LastAccessId, strFlashState.u8Success, strFlashState.u8Changed);

                m2m_flash_init();
#endif

                pDcpt->isInit  = true;
                pDcpt->sysStat = SYS_STATUS_READY;
            }
#endif
            break;
        }

        /* Running steady state. */
        case SYS_STATUS_READY:
        {
            if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pDcpt->eventProcessMutex, OSAL_WAIT_FOREVER))
            {
                if (pDcpt->isOpen == true)
                {
                    /* If driver instance is open the check HIF ISR semaphore and
                       handle a pending event. */

                    if (OSAL_RESULT_TRUE == OSAL_SEM_Pend(&pDcpt->isrSemaphore, OSAL_WAIT_FOREVER))
                    {
                        if (M2M_SUCCESS != m2m_wifi_handle_events())
                        {
                            OSAL_SEM_Post(&pDcpt->isrSemaphore);
                        }
                    }
                }
                OSAL_MUTEX_Unlock(&pDcpt->eventProcessMutex);
            }
            break;
        }

        /* Error state.*/
        case SYS_STATUS_ERROR:
        {
            break;
        }

        default:
        {
            WDRV_DBG_ERROR_PRINT("Should never happen\r\n");
            pDcpt->sysStat = SYS_STATUS_ERROR;
            break;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Client Implementation
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

  Remarks:
    See wdrv_winc.h for usage information.

 */

void WDRV_WINC_DebugRegisterCallback
(
    WDRV_WINC_DEBUG_PRINT_CALLBACK const pfDebugPrintCallback
)
{
    pfWINCDebugPrintCb = pfDebugPrintCallback;
}

//*******************************************************************************
/*
  Function:
    DRV_HANDLE WDRV_WINC_Open(const SYS_MODULE_INDEX index, const DRV_IO_INTENT intent)

  Summary:
    Opens an instance of the WINC driver.

  Description:
    Opens an instance of the WINC driver and returns a handle which must be
    used when calling other driver functions.

  Remarks:
    See wdrv_winc.h for usage information.

*/

DRV_HANDLE WDRV_WINC_Open(const SYS_MODULE_INDEX index, const DRV_IO_INTENT intent)
{
    tstrWifiInitParam wifiParam;
    WDRV_WINC_DCPT *const pDcpt = &wincDescriptor;

    /* Check that the driver has been initialised. */
    if (false == pDcpt->isInit)
    {
        return DRV_HANDLE_INVALID;
    }

    /* Check if the driver has already been opened. */
    if (true == pDcpt->isOpen)
    {
        if (pDcpt->intent == intent)
        {
            return (DRV_HANDLE)pDcpt;
        }
        else
        {
            return DRV_HANDLE_INVALID;
        }
    }

    /* Create a semaphore for tracking WINC HIF events. */
    if (OSAL_RESULT_TRUE != OSAL_SEM_Create(&pDcpt->isrSemaphore,
                                            OSAL_SEM_TYPE_COUNTING, 10, 0))
    {
        return DRV_HANDLE_INVALID;
    }

    pDcpt->intent = intent;

    /* Check if opening for exclusive access to NVM rather than whole device. */

    if (0 != (pDcpt->intent & DRV_IO_INTENT_EXCLUSIVE))
    {
#ifdef WDRV_WINC_DEVICE_SPLIT_INIT
        m2m_wifi_deinit(NULL);
#endif
        if (M2M_SUCCESS != m2m_wifi_download_mode())
        {
            return DRV_HANDLE_INVALID;
        }

        pDcpt->isOpen = true;

        return (DRV_HANDLE)pDcpt;
    }

    /* Initialise the interrupts. */
    WDRV_WINC_INTInitialize();

    memset(&wifiParam, 0, sizeof(tstrWifiInitParam));
    /* Construct M2M WiFi initialisation structure. */
    wifiParam.pfAppWifiCb = _WDRV_WINC_WifiCallback;
#ifndef WDRV_WINC_NETWORK_MODE_SOCKET
    /* Ethernet mode required callback and Ethernet buffers. */
    wifiParam.strEthInitParam.pfAppEthCb = _WDRV_WINC_EthernetCallback;
    wifiParam.strEthInitParam.au8ethRcvBuf = NULL;
    wifiParam.strEthInitParam.u16ethRcvBufSize = 0;
#ifdef WDRV_WINC_DEVICE_DYNAMIC_BYPASS_MODE
    wifiParam.strEthInitParam.u8EthernetEnable = true;
#endif
#else
    /* For socket mode. */
#ifdef WDRV_WINC_DEVICE_DYNAMIC_BYPASS_MODE
    wifiParam.strEthInitParam.u8EthernetEnable = false;
#endif
#endif

    /* Initialise M2M WiFi and thus WINC device. */
#ifndef WDRV_WINC_DEVICE_SPLIT_INIT
    if (M2M_SUCCESS != m2m_wifi_init(&wifiParam))
#else
    if (M2M_SUCCESS != m2m_wifi_init_start(&wifiParam))
#endif
    {
        WDRV_DBG_ERROR_PRINT("m2m_wifi_init_start failed\r\n");
        pDcpt->sysStat = SYS_STATUS_ERROR;

        return DRV_HANDLE_INVALID;
    }

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    /* Socket mode requires the socket interface and SSL be initialised. */
    socketInit();

    m2m_ssl_init(&_WDRV_WINC_SSLCallback);
#endif

    /* Initialise driver state. */
    pDcpt->isOpen                   = true;
    pDcpt->isAP                     = false;
    pDcpt->isProvisioning           = false;
    pDcpt->isConnected              = false;
    pDcpt->scanInProgress           = false;
    pDcpt->isBSSScanInfoValid       = false;
    pDcpt->scanParamDefault         = true;
    pDcpt->scanNumSlots             = M2M_SCAN_DEFAULT_NUM_SLOTS;
    pDcpt->scanActiveScanTime       = M2M_SCAN_DEFAULT_SLOT_TIME;
    pDcpt->scanNumProbes            = M2M_SCAN_DEFAULT_NUM_PROBE;
    pDcpt->scanRSSIThreshold        = M2M_FASTCONNECT_DEFAULT_RSSI_THRESH;
    pDcpt->scanPassiveScanTime      = M2M_SCAN_DEFAULT_PASSIVE_SLOT_TIME;
    pDcpt->powerSaveDTIMInterval    = 0;
    pDcpt->rssi                     = 0;
    pDcpt->userHandle               = 0;

    pDcpt->assocInfoValid           = false;

    pDcpt->pfBSSFindNotifyCB        = NULL;
    pDcpt->pfConnectNotifyCB        = NULL;
    pDcpt->pfSystemTimeGetCurrentCB = NULL;
    pDcpt->pfAssociationInfoCB      = NULL;
    pDcpt->pfAssociationRSSICB      = NULL;
    pDcpt->pfWPSDiscoveryCB         = NULL;

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    pDcpt->updateInProgress         = false;
    pDcpt->haveIPAddress            = false;
    pDcpt->useDHCP                  = true;

    pDcpt->ipAddress                = 0;
    pDcpt->netMask                  = 0;
    pDcpt->dnsServerAddress         = 0;
    pDcpt->gatewayAddress           = 0;
    pDcpt->dhcpServerAddress        = 0x010AA8C0;

    pDcpt->pfDHCPAddressEventCB     = NULL;
    pDcpt->pfICMPEchoResponseCB     = NULL;
    pDcpt->pfProvConnectInfoCB      = NULL;
    pDcpt->pfOTADownloadStatusCB    = NULL;
    pDcpt->pfSwitchFirmwareStatusCB = NULL;
    pDcpt->pfSSLCipherSuiteListCB   = NULL;
#else
    pDcpt->isEthBufSet              = false;

    pDcpt->ethFragNum               = 1;

    pDcpt->pfEthernetMsgRecvCB      = NULL;
#endif
#ifdef WDRV_WINC_ENABLE_BLE
    pDcpt->bleActive                = false;
#endif

#ifdef WDRV_WINC_DEVICE_HOST_FILE_DOWNLOAD
    pDcpt->hostFileDcpt.handle      = HFD_INVALID_HANDLER;
    pDcpt->hostFileDcpt.pBuffer     = NULL;
    pDcpt->hostFileDcpt.getFileCB   = _WDRV_WINC_HostFileGetCallback;
    pDcpt->hostFileDcpt.readFileCB  = _WDRV_WINC_HostFileReadHIFCallback;
    pDcpt->hostFileDcpt.eraseFileCB = _WDRV_WINC_HostFileEraseCallback;

    pDcpt->pfHostFileCB             = NULL;
#endif

#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
#if defined(WDRV_WINC_DEVICE_WINC1500)
#ifdef WDRV_WINC_DEVICE_HOST_FILE_DOWNLOAD
    m2m_ota_init(_WDRV_WINC_OTAUpdateCallback, NULL, _WDRV_WINC_HostFileGetCallback);
#else
    m2m_ota_init(&_WDRV_WINC_OTAUpdateCallback, NULL);
#endif // #ifdef WDRV_WINC_DEVICE_HOST_FILE_DOWNLOAD
#elif defined(WDRV_WINC_DEVICE_WINC3400)
    m2m_ota_init(_WDRV_WINC_OTAUpdateCallback);
#endif
#endif

    /* Retrieve MAC address from WINC device. */
    m2m_wifi_get_mac_address(pDcpt->ethAddress);

#ifdef WDRV_WINC_DEVICE_WINC3400
    /* On WINC3400 place BLE into restricted state. */
    m2m_wifi_req_restrict_ble();
#endif

    return (DRV_HANDLE)pDcpt;
}

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

  Remarks:
    See wdrv_winc.h for usage information.

*/

void WDRV_WINC_Close(DRV_HANDLE handle)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return;
    }

    /* De-initialise the interrupts. */
    WDRV_WINC_INTDeinitialize();

    /* Destroy M2M HIF semaphore. */
    OSAL_SEM_Post(&pDcpt->isrSemaphore);
    if (OSAL_RESULT_TRUE == OSAL_MUTEX_Lock(&pDcpt->eventProcessMutex, OSAL_WAIT_FOREVER))
    {
        /* Destroy M2M HIF semaphore. */
        OSAL_SEM_Delete(&pDcpt->isrSemaphore);


        /* Reset minimal state to show driver is closed. */
        pDcpt->isOpen        = false;
        OSAL_MUTEX_Unlock(&pDcpt->eventProcessMutex);
    }
    pDcpt->isConnected   = false;
#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
    pDcpt->haveIPAddress = false;
#endif
}

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver General Implementation
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

  Remarks:
    See wdrv_winc.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_MulticastMACFilterAdd
(
    DRV_HANDLE handle,
    const uint8_t *pEthAddress
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pEthAddress))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Update multicast in WINC device. */
    if (M2M_SUCCESS != m2m_wifi_enable_mac_mcast((uint8_t*)pEthAddress, 1))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_MulticastMACFilterRemove
(
    DRV_HANDLE handle,
    const uint8_t *pEthAddress
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pEthAddress))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Update multicast in WINC device. */
    if (M2M_SUCCESS != m2m_wifi_enable_mac_mcast((uint8_t*)pEthAddress, 0))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_EthernetAddressGet
(
    DRV_HANDLE handle,
    uint8_t *pEthAddress
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pEthAddress))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Copy the WINC MAC address to caller buffer. */
    memcpy(pEthAddress, pDcpt->ethAddress, 6);

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_OP_MODE WDRV_WINC_OperatingModeGet(DRV_HANDLE handle)

  Summary:
    Returns the current operating mode of the WINC.

  Description:
    Returns the current operating mode of the WINC.

  Remarks:
    See wdrv_winc.h for usage information.

*/

WDRV_WINC_OP_MODE WDRV_WINC_OperatingModeGet(DRV_HANDLE handle)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return WDRV_WINC_OP_MODE_UNKNOWN;
    }

    if (false == pDcpt->isAP)
    {
        /* WINC is acting as a STA. */
        return WDRV_WINC_OP_MODE_STA;
    }
    else if (false == pDcpt->isProvisioning)
    {
        /* WINC is acting as an AP. */
        return WDRV_WINC_OP_MODE_AP;
    }
    else
    {
        /* WINC is acting as a provisioning AP. */
        return WDRV_WINC_OP_MODE_PROV_AP;
    }
}

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

  Remarks:
    See wdrv_winc.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_UserHandleSet(DRV_HANDLE handle, uintptr_t userHandle)
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

    pDcpt->userHandle = userHandle;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    uintptr_t WDRV_WINC_UserHandleGet(DRV_HANDLE handle)

  Summary:
    Returns the current user supplied handle associated with the driver.

  Description:
    This function returns the handle previously associated with the driver
    instance by a call to WDRV_WINC_UserHandleSet.

  Remarks:
    See wdrv_winc.h for usage information.

*/

uintptr_t WDRV_WINC_UserHandleGet(DRV_HANDLE handle)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return 0;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return 0;
    }

    return pDcpt->userHandle;
}

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

  Remarks:
    See wdrv_winc.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_InfoDriverVersionGet
(
    DRV_HANDLE handle,
    WDRV_WINC_DRIVER_VERSION_INFO *const pDriverVersion
)
{
    const WDRV_WINC_DCPT *const pDcpt = (const WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pDriverVersion))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Copy driver version information. */
    memset(pDriverVersion, 0, sizeof(WDRV_WINC_DRIVER_VERSION_INFO));

#if defined(WDRV_WINC_DEVICE_WINC1500)
    pDriverVersion->version.major = M2M_RELEASE_VERSION_MAJOR_NO;
    pDriverVersion->version.minor = M2M_RELEASE_VERSION_MINOR_NO;
    pDriverVersion->version.patch = M2M_RELEASE_VERSION_PATCH_NO;
#elif defined(WDRV_WINC_DEVICE_WINC3400)
    pDriverVersion->version.major = M2M_DRIVER_VERSION_MAJOR_NO;
    pDriverVersion->version.minor = M2M_DRIVER_VERSION_MINOR_NO;
    pDriverVersion->version.patch = M2M_DRIVER_VERSION_PATCH_NO;
    pDriverVersion->hif.major     = M2M_HIF_MAJOR_VALUE;
    pDriverVersion->hif.minor     = M2M_HIF_MINOR_VALUE;
    pDriverVersion->hif.block     = M2M_HIF_BLOCK_VALUE;
#endif

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_InfoDeviceFirmwareVersionGet
(
    DRV_HANDLE handle,
    bool active,
    WDRV_WINC_FIRMWARE_VERSION_INFO *const pFirmwareVersion
)
{
    const WDRV_WINC_DCPT *const pDcpt = (const WDRV_WINC_DCPT *const)handle;

    tstrM2mRev info;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pFirmwareVersion))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    if (true == active)
    {
        /* For active partition retrieve version information. */
        if (M2M_SUCCESS != m2m_wifi_get_firmware_version(&info))
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }
    }
    else
    {
#ifndef WDRV_WINC_NETWORK_MODE_SOCKET
        /* Non-active partition not supported for Ethernet mode. */
        return WDRV_WINC_STATUS_REQUEST_ERROR;
#else
        /* Retrieve non-active partition information. */
        if (M2M_SUCCESS != m2m_ota_get_firmware_version(&info))
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }
#endif
    }

    /* Copy firmware version information. */
    memset(pFirmwareVersion, 0, sizeof(WDRV_WINC_FIRMWARE_VERSION_INFO));

    pFirmwareVersion->version.major = info.u8FirmwareMajor;
    pFirmwareVersion->version.minor = info.u8FirmwareMinor;
    pFirmwareVersion->version.patch = info.u8FirmwarePatch;
#if defined(WDRV_WINC_DEVICE_WINC1500)
#elif defined(WDRV_WINC_DEVICE_WINC3400)
    pFirmwareVersion->hif.major     = M2M_GET_HIF_MAJOR(info.u16FirmwareHifInfo);
    pFirmwareVersion->hif.minor     = M2M_GET_HIF_MINOR(info.u16FirmwareHifInfo);
    pFirmwareVersion->hif.block     = M2M_GET_HIF_BLOCK(info.u16FirmwareHifInfo);
#endif
    memcpy(pFirmwareVersion->build.date, info.BuildDate, 12);
    memcpy(pFirmwareVersion->build.time, info.BuildTime, 9);

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    uint32_t WDRV_WINC_InfoDeviceIDGet(DRV_HANDLE handle)

  Summary:
    Returns the device ID.

  Description:
    Returns the device ID assigned to the WINC.

  Remarks:
    See wdrv_winc.h for usage information.

*/

uint32_t WDRV_WINC_InfoDeviceIDGet(DRV_HANDLE handle)
{
    const WDRV_WINC_DCPT *const pDcpt = (const WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return 0;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return 0;
    }

    return m2m_wifi_get_chipId();
}

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

  Remarks:
    See wdrv_winc.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_InfoDeviceMACAddressGet(DRV_HANDLE handle,
                                    bool permanent, uint8_t *const pMACAddress)
{
    const WDRV_WINC_DCPT *const pDcpt = (const WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pMACAddress))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    if (true == permanent)
    {
        uint8_t isValid;

        /* Retrieve the permanent MAC address from the WINC device. */
        if (M2M_SUCCESS != m2m_wifi_get_otp_mac_address(pMACAddress, &isValid))
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }

        if (0 == isValid)
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }
    }
    else
    {
        /* Retrieve the current MAC address from the WINC device. */
        if (M2M_SUCCESS != m2m_wifi_get_mac_address(pMACAddress))
        {
            return WDRV_WINC_STATUS_REQUEST_ERROR;
        }
    }

    return WDRV_WINC_STATUS_OK;
}

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

  Remarks:
    See wdrv_winc.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_InfoDeviceNameSet
(
    DRV_HANDLE handle,
    const char* pDeviceName
)
{
    const WDRV_WINC_DCPT *const pDcpt = (const WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle and user pointer is valid. */
    if ((NULL == pDcpt) || (NULL == pDeviceName))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Pass the device name to the WINC device. */
    if (M2M_SUCCESS != m2m_wifi_set_device_name((uint8_t*)pDeviceName, strlen(pDeviceName)))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_GainTableIndexSet(DRV_HANDLE handle, uint8_t index)

  Summary:
    Sets the gain table index.

  Description:
    Sets the gain table index.

  Remarks:
    See wdrv_winc.h for usage information.

*/

#ifdef WDRV_WINC_DEVICE_MULTI_GAIN_TABLE
WDRV_WINC_STATUS WDRV_WINC_GainTableIndexSet(DRV_HANDLE handle, uint8_t index)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle is valid. */
    if (NULL == pDcpt)
    {
        return 0;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return 0;
    }

#if defined(WDRV_WINC_DEVICE_WINC1500)
    if (M2M_SUCCESS != m2m_wifi_set_gain_table_idx(index))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }
#elif defined(WDRV_WINC_DEVICE_WINC3400)
    if (M2M_SUCCESS != m2m_wifi_ble_set_gain_table(index))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }
#endif

    return WDRV_WINC_STATUS_OK;
}
#endif

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_ISR(void);

  Summary:
    WINC interrupt handler.

  Description:
    This function is called by the interrupt routines to signal an interrupt
      from the WINC.

  Remarks:
    See wdrv_winc_api.h for usage information.

*/

void WDRV_WINC_ISR(void)
{
    WDRV_WINC_DCPT *const pDcpt = &wincDescriptor;

    OSAL_SEM_PostISR(&pDcpt->isrSemaphore);
}
