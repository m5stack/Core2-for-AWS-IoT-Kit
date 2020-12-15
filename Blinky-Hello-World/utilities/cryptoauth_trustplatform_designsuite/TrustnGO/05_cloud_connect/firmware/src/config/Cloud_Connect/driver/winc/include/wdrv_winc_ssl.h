/*******************************************************************************
  WINC Driver TLS/SSL Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_ssl.h

  Summary:
    WINC wireless driver TLS/SSL header file.

  Description:
    Provides an interface to configure TLS/SSL support.
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

#ifndef _WDRV_WINC_SSL_H
#define _WDRV_WINC_SSL_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_winc_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver TLS/SSL Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* List of IANA cipher suite IDs

  Summary:
    List of IANA cipher suite IDs.

  Description:
    These defines list the IANA cipher suite IDs.

  Remarks:
    None.

*/

#define WDRV_WINC_TLS_NULL_WITH_NULL_NULL                       0x0000
#define WDRV_WINC_TLS_RSA_WITH_AES_128_CBC_SHA                  0x002f
#define WDRV_WINC_TLS_RSA_WITH_AES_128_CBC_SHA256               0x003c
#define WDRV_WINC_TLS_DHE_RSA_WITH_AES_128_CBC_SHA              0x0033
#define WDRV_WINC_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256           0x0067
#define WDRV_WINC_TLS_RSA_WITH_AES_128_GCM_SHA256               0x009c
#define WDRV_WINC_TLS_DHE_RSA_WITH_AES_128_GCM_SHA256           0x009e
#define WDRV_WINC_TLS_RSA_WITH_AES_256_CBC_SHA                  0x0035
#define WDRV_WINC_TLS_RSA_WITH_AES_256_CBC_SHA256               0x003d
#define WDRV_WINC_TLS_DHE_RSA_WITH_AES_256_CBC_SHA              0x0039
#define WDRV_WINC_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256           0x006b
#define WDRV_WINC_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA            0xc013
#define WDRV_WINC_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA            0xc014
#define WDRV_WINC_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256         0xc027
#define WDRV_WINC_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256       0xc023
#define WDRV_WINC_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256         0xc02f
#define WDRV_WINC_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256       0xc02b

// *****************************************************************************
/* The size of the the largest supported EC

  Summary:
    The size of the the largest supported EC. For now, assuming
    the 256-bit EC is the largest supported curve type.

  Description:
    These defines the size of the the largest supported EC.

  Remarks:
    None.

*/

#define ECC_LARGEST_CURVE_SIZE                      (32)

// *****************************************************************************
/* Maximum size of one coordinate of an EC point

  Summary:
    Maximum size of one coordinate of an EC point.

  Description:
    These defines the maximum size of one coordinate of an EC point.

  Remarks:
    None.

*/

#define ECC_POINT_MAX_SIZE                          ECC_LARGEST_CURVE_SIZE

// *****************************************************************************
/*  Elliptic Curve point representation

  Summary:
    Elliptic Curve point representation structure.

  Description:
    This structure contains information about Elliptic Curve point representation

  Remarks:
    None.
*/
typedef struct{
    /* The X-coordinate of the ec point. */
    uint8_t     x[ECC_POINT_MAX_SIZE];
    
    /* The Y-coordinate of the ec point. */
    uint8_t     y[ECC_POINT_MAX_SIZE];

    /* Point size in bytes (for each of the coordinates). */
    uint16_t    size;

    /* ID for the corresponding private key. */
    uint16_t    privKeyID;
}WDRV_WINC_EC_Point_Rep;

// *****************************************************************************
/*  ECDSA Verify Request Information

  Summary:
    ECDSA Verify Request Information structure.

  Description:
    This structure contains information about ECDSA verify request.

  Remarks:
    None.
*/

typedef struct
{
    uint32_t    nSig;
} WDRV_WINC_ECDSA_VERIFY_REQ_INFO;

// *****************************************************************************
/*  ECDSA Sign Request Information

  Summary:
    ECDSA Sign Request Information structure.

  Description:
    This structure contains information about ECDSA sign request.

  Remarks:
    None.
*/

typedef struct
{
    uint16_t    curveType;
    uint16_t    hashSz;

} WDRV_WINC_ECDSA_SIGN_REQ_INFO;


// *****************************************************************************
/*  ECDH Request Information

  Summary:
    ECDH Request Information structure.

  Description:
    This structure contains information about ECDH request from WINC.

  Remarks:
    None.
*/

typedef struct EcdhInfo
{
    WDRV_WINC_EC_Point_Rep pubKey;
    uint8_t     key[ECC_POINT_MAX_SIZE];

} WDRV_WINC_ECDH_REQ_INFO, WDRV_WINC_ECDH_RSP_INFO;

// *****************************************************************************
/*  ECC Request Information

  Summary:
    ECC Request Information structure.

  Description:
    This structure contains information about ECC request from WINC.

  Remarks:
    None.
*/

typedef struct
{
    uint16_t    reqCmd;
    uint16_t    status;
    uint32_t    userData;
    uint32_t    seqNo;
} WDRV_WINC_ECC_REQ_INFO;


