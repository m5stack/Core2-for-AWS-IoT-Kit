/*******************************************************************************
  WINC Driver Custom IE Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_custie.h

  Summary:
    WINC wireless driver custom IE header file.

  Description:
    This file provides an interface for manipulating the custom information
      element store. Custom IE's can be included in the Soft-AP beacons.
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

#ifndef _WDRV_WINC_CUSTIE_H
#define _WDRV_WINC_CUSTIE_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_winc_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Custom IE Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  Custom IE Structure

  Summary:
    Defines the format of a custom IE.

  Description:
    Custom IEs consist of an ID, length and data.

  Remarks:
    This definition does not allocate any storage for data.
*/

typedef struct
{
    /* ID. */
    uint8_t id;

    /* Length of data. */
    uint8_t length;

    /* Data. */
    uint8_t data[];
} WDRV_WINC_CUST_IE;

// *****************************************************************************
/*  Custom IE Store Structure

  Summary:
    Defines the storage used for holding custom IEs.

  Description:
    Custom IEs are past to the Soft-AP via the custom IE store which packages
      the IEs together.

  Remarks:
    None.
*/

typedef struct
{
    /* Maximum length of the IE store data. */
    uint8_t maxLength;

    /* Current length of data in the store. */
    uint8_t curLength;

    /* IE data in store. */
    uint8_t ieData[];
} WDRV_WINC_CUST_IE_STORE_CONTEXT;

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Custom IE Routines
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

  Precondition:
    None.

  Parameters:
    pStorage   - Pointer to storage to use for custom IE store.
    lenStorage - Length of storage pointed to by pStorage.

  Returns:
    Pointer to custom IE store, or NULL if error occurs.

  Remarks:
    lenStorage should be: (2 <= lenStorage <= 257). If less than 2 then an error
      will be signalled, if more that 257 only 257 bytes will be used.

*/

WDRV_WINC_CUST_IE_STORE_CONTEXT* WDRV_WINC_CustIEStoreCtxSetStorage
(
    uint8_t *const pStorage,
    uint16_t lenStorage
);

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

  Precondition:
    WDRV_WINC_CustIEStoreCtxSetStorage must have been called.

  Parameters:
    pCustIECtx - Pointer to custom IE store.
    id         - ID of custom IE.
    pData      - Pointer to data to be stored in the custom IE.
    dataLength - Length of data pointed to by pData.

  Returns:
    WDRV_WINC_STATUS_OK          - The data was added successfully.
    WDRV_WINC_STATUS_INVALID_ARG - The parameters were incorrect.
    WDRV_WINC_STATUS_NO_SPACE    - The data will not fit into the custom IE store.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_CustIEStoreCtxAddIE
(
    WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id,
    const uint8_t *const pData,
    uint8_t dataLength
);

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

  Precondition:
    WDRV_WINC_CustIEStoreCtxSetStorage must have been called.

  Parameters:
    pCustIECtx - Pointer to custom IE store.
    id         - ID of custom IE to remove.

  Returns:
    WDRV_WINC_STATUS_OK          - The data was added successfully.
    WDRV_WINC_STATUS_INVALID_ARG - The parameters were incorrect.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_CustIEStoreCtxRemoveIE
(
    WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id
);

#endif /* _WDRV_WINC_CUSTIE_H */
