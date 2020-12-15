/*******************************************************************************
  WINC Driver HTTP Provision Context Implementation

  File Name:
    wdrv_winc_httpprovctx.c

  Summary:
    WINC wireless driver HTTP provisioning context implementation.

  Description:
    This interface manages the HTTP provisioning context which is used to
      define the behaviour of the HTTP provisioning provided by Soft-AP mode.
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

#include "wdrv_winc_common.h"
#include "wdrv_winc_httpprovctx.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver HTTP Provisioning Context Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_HTTPProvCtxSetDefaults
    (
        WDRV_WINC_HTTP_PROV_CONTEXT *const pHTTPProvCtx
    )

  Summary:
    Initialize provisioning store.

  Description:
    Ensure the provisioning store is initialized to appropriate defaults.

  Remarks:
    See wdrv_winc_httpprovctx.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_HTTPProvCtxSetDefaults
(
    WDRV_WINC_HTTP_PROV_CONTEXT *const pHTTPProvCtx
)
{
    /* Ensure provisioning context is valid. */
    if (NULL == pHTTPProvCtx)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Set the context to no domain, with wild card URL mapping and no callback. */
    pHTTPProvCtx->wildcardURL         = true;
    pHTTPProvCtx->domainName[0]       = '\0';
    pHTTPProvCtx->pfProvConnectInfoCB = NULL;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_HTTPProvCtxSetDomainName
    (
        WDRV_WINC_HTTP_PROV_CONTEXT *const pHTTPProvCtx,
        const char *const pname
    )

  Summary:
    Define the DNS domain name for the provisioning server.

  Description:
    This defines the DNS domain name associated with the Soft-AP instance web server.

  Remarks:
    See wdrv_winc_httpprovctx.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_HTTPProvCtxSetDomainName
(
    WDRV_WINC_HTTP_PROV_CONTEXT *const pHTTPProvCtx,
    const char *const pname
)
{
    size_t nameLength;

    /* Ensure provisioning context is valid. */
    if (NULL == pHTTPProvCtx)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Check length of domain name. */
    nameLength = strlen(pname);

    if (nameLength > 64)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Copy domain name into context ensuring unused space is zeroed. */
    memset(&pHTTPProvCtx->domainName, 0, 64+1);
    memcpy(pHTTPProvCtx->domainName, pname, nameLength);

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_HTTPProvCtxSetURLWildcard
    (
        WDRV_WINC_HTTP_PROV_CONTEXT *const pHTTPProvCtx,
        bool wildcardURL
    )

  Summary:
    Define the use of a wildcard URL check.

  Description:
    Indicates if the Soft-AP should map all DNS domain names to the server to
      capture the clients device.

  Remarks:
    See wdrv_winc_httpprovctx.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_HTTPProvCtxSetURLWildcard
(
    WDRV_WINC_HTTP_PROV_CONTEXT *const pHTTPProvCtx,
    bool wildcardURL
)
{
    /* Ensure provisioning context is valid. */
    if (NULL == pHTTPProvCtx)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Set wild card flag. */
    pHTTPProvCtx->wildcardURL = wildcardURL;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_HTTPProvCtxSetConnectCallback
    (
        WDRV_WINC_HTTP_PROV_CONTEXT *const pHTTPProvCtx,
        const WDRV_WINC_HTTPPROV_INFO_CALLBACK pfProvConnectInfoCB
    )

  Summary:
    Defines the connection information callback.

  Description:
    Associates the function provided with the provisioning context. The callback
      will be used once the client has provided credentials to be used.

  Remarks:
    See wdrv_winc_httpprovctx.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_HTTPProvCtxSetConnectCallback
(
    WDRV_WINC_HTTP_PROV_CONTEXT *const pHTTPProvCtx,
    const WDRV_WINC_HTTPPROV_INFO_CALLBACK pfProvConnectInfoCB
)
{
    /* Ensure provisioning context is valid. */
    if (NULL == pHTTPProvCtx)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Set the provision callback. */
    pHTTPProvCtx->pfProvConnectInfoCB = pfProvConnectInfoCB;

    return WDRV_WINC_STATUS_OK;
}