// *****************************************************************************
/*  ECC Response Information

  Summary:
    ECC Response Information structure.

  Description:
    This structure contains information about ECC response to WINC.

  Remarks:
    None.
*/

typedef struct
{
    uint16_t    reqCmd;
    uint16_t    status;
    uint32_t    userData;
    uint32_t    seqNo;
    WDRV_WINC_ECDH_RSP_INFO ecdhRspInfo;
} WDRV_WINC_ECC_RSP_INFO;

// *****************************************************************************
/*  Cipher Suite Context

  Summary:
    Cipher suite context structure.

  Description:
    This structure contains information about cipher suites.

  Remarks:
    None.
*/

typedef struct
{
    /* Bit mask of cipher suites. */
    uint32_t ciperSuites;
} WDRV_WINC_CIPHER_SUITE_CONTEXT;

// *****************************************************************************
/*  Set Cipher Suite List Callback

  Summary:
    Callback to report cipher suites.

  Description:
    Called when the cipher suites are changed.

  Parameters:
    handle              - Client handle obtained by a call to WDRV_WINC_Open.
    pSSLCipherSuiteCtx  - Pointer to cipher suite list.

  Returns:
    None.

  Remarks:
    None.
*/

typedef void (*WDRV_WINC_SSL_CIPHERSUITELIST_CALLBACK)
(
    DRV_HANDLE handle,
    WDRV_WINC_CIPHER_SUITE_CONTEXT *pSSLCipherSuiteCtx
);

// *****************************************************************************
/*  SSL Request ECC Info Callback

  Summary:
    Callback to request ECC information.

  Description:
    Called when authentication with ECC Cipher suites.

  Parameters:
    handle              - Client handle obtained by a call to WDRV_WINC_Open.
    eccReqInfo          - ECC request info.
    pExtendInfo         - Extend Information 
                            (If the eccReqInfo.reqCmd is ECC_REQ_CLIENT_ECDH or ECC_REQ_SERVER_ECDH, pExtendInfo is WDRV_WINC_ECDH_REQ_INFO struct
 *                           If the eccReqInfo.reqCmd is ECC_REQ_SIGN_VERIFY, pExtendInfo is WDRV_WINC_ECDSA_VERIFY_REQ_INFO struct
 *                           If the eccReqInfo.reqCmd is ECC_REQ_SIGN_GEN, pExtendInfo is WDRV_WINC_ECDSA_SIGN_REQ_INFO struct   )
 
  Returns:
    None.

  Remarks:
    None.
*/

typedef void (*WDRV_WINC_SSL_REQ_ECC_CALLBACK)
(
    DRV_HANDLE handle,
    WDRV_WINC_ECC_REQ_INFO eccReqInfo,
    void*   pExtendInfo    
);

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver TLS/SSL Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_SSLCTXDefault
    (
        WDRV_WINC_CIPHER_SUITE_CONTEXT *pSSLCipherSuiteCtx
    )

  Summary:
    Initialize an SSL cipher suite context.

  Description:
    Sets the default values for the cipher suite context.

  Precondition:
    None.

  Parameters:
    pSSLCipherSuiteCtx - Pointer to cipher suite context.

  Returns:
    WDRV_WINC_STATUS_OK          - The operation was performed.
    WDRV_WINC_STATUS_INVALID_ARG - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_SSLCTXDefault
