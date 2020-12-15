/*******************************************************************************
  WINC Driver Custom IE Implementation

  File Name:
    wdrv_winc_custie.c

  Summary:
    WINC wireless driver custom IE implementation.

  Description:
    This file provides an interface for manipulating the custom information
      element store. Custom IE's can be included in the Soft-AP beacons.
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
#include "wdrv_winc_custie.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Custom IE Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_CUST_IE_STORE_CONTEXT* WDRV_WINC_CustIEStoreCtxSetStorage
    (
        uint8_t *const pStorage,
        uint16_t lenStorage
    )

  Summary:
    Initialize the custom IE store.

  Description:
    The caller provides storage for the custom IE store, this will be initialized
      and a pointer provided which can be passed to WDRV_WINC_APSetCustIE after
      custom IEs are added by WDRV_WINC_CustIEStoreCtxAddIE.

  Remarks:
    See wdrv_winc_custie.h for usage information.

*/

WDRV_WINC_CUST_IE_STORE_CONTEXT* WDRV_WINC_CustIEStoreCtxSetStorage
(
    uint8_t *const pStorage,
    uint16_t lenStorage
)
{
    WDRV_WINC_CUST_IE_STORE_CONTEXT *pCustIECtx;

    /* Ensure user application provided storage is valid. */
    if ((NULL == pStorage) || (lenStorage < 2))
    {
        return NULL;
    }

    /* Initialise the storage area. */

    pCustIECtx = (WDRV_WINC_CUST_IE_STORE_CONTEXT*)pStorage;

    memset(pStorage, 0, lenStorage);

    pCustIECtx->maxLength = (lenStorage > (255+2)) ? 255 : (lenStorage - 2);

    return pCustIECtx;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_CustIEStoreCtxAddIE
    (
        WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx,
        uint8_t id,
        const uint8_t *const pData,
        uint8_t dataLength
    )

  Summary:
    Add data to the custom IE store.

  Description:
    The data and ID provided are copied into the custom IE store.

  Remarks:
    See wdrv_winc_custie.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_CustIEStoreCtxAddIE
(
    WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id,
    const uint8_t *const pData,
    uint8_t dataLength
)
{
    WDRV_WINC_CUST_IE *pIE;
    uint8_t dataOffset;

    /* Ensure the storage context is valid. */
    if (NULL == pCustIECtx)
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the new IE will fit. */
    if ((pCustIECtx->maxLength - pCustIECtx->curLength) < (dataLength + 2))
    {
        return WDRV_WINC_STATUS_NO_SPACE;
    }

    /* Walk the IEs until the end is found. */
    dataOffset = 0;
    pIE = (WDRV_WINC_CUST_IE*)&pCustIECtx->ieData[dataOffset];

    while((0 != pIE->id) && (dataOffset < pCustIECtx->curLength))
    {
        dataOffset += (2+pIE->length);
        pIE = (WDRV_WINC_CUST_IE*)&pCustIECtx->ieData[dataOffset];
    }

    /* Copy in new IE. */
    pIE->id = id;
    pIE->length = dataLength;
    memcpy(&pIE->data, pData, dataLength);

    pCustIECtx->curLength += (dataLength + 2);

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_CustIEStoreCtxRemoveIE
    (
        WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx,
        uint8_t id
    )

  Summary:
    Removes data from the custom IE store.

  Description:
    This function removes a custom IE from the store which matches the ID provided.

  Remarks:
    See wdrv_winc_custie.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_CustIEStoreCtxRemoveIE
(
    WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id
)
{
    WDRV_WINC_CUST_IE *pIE;
    uint8_t dataOffset;

    /* Ensure the storage context and ID are valid. */
    if ((NULL == pCustIECtx) || (0 == id))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Walk the IEs looking for the supplied ID. */
    dataOffset = 0;
    pIE = (WDRV_WINC_CUST_IE*)&pCustIECtx->ieData[dataOffset];

    while((0 != pIE->id) && (dataOffset < pCustIECtx->curLength))
    {
        if (pIE->id == id)
        {
            /* The ID has been found, copy remaining IEs over the top to remove it. */
            pCustIECtx->curLength -= (2+pIE->length);

            memcpy(&pCustIECtx->ieData[dataOffset],
                    &pCustIECtx->ieData[dataOffset+(2+pIE->length)],
                    pCustIECtx->maxLength - dataOffset - (2+pIE->length));

            pIE = (WDRV_WINC_CUST_IE*)&pCustIECtx->ieData[pCustIECtx->curLength];

            pIE->id = 0;
            pIE->length = 0;
            break;
        }

        dataOffset += (2+pIE->length);
        pIE = (WDRV_WINC_CUST_IE*)&pCustIECtx->ieData[dataOffset];
    }

    return WDRV_WINC_STATUS_OK;
}
