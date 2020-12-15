/*******************************************************************************
  WINC Driver Authentication Context Implementation

  File Name:
    wdrv_winc_authctx.c

  Summary:
    WINC wireless driver authentication context implementation.

  Description:
    This interface manages the authentication contexts which 'wrap' the state
      associated with authentication schemes.
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

#include "wdrv_winc_common.h"
#include "wdrv_winc_authctx.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Authentication Context Global Data
// *****************************************************************************
// *****************************************************************************

/* ASN.1 tag IDs. */
#define ASN1_TAG_SEQUENCE   (0x30)
#define ASN1_TAG_INTEGER    (0x02)

// *****************************************************************************
/*  Address and length structure

  Summary:
    Holds an address and length.

  Description:
    Structure used to maintain an address and length to some data.

  Remarks:
    None.
*/

typedef struct
{
    const uint8_t *pAddress;
    uint32_t length;
} ADDR_LENGTH;

// *****************************************************************************
/*  PKCS#1 RSA Private Key

  Summary:
    Elements of a parsed RSA private key.

  Description:
    A series of pointer/length of each element of an RSA private key.

  Remarks:
    None.
*/

typedef struct
{
    ADDR_LENGTH version;
    ADDR_LENGTH modulus;
    ADDR_LENGTH publicExponent;
    ADDR_LENGTH privateExponent;
    ADDR_LENGTH prime1;
    ADDR_LENGTH prime2;
    ADDR_LENGTH exponent1;
    ADDR_LENGTH exponent2;
    ADDR_LENGTH coefficient;
} PKCS1_RSA_PRIVATE_KEY;

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Authentication Context Implementation
// *****************************************************************************
// *****************************************************************************

#ifdef WDRV_WINC_DEVICE_ENTERPRISE_CONNECT
static int asn1DecodeTag(const uint8_t *pASN1, uint8_t *pTag, uint32_t *pLength,
                                                    const uint8_t **pContents)
{
    int numTagBytes;

    if (NULL != pTag)
        *pTag = *pASN1;
    pASN1++;
    numTagBytes = 1;

    if (*pASN1 & 0x80)
    {
        int numLenBytes;

        numLenBytes = (*pASN1 & 0x7f);

        pASN1++;
        numTagBytes += (1+numLenBytes);

        if (NULL != pLength)
        {
            *pLength = 0;
            while(numLenBytes--)
            {
                *pLength = (*pLength << 8) | *pASN1++;
            }
        }
        else
        {
            pASN1 += numLenBytes;
        }
    }
    else
    {
        numTagBytes++;
        *pLength = *pASN1++;
    }

    numTagBytes += *pLength;

    if (NULL != pContents)
    {
        *pContents = pASN1;
    }

    return numTagBytes;
}