(
    WDRV_WINC_CIPHER_SUITE_CONTEXT *pSSLCipherSuiteCtx
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_SSLCTXCipherSuitesSet
    (
        WDRV_WINC_CIPHER_SUITE_CONTEXT *pSSLCipherSuiteCtx,
        uint16_t *pCipherSuiteList,
        uint8_t numCipherSuites
    )

  Summary:
    Set the cipher suites enabled in a context.

  Description:
    Sets the list of enabled cipher suites within a cipher suite context using
      IANA defined IDs.

  Precondition:
    None.

  Parameters:
    pSSLCipherSuiteCtx  - Pointer to cipher suite context.
    pCipherSuiteList    - Pointer to list of cipher suites to enable.
    numCipherSuites     - Number of cipher suites to enable.

  Returns:
    WDRV_WINC_STATUS_OK          - The operation was performed.
    WDRV_WINC_STATUS_INVALID_ARG - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_SSLCTXCipherSuitesSet
(
    WDRV_WINC_CIPHER_SUITE_CONTEXT *pSSLCipherSuiteCtx,
    uint16_t *pCipherSuiteList,
    uint8_t numCipherSuites
);

//*******************************************************************************
/*
  Function:
    uint8_t WDRV_WINC_SSLCTXCipherSuitesGet
    (
        WDRV_WINC_CIPHER_SUITE_CONTEXT *pSSLCipherSuiteCtx,
        uint16_t *pCipherSuiteList,
        uint8_t maxNumCipherSuites
    )

  Summary:
    Returns the enabled cipher suites from a context.

  Description:
    Gets the list of cipher suites from a context as a list of IANA IDs

  Precondition:
    None.

  Parameters:
    pSSLCipherSuiteCtx  - Pointer to cipher suite context.
    pCipherSuiteList    - Pointer to list of cipher suites to populate.
    maxNumCipherSuites  - Maximum number of cipher suites to populate in list.

  Returns:
    WDRV_WINC_STATUS_OK          - The operation was performed.
    WDRV_WINC_STATUS_INVALID_ARG - The parameters were incorrect.

  Remarks:
    None.

*/

uint8_t WDRV_WINC_SSLCTXCipherSuitesGet
(
    WDRV_WINC_CIPHER_SUITE_CONTEXT *pSSLCipherSuiteCtx,
    uint16_t *pCipherSuiteList,
    uint8_t maxNumCipherSuites
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_SSLActiveCipherSuitesSet
    (
        DRV_HANDLE handle,
        WDRV_WINC_CIPHER_SUITE_CONTEXT *pSSLCipherSuiteCtx,
        WDRV_WINC_SSL_CIPHERSUITELIST_CALLBACK pfSSLListCallback,
        WDRV_WINC_REQ_ECC_CALLBACK pfECCREQCallback
    )

  Summary:
    Set the active cipher suites.

  Description:
    Passes the active cipher suite list to the WINC.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle             - Client handle obtained by a call to WDRV_WINC_Open.
    pSSLCipherSuiteCtx - Pointer to cipher suite context.
    pfSSLListCallback  - Pointer to callback function to receive updated list of
                           cipher suites.
    pfECCREQCallback   - Pointer to callback function to receive ECC request information from WINC
                           This value can be NULL if ChiperSuites are not ECC.

  Returns:
    WDRV_WINC_STATUS_OK             - The operation was performed.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_SSLActiveCipherSuitesSet
(
    DRV_HANDLE handle,
    WDRV_WINC_CIPHER_SUITE_CONTEXT *pSSLCipherSuiteCtx,
    WDRV_WINC_SSL_CIPHERSUITELIST_CALLBACK pfSSLListCallback,
    WDRV_WINC_SSL_REQ_ECC_CALLBACK pfECCREQCallback
);


//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_SSLECCHandShakeRsp
    (
        WDRV_WINC_ECC_RSP_INFO eccRsp,
        uint8_t *pRspDataBuff, 
        uint16_t rspDataSz
    )

  Summary:
    Handshake Response for ECC cipher suites .

  Description:
    Passes the ECC response data to WINC.

  Precondition:
    None

  Parameters:
    eccRsp              - ECC response structure.
    pu8RspDataBuff      - Pointer of the response data to be sent.
    u16RspDataSz        - Response data size.
                           
  Returns:
    WDRV_WINC_STATUS_OK             - The operation was performed.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The operation is not success

  Remarks:
    None.

*/
WDRV_WINC_STATUS WDRV_WINC_SSLECCHandShakeRsp
(
    WDRV_WINC_ECC_RSP_INFO eccRsp,
    uint8_t *pRspDataBuff, 
    uint16_t rspDataSz
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_SSLRetrieveCert
    (
        uint16_t *pCurveType, 
        uint8_t *pHash, 
        uint8_t *pSig, 
        WDRV_WINC_EC_Point_Rep *pKey
    )

  Summary:
    Retrieve the certificate to be verified from the WINC

  Description:
    Retrieve the certificate to be verified from the WINC

  Precondition:
    None

  Parameters:
    pCurveType   - Pointer to the certificate curve type.
    pHash         - Pointer to the certificate hash.
    pSig          - Pointer to the certificate signature.
    pKey          - Pointer to the certificate Key.
                           
  Returns:
    WDRV_WINC_STATUS_OK             - The operation was performed.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The operation is not success

  Remarks:
    None.

*/
WDRV_WINC_STATUS WDRV_WINC_SSLRetrieveCert
(
    uint16_t *pCurveType, 
    uint8_t *pHash, 
    uint8_t *pSig, 
    WDRV_WINC_EC_Point_Rep* pKey
);
        
//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_SSLRetrieveHash
    (
        uint8_t *pHash, 
        uint16_t hashSz
    )

  Summary:
    Retrieve the certificate hash

  Description:
    Retrieve the certificate hash from the WINC

  Precondition:
    None

  Parameters:
    pHash           - Pointer to the certificate hash.
    hashSz          - Hash size.
    
                           
  Returns:
    WDRV_WINC_STATUS_OK             - The operation was performed.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The operation is not success

  Remarks:
    None.

*/
WDRV_WINC_STATUS WDRV_WINC_SSLRetrieveHash
(
    uint8_t	*pHash, 
    uint16_t hashSz
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_SSLStopRetrieveCert(void)

  Summary:
    Stop processing the certificate

  Description:
    Stop processing the certificate

  Precondition:
    None

  Parameters:
    None
    
                           
  Returns:
    WDRV_WINC_STATUS_OK             - The operation was performed.

  Remarks:
    None.

*/
WDRV_WINC_STATUS WDRV_WINC_SSLStopRetrieveCert(void);
#endif /* _WDRV_WINC_SSL_H */
