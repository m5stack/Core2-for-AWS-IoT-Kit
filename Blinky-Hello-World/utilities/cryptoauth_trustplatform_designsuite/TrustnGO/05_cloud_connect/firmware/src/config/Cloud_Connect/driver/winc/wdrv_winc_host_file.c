/*******************************************************************************
  WINC Driver Host File Download Implementation

  File Name:
    wdrv_winc_host_file.c

  Summary:
    WINC wireless driver host file download implementation.

  Description:
    This file provides an interface for downloading files into the WINCs
      internal flash memory for later retrieval by the host MCU.
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

#include "wdrv_winc.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc_host_file.h"
#include "m2m_ota.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Host File Download Implementations
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_HostFileDownload
    (
        DRV_HANDLE handle,
        const char *pFileURL,
        const WDRV_WINC_HOST_FILE_STATUS_CALLBACK pfHostFileGetCB
    );

  Summary:
    Request the WINC downloads a file to it's internal flash memory.

  Description:
    Makes a request to the WINC device to begin downloading the specified
      file from the URL provided to the internal flash memory of the WINC device.

  Remarks:
    See wdrv_winc_host_flle.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_HostFileDownload
(
    DRV_HANDLE handle,
    const char *pFileURL,
    const WDRV_WINC_HOST_FILE_STATUS_CALLBACK pfHostFileGetCB
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle and file URL are valid. */
    if ((NULL == pDcpt) || (NULL == pFileURL) || (NULL == pfHostFileGetCB))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure WINC is connected. */
    if (false == pDcpt->isConnected)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    pDcpt->hostFileDcpt.handle = HFD_INVALID_HANDLER;
    pDcpt->hostFileDcpt.size   = 0;

    if (M2M_ERR_FAIL == m2m_ota_host_file_get((char*)pFileURL,
                                                pDcpt->hostFileDcpt.getFileCB))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    pDcpt->pfHostFileCB = pfHostFileGetCB;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_HostFileRead
    (
        DRV_HANDLE handle,
        WDRV_WINC_HOST_FILE_HANDLE fileHandle,
        void *pBuffer,
        uint32_t offset,
        uint32_t size,
        const WDRV_WINC_HOST_FILE_STATUS_CALLBACK pfHostFileReadStatusCB
    );

  Summary:
    Requests to read the data from a downloaded file.

  Description:
    Makes a request to the WINC device to provide the data from a file
      previously downloaded into internal flash memory.

  Remarks:
    See wdrv_winc_host_flle.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_HostFileRead
(
    DRV_HANDLE handle,
    WDRV_WINC_HOST_FILE_HANDLE fileHandle,
    void *pBuffer,
    uint32_t offset,
    uint32_t size,
    const WDRV_WINC_HOST_FILE_STATUS_CALLBACK pfHostFileReadStatusCB
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;
    WDRV_WINC_HOST_FILE_DCPT *const pHostFileDcpt
                                    = (WDRV_WINC_HOST_FILE_DCPT *const)fileHandle;
    uint32_t hifSize;

    /* Ensure the driver handle and file handle are valid. */
    if ((NULL == pDcpt) || (NULL == pHostFileDcpt))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure the file handle is valid. */
    if ((pHostFileDcpt != &pDcpt->hostFileDcpt) ||
                                (HFD_INVALID_HANDLER == pHostFileDcpt->handle))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    if (NULL != pBuffer)
    {
        pHostFileDcpt->pBuffer = pBuffer;
        pHostFileDcpt->offset  = offset;
        pHostFileDcpt->remain  = size;
    }

    if ((0 != pHostFileDcpt->size) && (pHostFileDcpt->offset >= pHostFileDcpt->size))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    hifSize = pHostFileDcpt->remain;
    if (hifSize > 128)
    {
        hifSize = 128;
    }

    if (M2M_ERR_FAIL == m2m_ota_host_file_read_hif(pHostFileDcpt->handle,
                pHostFileDcpt->offset, hifSize, pDcpt->hostFileDcpt.readFileCB))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    pDcpt->pfHostFileCB = pfHostFileReadStatusCB;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_HostFileErase
    (
        DRV_HANDLE handle,
        WDRV_WINC_HOST_FILE_HANDLE fileHandle,
        const WDRV_WINC_HOST_FILE_STATUS_CALLBACK pfHostFileEraseStatusCB
    );

  Summary:
    Requests to erase a downloaded file.

  Description:
    Makes a request to the WINC device to erase the specified file from it's
      internal flash memory.

  Remarks:
    See wdrv_winc_host_flle.h for usage information.

*/

WDRV_WINC_STATUS WDRV_WINC_HostFileErase
(
    DRV_HANDLE handle,
    WDRV_WINC_HOST_FILE_HANDLE fileHandle,
    const WDRV_WINC_HOST_FILE_STATUS_CALLBACK pfHostFileEraseStatusCB
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;
    WDRV_WINC_HOST_FILE_DCPT *const pHostFileDcpt
                                = (WDRV_WINC_HOST_FILE_DCPT *const)fileHandle;

    /* Ensure the driver handle and file handle are valid. */
    if ((NULL == pDcpt) || (NULL == pHostFileDcpt))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure the file handle is valid. */
    if ((pHostFileDcpt != &pDcpt->hostFileDcpt) ||
                                (HFD_INVALID_HANDLER == pHostFileDcpt->handle))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    if (M2M_ERR_FAIL == m2m_ota_host_file_erase(pHostFileDcpt->handle,
                                                    pHostFileDcpt->eraseFileCB))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    pDcpt->pfHostFileCB = pfHostFileEraseStatusCB;

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_HOST_FILE_HANDLE WDRV_WINC_HostFileFindByID
    (
        DRV_HANDLE handle,
        uint8_t id
    );

  Summary:
    Maps a file ID to a driver file handle.

  Description:
    The WINC uses a file handle ID to refer to files, this function maps
      that ID to a file handle object which can be used with the WINC driver API.

  Remarks:
    See wdrv_winc_host_flle.h for usage information.

*/

WDRV_WINC_HOST_FILE_HANDLE WDRV_WINC_HostFileFindByID
(
    DRV_HANDLE handle,
    uint8_t id
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;

    /* Ensure the driver handle and file handle are valid. */
    if ((NULL == pDcpt) || (HFD_INVALID_HANDLER == id))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    if (HFD_INVALID_HANDLER == pDcpt->hostFileDcpt.handle)
    {
        pDcpt->hostFileDcpt.handle = m2m_ota_host_file_get_id();
    }

    if (id == pDcpt->hostFileDcpt.handle)
    {
        return (WDRV_WINC_HOST_FILE_HANDLE)&pDcpt->hostFileDcpt;
    }

    return WDRV_WINC_HOST_FILE_INVALID_HANDLE;
}

//*******************************************************************************
/*
  Function:
    uint32_t WDRV_WINC_HostFileGetSize(const WDRV_WINC_HOST_FILE_HANDLE fileHandle);

  Summary:
    Retrieves the file size from a file handle.

  Description:
    Given a driver file handle this function will retrieve the file size in bytes.

  Remarks:
    See wdrv_winc_host_flle.h for usage information.

*/

uint32_t WDRV_WINC_HostFileGetSize(const WDRV_WINC_HOST_FILE_HANDLE fileHandle)
{
    WDRV_WINC_HOST_FILE_DCPT *const pHostFileDcpt
                                = (WDRV_WINC_HOST_FILE_DCPT *const)fileHandle;

    if (NULL == pHostFileDcpt)
    {
        return 0;
    }

    return pHostFileDcpt->size;
}