static void PKCS1_ParseRSAPrivateKeyDER(const uint8_t *pKey, size_t keyLength,
                                        PKCS1_RSA_PRIVATE_KEY * const pPrivateKey)
{
    uint8_t tagID;
    uint32_t tagLength;
    const uint8_t *pTagContents;
    int numTagBytes;
    int tagIndex = 0;

/*
    RSAPrivateKey ::= SEQUENCE {
        version           Version,
        modulus           INTEGER,  -- n
        publicExponent    INTEGER,  -- e
        privateExponent   INTEGER,  -- d
        prime1            INTEGER,  -- p
        prime2            INTEGER,  -- q
        exponent1         INTEGER,  -- d mod (p-1)
        exponent2         INTEGER,  -- d mod (q-1)
        coefficient       INTEGER,  -- (inverse of q) mod p
        otherPrimeInfos   OtherPrimeInfos OPTIONAL
    }
*/

    if ((NULL == pKey) || (NULL == pPrivateKey))
    {
        return;
    }

    memset(pPrivateKey, 0, sizeof(PKCS1_RSA_PRIVATE_KEY));

    numTagBytes = asn1DecodeTag(pKey, &tagID, &tagLength, &pTagContents);

    if (ASN1_TAG_SEQUENCE != tagID)
    {
        // Not SEQUENCE
        return;
    }

    pKey = pTagContents;
    keyLength = tagLength;

    do
    {
        numTagBytes = asn1DecodeTag(pKey, &tagID, &tagLength, &pTagContents);

        if (ASN1_TAG_INTEGER == tagID)
        {
            if ((tagLength > 1)
                && (0x00 == pTagContents[0]) && (0x00 != (pTagContents[1] & 0x80)))
            {
                pTagContents++;
                tagLength--;
            }

            switch (tagIndex++)
            {
                case 0:
                {
                    pPrivateKey->version.pAddress = pTagContents;
                    pPrivateKey->version.length   = tagLength;
                    break;
                }

                case 1:
                {
                    pPrivateKey->modulus.pAddress = pTagContents;
                    pPrivateKey->modulus.length   = tagLength;
                    break;
                }

                case 2:
                {
                    pPrivateKey->publicExponent.pAddress = pTagContents;
                    pPrivateKey->publicExponent.length   = tagLength;
                    break;
                }

                case 3:
                {
                    pPrivateKey->privateExponent.pAddress = pTagContents;
                    pPrivateKey->privateExponent.length   = tagLength;
                    break;
                }

                case 4:
                {
                    pPrivateKey->prime1.pAddress = pTagContents;
                    pPrivateKey->prime1.length   = tagLength;
                    break;
                }

                case 5:
                {
                    pPrivateKey->prime2.pAddress = pTagContents;
                    pPrivateKey->prime2.length   = tagLength;
                    break;
                }

                case 6:
                {
                    pPrivateKey->exponent1.pAddress = pTagContents;
                    pPrivateKey->exponent1.length   = tagLength;
                    break;
                }

                case 7:
                {
                    pPrivateKey->exponent2.pAddress = pTagContents;
                    pPrivateKey->exponent2.length   = tagLength;
                    break;
                }

                case 8:
                {
                    pPrivateKey->coefficient.pAddress = pTagContents;
                    pPrivateKey->coefficient.length   = tagLength;
                    break;
                }
            }
        }

        pKey += numTagBytes;
        keyLength -= numTagBytes;
    }
    while(keyLength);
}
#endif

//*******************************************************************************
/*
  Function:
    bool WDRV_WINC_AuthCtxIsValid(const WDRV_WINC_AUTH_CONTEXT *const pAuthCtx)

  Summary:
    Tests if an authentication context is valid.

  Description:
    Tests the elements of the authentication context to judge if their values are legal.

  Remarks:
    See wdrv_winc_authctx.h for usage information.

*/

bool WDRV_WINC_AuthCtxIsValid(const WDRV_WINC_AUTH_CONTEXT *const pAuthCtx)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return false;
    }

    switch (pAuthCtx->authType)
    {
        /* Nothing to check for Open authentication. */
        case WDRV_WINC_AUTH_TYPE_OPEN:
        {
            break;
        }

        /* Check WPA authentication. */
        case WDRV_WINC_AUTH_TYPE_WPA_PSK:
        {
            break;
        }

        /* Check WEP authentication. */
        case WDRV_WINC_AUTH_TYPE_WEP:
        {
            /* Ensure WEP index and key size is valid. */
            if ((pAuthCtx->authInfo.WEP.idx > 4) ||
               (  (WEP_40_KEY_STRING_SIZE != pAuthCtx->authInfo.WEP.size) &&
                  (WEP_104_KEY_STRING_SIZE != pAuthCtx->authInfo.WEP.size)))
            {
                return false;
            }
            break;
        }

        /* Check Enterprise authentication. */
        case WDRV_WINC_AUTH_TYPE_802_1X:
        {
            break;
        }

        /* Unknown authentication scheme. */
        default:
        {
            return false;
        }
    }

    return true;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetDefaults
    (
        WDRV_WINC_AUTH_CONTEXT *const pAuthCtx
    )

  Summary:
    Configures an authentication context into a default state.

  Description:
    Ensures that each element of the structure is configured into a default state.

  Remarks:
    See wdrv_winc_authctx.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetDefaults
