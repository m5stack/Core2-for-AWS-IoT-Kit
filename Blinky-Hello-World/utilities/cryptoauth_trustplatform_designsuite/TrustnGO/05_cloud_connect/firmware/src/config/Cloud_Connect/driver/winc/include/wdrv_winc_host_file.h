/*******************************************************************************
  WINC Driver Host File Download Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_host_file.h

  Summary:
    WINC wireless driver host file download header file.

  Description:
    This interface manages the downloading of file via the WINCs internal
      flash memory.
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

#ifndef _WDRV_WINC_HOST_FILE_H
#define _WDRV_WINC_HOST_FILE_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_winc_common.h"
#include "m2m_ota.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  Host File Descriptor

  Summary:
    Defines the format of the host file descriptor.

  Description:
    Structure to manage state for the host file download.

  Remarks:
    None.
*/

typedef struct
{
    /* File handle received from WINC. */
    uint8_t         handle;

    /* File get operation callback. */
    tpfFileGetCb    getFileCB;

    /* Read file operation callback. */
    tpfFileReadCb   readFileCB;

    /* Erase file operation callback. */
    tpfFileEraseCb  eraseFileCB;

    /* Current offset within file being read. */
    uint32_t        offset;

    /* Bytes remaining to read from file. */
    uint32_t        remain;

    /* Total size of current file. */
    uint32_t        size;

    /* Pointer to current write buffer for read operation. */
    uint8_t         *pBuffer;
} WDRV_WINC_HOST_FILE_DCPT;

// *****************************************************************************
/*  Host File Operations

  Summary:
    Possible host file operations.

  Description:
    Defines the types of operations the host file download module can perform.

  Remarks:
    Used when calling user supplied callback to indicate type of operation.
*/

typedef enum
{
    /* Invalid or unset operation, should never be seen. */

    WDRV_WINC_HOST_FILE_OP_INVALID,

    /* File download (get) operation. */
    WDRV_WINC_HOST_FILE_OP_DOWNLOAD,

    /* File read operation. */
    WDRV_WINC_HOST_FILE_OP_READ,

    /* File erase operation. */
    WDRV_WINC_HOST_FILE_OP_ERASE
} WDRV_WINC_HOST_FILE_OPERATION;

// *****************************************************************************
/*  Host File Status

  Summary:
    Possible host file statuses.

  Description:
    Defines the types of statuses reported.

  Remarks:
    None.
*/

typedef enum
{
    WDRV_WINC_HOST_FILE_STATUS_SUCCESS = OTA_STATUS_SUCCESS,
} WDRV_WINC_HOST_FILE_STATUS;

// *****************************************************************************
/*  Host File Handle

  Summary:
    File handle type definition.

  Description:
    Defines the type used for host file handles.

  Remarks:
    None.
*/

typedef uintptr_t WDRV_WINC_HOST_FILE_HANDLE;

// *****************************************************************************
/*  Host File Handle Values

  Summary:
    Possible host file handle values.

  Description:
    Defines key values for host file download handles.

  Remarks:
    None.
*/

#define WDRV_WINC_HOST_FILE_INVALID_HANDLE  0

// *****************************************************************************
/*  Host File Download Operation Status Callback

  Summary:
    Pointer to a callback function to receive status updates about host file
      download operations.

  Description:
    This data type defines a function callback for host file download operations.
      This callback provides the file handle as well as the type of operation
      and status.

  Parameters:
    handle      - Client handle obtained by a call to WDRV_WINC_Open.
    fileHandle  - Host file handle.
    operation   - Type of operation, see WDRV_WINC_HOST_FILE_OPERATION.
    status      - Status of operation, see WDRV_WINC_HOST_FILE_STATUS.

  Returns:
    None.

  Remarks:
    None.

*/

typedef void (*WDRV_WINC_HOST_FILE_STATUS_CALLBACK)
(
    DRV_HANDLE handle,
    WDRV_WINC_HOST_FILE_HANDLE fileHandle,
    WDRV_WINC_HOST_FILE_OPERATION operation,
    uint8_t status
);

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Host File Download Routines
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

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.
    An active WiFI connection.

  Parameters:
    handle          - Client handle obtained by a call to WDRV_WINC_Open.
    pFileURL        - Pointer to a URL specifying the file to download.
    pfHostFileGetCB - Pointer to the callback function to receive the status.

  Returns:
    WDRV_WINC_STATUS_OK             - The information has been returned.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_HostFileDownload
(
    DRV_HANDLE handle,
    const char *pFileURL,
    const WDRV_WINC_HOST_FILE_STATUS_CALLBACK pfHostFileGetCB
);

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

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.
    WDRV_WINC_HostFileDownload should have been called to obtain a valid file handle.

  Parameters:
    handle                  - Client handle obtained by a call to WDRV_WINC_Open.
    fileHandle              - File handle obtained by call to WDRV_WINC_HostFileDownload
    pBuffer                 - Pointer to buffer to receive data from file.
    offset                  - Offset into the file to begin the read operation from.
    size                    - Size of read operation.
    pfHostFileReadStatusCB  - Pointer to callback function to receive status.

  Returns:
    WDRV_WINC_STATUS_OK             - The information has been returned.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_HostFileRead
(
    DRV_HANDLE handle,
    WDRV_WINC_HOST_FILE_HANDLE fileHandle,
    void *pBuffer,
    uint32_t offset,
    uint32_t size,
    const WDRV_WINC_HOST_FILE_STATUS_CALLBACK pfHostFileReadStatusCB
);

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

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.
    WDRV_WINC_HostFileDownload should have been called to obtain a valid file handle.

  Parameters:
    handle                  - Client handle obtained by a call to WDRV_WINC_Open.
    fileHandle              - File handle obtained by call to WDRV_WINC_HostFileDownload
    pfHostFileEraseStatusCB - Pointer to callback function to receive status.

  Returns:
    WDRV_WINC_STATUS_OK             - The information has been returned.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_HostFileErase
(
    DRV_HANDLE handle,
    WDRV_WINC_HOST_FILE_HANDLE fileHandle,
    const WDRV_WINC_HOST_FILE_STATUS_CALLBACK pfHostFileEraseStatusCB
);

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

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle  - Client handle obtained by a call to WDRV_WINC_Open.
    id      - File handle ID to translate.

  Returns:
    Driver file handle object, or NULL in case of error.

  Remarks:
    None.

*/

WDRV_WINC_HOST_FILE_HANDLE WDRV_WINC_HostFileFindByID
(
    DRV_HANDLE handle,
    uint8_t id
);

//*******************************************************************************
/*
  Function:
    uint32_t WDRV_WINC_HostFileGetSize(const WDRV_WINC_HOST_FILE_HANDLE fileHandle);

  Summary:
    Retrieves the file size from a file handle.

  Description:
    Given a driver file handle this function will retrieve the file size in bytes.

  Precondition:
    None.

  Parameters:
    fileHandle - File handle obtained by call to WDRV_WINC_HostFileDownload

  Returns:
    Size of the file in bytes, or 0 in case of error.

  Remarks:
    None.

*/

uint32_t WDRV_WINC_HostFileGetSize(const WDRV_WINC_HOST_FILE_HANDLE fileHandle);

#endif /* _WDRV_WINC_HOST_FILE_H */