(
    WDRV_WINC_AUTH_CONTEXT *const pAuthCtx
)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure authentication type is a known invalid type. */
    pAuthCtx->authType = WDRV_WINC_AUTH_TYPE_INVALID;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetOpen
    (
        WDRV_WINC_AUTH_CONTEXT *const pAuthCtx
    )

  Summary:
    Configure an authentication context for Open authentication.

  Description:
    The type and state information are configured appropriately for Open
      authentication.

  Remarks:
    See wdrv_winc_authctx.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetOpen
(
    WDRV_WINC_AUTH_CONTEXT *const pAuthCtx
)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Set authentication type to Open. */
    pAuthCtx->authType = WDRV_WINC_AUTH_TYPE_OPEN;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetWEP
    (
        WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
        uint8_t idx,
        uint8_t *pKey,
        uint8_t size
    )

  Summary:
    Configure an authentication context for WEP authentication.

  Description:
    The type and state information are configured appropriately for WEP
      authentication.

  Remarks:
    See wdrv_winc_authctx.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetWEP
(
    WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
    uint8_t idx,
    uint8_t *const pKey,
    uint8_t size
)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the index and key size are valid. */
    if ((idx > 4) ||
        ((WEP_40_KEY_STRING_SIZE != size) && (WEP_104_KEY_STRING_SIZE != size))
        || (NULL == pKey))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Set authentication type to WEP. */
    pAuthCtx->authType = WDRV_WINC_AUTH_TYPE_WEP;

    /* Set key index and size. */
    pAuthCtx->authInfo.WEP.idx  = idx;
    pAuthCtx->authInfo.WEP.size = size;

    /* Copy WEP key and ensure zero terminated. */
    memcpy(&pAuthCtx->authInfo.WEP.key, pKey, size);
    pAuthCtx->authInfo.WEP.key[size] = '\0';

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetWPA
    (
        WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
        uint8_t *const pPSK,
        uint8_t size
    )

  Summary:
    Configure an authentication context for WPA-PSK authentication.

  Description:
    The type and state information are configured appropriately for WPA-PSK
      authentication.

  Remarks:
    See wdrv_winc_authctx.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetWPA
(
    WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
    uint8_t *const pPSK,
    uint8_t size
)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the key size is correct and the key was provided. */
    if ((size >= M2M_MAX_PSK_LEN) || (NULL == pPSK))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Set authentication type to WPA. */
    pAuthCtx->authType = WDRV_WINC_AUTH_TYPE_WPA_PSK;

    /* Copy the key and zero out unused parts of the buffer. */
    memset(&pAuthCtx->authInfo.WPAPerPSK.key, 0, M2M_MAX_PSK_LEN);
    memcpy(&pAuthCtx->authInfo.WPAPerPSK.key, pPSK, size);
    pAuthCtx->authInfo.WPAPerPSK.size = size;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetWPAEnterpriseMSCHAPv2
    (
        WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
        const char *const pUserDomain,
        const uint8_t *const pPassword,
        const uint16_t passwordLength,
        const bool visibleUserName
    )

  Summary:
    Configure an authentication context for WPA(2)-Enterprise authentication
    using MS-CHAPv2.

  Description:
    The type and state information are configured appropriately for WPA-Enterprise
      authentication.

  Remarks:
    See wdrv_winc_authctx.h for usage information.

*/

#ifdef WDRV_WINC_DEVICE_ENTERPRISE_CONNECT
WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetWPAEnterpriseMSCHAPv2
(
    WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
    const char *const pUserDomain,
    const uint8_t *const pPassword,
    const uint16_t passwordLength,
    const bool visibleUserName
)
{
    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    if ((NULL == pPassword) || (NULL == pUserDomain))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    if ((passwordLength > M2M_AUTH_1X_PASSWORD_LEN_MAX) ||
                                (strlen(pUserDomain) > M2M_AUTH_1X_USER_LEN_MAX))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Set authentication type to WPA. */
    pAuthCtx->authType = WDRV_WINC_AUTH_TYPE_802_1X_MSCHAPV2;

    memset(&pAuthCtx->authInfo.WPAEntMSCHAPv2.domainUserName, 0, M2M_AUTH_1X_USER_LEN_MAX+1);
    memcpy(&pAuthCtx->authInfo.WPAEntMSCHAPv2.domainUserName, pUserDomain, strlen(pUserDomain));
    memset(&pAuthCtx->authInfo.WPAEntMSCHAPv2.password, 0, M2M_AUTH_1X_PASSWORD_LEN_MAX);
    memcpy(&pAuthCtx->authInfo.WPAEntMSCHAPv2.password, pPassword, passwordLength);

    pAuthCtx->authInfo.WPAEntMSCHAPv2.passwordLength = passwordLength;

    if (NULL != strchr(pUserDomain, '\\'))
    {
        pAuthCtx->authInfo.WPAEntMSCHAPv2.domainIsFirst = true;
    }
    else
    {
        pAuthCtx->authInfo.WPAEntMSCHAPv2.domainIsFirst = false;
    }

    pAuthCtx->authInfo.WPAEntMSCHAPv2.visibleUserName = visibleUserName;

    return WDRV_WINC_STATUS_OK;
}
#endif

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetWPAEnterpriseTLS
    (
        WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
        const char *const pUserDomain,
        const uint8_t *const pPrivateKey,
        const uint16_t privateKeyLength,
        const uint8_t *const pCert,
        const uint16_t certLength,
        const bool visibleUserName
    )

  Summary:
    Configure an authentication context for WPA(2)-Enterprise authentication
    using TLS.

  Description:
    The type and state information are configured appropriately for WPA-Enterprise
      authentication.

  Remarks:
    See wdrv_winc_authctx.h for usage information.

*/

#ifdef WDRV_WINC_DEVICE_ENTERPRISE_CONNECT
WDRV_WINC_STATUS WDRV_WINC_AuthCtxSetWPAEnterpriseTLS
(
    WDRV_WINC_AUTH_CONTEXT *const pAuthCtx,
    const char *const pUserDomain,
    const uint8_t *const pPrivateKey,
    const uint16_t privateKeyLength,
    const uint8_t *const pCert,
    const uint16_t certLength,
    const bool visibleUserName
)
{
    PKCS1_RSA_PRIVATE_KEY privateKey;

    /* Ensure authentication context is valid. */
    if (NULL == pAuthCtx)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    if ((NULL == pPrivateKey) || (NULL == pCert) || (NULL == pUserDomain))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    PKCS1_ParseRSAPrivateKeyDER(pPrivateKey, privateKeyLength, &privateKey);

    /* Set authentication type to WPA. */
    pAuthCtx->authType = WDRV_WINC_AUTH_TYPE_802_1X_TLS;

    memset(&pAuthCtx->authInfo.WPAEntTLS.domainUserName, 0, M2M_AUTH_1X_USER_LEN_MAX+1);
    memcpy(&pAuthCtx->authInfo.WPAEntTLS.domainUserName, pUserDomain, strlen(pUserDomain));

    pAuthCtx->authInfo.WPAEntTLS.pRSAPrivateKeyModulus      = privateKey.modulus.pAddress;
    pAuthCtx->authInfo.WPAEntTLS.RSAPrivateKeyModulusLength = privateKey.modulus.length;

    pAuthCtx->authInfo.WPAEntTLS.pRSAPrivateKeyExponent = privateKey.privateExponent.pAddress;

    pAuthCtx->authInfo.WPAEntTLS.pRSACertificate      = pCert;
    pAuthCtx->authInfo.WPAEntTLS.RSACertificateLength = certLength;

    if (NULL != strchr(pUserDomain, '\\'))
    {
        pAuthCtx->authInfo.WPAEntTLS.domainIsFirst = true;
    }
    else
    {
        pAuthCtx->authInfo.WPAEntTLS.domainIsFirst = false;
    }

    pAuthCtx->authInfo.WPAEntTLS.visibleUserName = visibleUserName;

    return WDRV_WINC_STATUS_OK;
}
#endif