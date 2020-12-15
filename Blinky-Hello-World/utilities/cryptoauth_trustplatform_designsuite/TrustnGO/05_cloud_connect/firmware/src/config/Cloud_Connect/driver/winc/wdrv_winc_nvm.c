/*******************************************************************************
  WINC Driver NVM Access Implementation

  File Name:
    wdrv_winc_httpprovctx.c

  Summary:
    WINC wireless driver NVM access implementation.

  Description:
     This interface manages access to the SPI flash of the WINC device.
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
#include "wdrv_winc_nvm.h"
#include "spi_flash.h"
#ifdef WDRV_WINC_DEVICE_FLEXIBLE_FLASH_MAP
#include "flexible_flash.h"
#endif
#include "spi_flash_map.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************

#ifndef WDRV_WINC_DEVICE_FLEXIBLE_FLASH_MAP
typedef struct
{
    uint32_t address;
    uint32_t size;
} WDRV_WINC_FLASH_MAP_ENTRY;
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

#ifndef M2M_TLS_ROOTCER_FLASH_SZ
#define M2M_TLS_ROOTCER_FLASH_SZ M2M_TLS_ROOTCER_FLASH_SIZE
#endif

#ifndef M2M_TLS_SERVER_FLASH_SZ
#define M2M_TLS_SERVER_FLASH_SZ M2M_TLS_SERVER_FLASH_SIZE
#endif

#ifdef WDRV_WINC_DEVICE_FLEXIBLE_FLASH_MAP
static const uint16_t flashLocationIDMap[] = {
    0x0000, // WDRV_WINC_NVM_REGION_RAW,
    0x0011, // WDRV_WINC_NVM_REGION_FIRMWARE_ACTIVE,
    0x0011, // WDRV_WINC_NVM_REGION_FIRMWARE_INACTIVE,
    0x0021, // WDRV_WINC_NVM_REGION_PLL_TABLE,
    0x0021, // WDRV_WINC_NVM_REGION_GAIN_TABLE,
    0x0021, // WDRV_WINC_NVM_REGION_PLL_AND_GAIN_TABLES
    0x0031, // WDRV_WINC_NVM_REGION_ROOT_CERTS,
    0x0036, // WDRV_WINC_NVM_REGION_LOCAL_CERTS,
    0x0034, // WDRV_WINC_NVM_REGION_CONN_PARAM,
    0x0035, // WDRV_WINC_NVM_REGION_HTTP_FILES,
#ifdef WDRV_WINC_DEVICE_HOST_FILE_DOWNLOAD
    0x0041, // WDRV_WINC_NVM_REGION_HOST_FILE,
#endif
};
#else

#if M2M_GAIN_FLASH_OFFSET != (M2M_PLL_FLASH_OFFSET + M2M_PLL_FLASH_SZ)
#error "PLL and gain tables are not contiguous"
#endif

static const WDRV_WINC_FLASH_MAP_ENTRY flashMap[] =
{
    {0                              , 0                                     },  // WDRV_WINC_NVM_REGION_RAW,
    {0                              , M2M_FIRMWARE_FLASH_SZ                 },  // WDRV_WINC_NVM_REGION_FIRMWARE_ACTIVE,
    {0                              , M2M_FIRMWARE_FLASH_SZ                 },  // WDRV_WINC_NVM_REGION_FIRMWARE_INACTIVE,
    {M2M_PLL_FLASH_OFFSET           , M2M_PLL_FLASH_SZ                      },  // WDRV_WINC_NVM_REGION_PLL_TABLE,
    {M2M_GAIN_FLASH_OFFSET          , M2M_GAIN_FLASH_SZ                     },  // WDRV_WINC_NVM_REGION_GAIN_TABLE,
    {M2M_PLL_FLASH_OFFSET           , M2M_PLL_FLASH_SZ+M2M_GAIN_FLASH_SZ    },  // WDRV_WINC_NVM_REGION_PLL_AND_GAIN_TABLES,
    {M2M_TLS_ROOTCER_FLASH_OFFSET   , M2M_TLS_ROOTCER_FLASH_SZ              },  // WDRV_WINC_NVM_REGION_ROOT_CERTS,
    {M2M_TLS_SERVER_FLASH_OFFSET    , M2M_TLS_SERVER_FLASH_SZ               },  // WDRV_WINC_NVM_REGION_LOCAL_CERTS,
    {M2M_CACHED_CONNS_FLASH_OFFSET  , M2M_CACHED_CONNS_FLASH_SZ             },  // WDRV_WINC_NVM_REGION_CONN_PARAM,
#ifdef WDRV_WINC_DEVICE_WINC3400
    {0                              , M2M_HTTP_MEM_FLASH_SZ                 },  // WDRV_WINC_NVM_REGION_HTTP_FILES,
#else
    {M2M_HTTP_MEM_FLASH_OFFSET      , M2M_HTTP_MEM_FLASH_SZ                 },  // WDRV_WINC_NVM_REGION_HTTP_FILES,
#endif
#ifdef WDRV_WINC_DEVICE_HOST_FILE_DOWNLOAD
    {0x80000                        , 512*1024                              },  // WDRV_WINC_NVM_REGION_HOST_FILE,
#endif
};
#endif

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver NVM Access Implementation
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    static uint8_t _WDRV_WINC_NVMCRC7(uint8_t crc, const uint8_t *buff, uint16_t len)

  Summary:
    Calculate a CRC.

  Description:
    Calculates a CRC over a region of data.

  Precondition:
    None.

  Parameters:
    crc   - Initial CRC value.
    pBuff - Pointer to data to calculate CRC over.
    len   - Length of data.

  Returns:
    - true for success.
    - false for failure.

  Remarks:
    None.

*/

static uint8_t _WDRV_WINC_NVMCRC7(uint8_t crc, const uint8_t *pBuff, uint16_t len)
{
    uint16_t i;
    uint16_t g;
    uint8_t inv;

    for (i=0; i<len; i++)
    {
        for (g=0; g<8; g++)
        {
            inv = (((pBuff[i] << g) & 0x80) >> 7) ^ ((crc >> 6) & 1);
            crc = ((crc << 1) & 0x7f) ^ (9 * inv);
        }
    }

    return crc;
}

//*******************************************************************************
/*
  Function:
    static bool _WDRV_WINC_NVMVerifyCtrlSec(tstrOtaControlSec *pstrControlSec)

  Summary:
    Verify flash control sector.

  Description:
    Verifies that the flash control sector contains a correct magic value and CRC.

  Precondition:
    None.

  Parameters:
    pstrControlSec - Pointer to control sector structure.

  Returns:
    - true for success.
    - false for failure.

  Remarks:
    None.

*/

static bool _WDRV_WINC_NVMVerifyCtrlSec(tstrOtaControlSec *pstrControlSec)
{
    if(pstrControlSec->u32OtaMagicValue != OTA_MAGIC_VALUE)
    {
        return false;
    }

    if(pstrControlSec->u32OtaControlSecCrc != _WDRV_WINC_NVMCRC7(0x7f, (uint8_t*)pstrControlSec, sizeof(tstrOtaControlSec) - 4))
    {
        return false;
    }

    return true;
}

//*******************************************************************************
/*
  Function:
    static bool _WDRV_WINC_NVMReadCtrlSec(tstrOtaControlSec *pstrControlSec)

  Summary:
    Reads the control sector from flash.

  Description:
    Attempts to read the control sector from flash, if that fails it will
    try to read the backup sector.

  Precondition:
    None.

  Parameters:
    pstrControlSec - Pointer to control sector structure.

  Returns:
    - true for success.
    - false for failure.

  Remarks:
    None.

*/

static bool _WDRV_WINC_NVMReadCtrlSec(tstrOtaControlSec *pstrControlSec)
{
    if (M2M_SUCCESS == spi_flash_read((uint8_t*)pstrControlSec, M2M_CONTROL_FLASH_OFFSET, sizeof(tstrOtaControlSec)))
    {
        if (true == _WDRV_WINC_NVMVerifyCtrlSec(pstrControlSec))
        {
            return true;
        }
    }

#ifdef WDRV_WINC_DEVICE_WINC3400
    if (M2M_SUCCESS == spi_flash_read((uint8_t*)pstrControlSec, M2M_BACKUP_FLASH_OFFSET, sizeof(tstrOtaControlSec)))
#else
    if (M2M_SUCCESS == spi_flash_read((uint8_t*)pstrControlSec, M2M_CONTROL_FLASH_BKP_OFFSET, sizeof(tstrOtaControlSec)))
#endif
    {
        if (true == _WDRV_WINC_NVMVerifyCtrlSec(pstrControlSec))
        {
            return true;
        }
    }

    return false;
}

//*******************************************************************************
/*
  Function:
    static bool _WDRV_WINC_NVMFindSection
    (
        WDRV_WINC_NVM_REGION region,
        uint32_t *pStartAddr,
        uint32_t *pSize
    )

  Summary:
    Resolve NVM regions types into address and size of region.

  Description:
    This function looks up the NVM region and determines the regions absolute
    address within the SPI flash and the size of the region.

  Precondition:
    None.

  Parameters:
    region      - Region type.
    pStartAddr  - Pointer to variable to receive region start address.
    pSize       - Pointer to variable to receive region size.

  Returns:
    - true for success.
    - false for failure.

  Remarks:
    None.

*/

static bool _WDRV_WINC_NVMFindSection
(
    WDRV_WINC_NVM_REGION region,
    uint32_t *pStartAddr,
    uint32_t *pSize
)
{
    /* Ensure the pointers and region are valid. */
    if ((NULL == pStartAddr) || (NULL == pSize) || (region >= NUM_WDRV_WINC_NVM_REGIONS))
    {
        return false;
    }

    /* For the raw region resolve the full flash space, otherwise lookup
       the region location and size from the flexible flash map. */
    if (WDRV_WINC_NVM_REGION_RAW == region)
    {
        *pStartAddr = 0;
        *pSize = spi_flash_get_size() << 17;
    }
#ifdef WDRV_WINC_DEVICE_FLEXIBLE_FLASH_MAP
    else if (M2M_SUCCESS != spi_flexible_flash_find_section(flashLocationIDMap[region], pStartAddr, pSize))
    {
        return false;
    }

    switch(region)
    {
        case WDRV_WINC_NVM_REGION_FIRMWARE_ACTIVE:
        case WDRV_WINC_NVM_REGION_FIRMWARE_INACTIVE:
        {
            tstrOtaControlSec strControl;

            /* Read control structure from flash. */
            if (false == _WDRV_WINC_NVMReadCtrlSec(&strControl))
            {
                return false;
            }

            switch(region)
            {
                case WDRV_WINC_NVM_REGION_FIRMWARE_ACTIVE:
                {
                    *pStartAddr = strControl.u32OtaCurrentWorkingImagOffset;
                    break;
                }

                case WDRV_WINC_NVM_REGION_FIRMWARE_INACTIVE:
                {
                    *pStartAddr = strControl.u32OtaRollbackImageOffset;
                    break;
                }

#ifdef WDRV_WINC_DEVICE_WINC3400
                case WDRV_WINC_NVM_REGION_HTTP_FILES:
                {
                    *pStartAddr = strControl.u32OtaCurrentWorkingImagOffset + (M2M_HTTP_MEM_FLASH_OFFSET - M2M_OTA_IMAGE1_OFFSET);
                    break;
                }
#endif

                default:
                {
                    break;
                }
            }

            break;
        }

        case WDRV_WINC_NVM_REGION_PLL_TABLE:
        {
            *pSize = 0x400;
            break;
        }

        case WDRV_WINC_NVM_REGION_GAIN_TABLE:
        {
            *pStartAddr += 0x400;
            *pSize      -= 0x400;
            break;
        }

        default:
        {
            break;
        }
    }
#else
    else
    {
        tstrOtaControlSec strControl;

        if (0 == flashMap[region].address)
        {
            /* Read control structure from flash. */
            if (false == _WDRV_WINC_NVMReadCtrlSec(&strControl))
            {
                return false;
            }
        }

        switch(region)
        {
            case WDRV_WINC_NVM_REGION_FIRMWARE_ACTIVE:
            {
                *pStartAddr = strControl.u32OtaCurrentWorkingImagOffset;
                break;
            }

            case WDRV_WINC_NVM_REGION_FIRMWARE_INACTIVE:
            {
                *pStartAddr = strControl.u32OtaRollbackImageOffset;
                break;
            }

#ifdef WDRV_WINC_DEVICE_WINC3400
            case WDRV_WINC_NVM_REGION_HTTP_FILES:
            {
                *pStartAddr = strControl.u32OtaCurrentWorkingImagOffset + (M2M_HTTP_MEM_FLASH_OFFSET - M2M_OTA_IMAGE1_OFFSET);
                break;
            }
#endif

            default:
            {
                *pStartAddr = flashMap[region].address;
                break;
            }
        }

        *pSize = flashMap[region].size;
    }
#endif

    WDRV_DBG_INFORM_PRINT("NVM Lookup %2d: 0x%06x %0d\r\n", region, *pStartAddr, *pSize);

    return true;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_NVMEraseSector
    (
        DRV_HANDLE handle,
        WDRV_WINC_NVM_REGION region,
        uint8_t startSector,
        uint8_t numSectors
    );

  Summary:
    Erase a sector within an NVM region.

  Description:
    Erases a number of sectors which exist within the requested region.

  Remarks:
    See wdrv_winc_nvm.h for usage information.

*/
WDRV_WINC_STATUS WDRV_WINC_NVMEraseSector
(
    DRV_HANDLE handle,
    WDRV_WINC_NVM_REGION region,
    uint8_t startSector,
    uint8_t numSectors
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;
    uint32_t flashAddress;
    uint32_t flashRegionSize;

    /* Ensure the driver handle and region are valid. */
    if ((NULL == pDcpt) || (region >= NUM_WDRV_WINC_NVM_REGIONS))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure the driver is opened for exclusive flash access. */
    if (0 == (pDcpt->intent & DRV_IO_INTENT_EXCLUSIVE))
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

#ifdef WDRV_WINC_DEVICE_WINC1500
    /* Ensure flash is out of power save mode. */
    if (M2M_SUCCESS != spi_flash_enable(1))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }
#endif

    /* Find region address and size. */
    if (false == _WDRV_WINC_NVMFindSection(region, &flashAddress, &flashRegionSize))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Erase is only supported for regions which begin on a sector boundary. and
       are a whole number of sectors long. Any partial sector regions are blocked. */
    if ((flashAddress | flashRegionSize) & (FLASH_SECTOR_SZ-1))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Check requested size fits within region size. */
    if ((((uint32_t)startSector + numSectors) * FLASH_SECTOR_SZ) > flashRegionSize)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Find start address of area within requested region. */
    flashAddress += (startSector * FLASH_SECTOR_SZ);

    /* Erase the requested sectors. */
    if (M2M_SUCCESS != spi_flash_erase(flashAddress, numSectors * FLASH_SECTOR_SZ))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

#ifdef WDRV_WINC_DEVICE_WINC1500
    /* Return flash to power save mode. */
    if (M2M_SUCCESS != spi_flash_enable(0))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }
#endif

    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_NVMWrite
    (
        DRV_HANDLE handle,
        WDRV_WINC_NVM_REGION region,
        void *pBuffer,
        uint32_t offset,
        uint32_t size
    );

  Summary:
    Writes data to an NVM region.

  Description:
    Writes the number of bytes specified to the NVM region.

  Remarks:
    See wdrv_winc_nvm.h for usage information.

*/
WDRV_WINC_STATUS WDRV_WINC_NVMWrite
(
    DRV_HANDLE handle,
    WDRV_WINC_NVM_REGION region,
    void *pBuffer,
    uint32_t offset,
    uint32_t size
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;
    uint32_t flashAddress;
    uint32_t flashRegionSize;

    /* Ensure the driver handle, buffer pointer and region are valid. */
    if ((NULL == pDcpt) || (NULL == pBuffer) || (region >= NUM_WDRV_WINC_NVM_REGIONS))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure the driver is opened for exclusive flash access. */
    if (0 == (pDcpt->intent & DRV_IO_INTENT_EXCLUSIVE))
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

#ifdef WDRV_WINC_DEVICE_WINC1500
    /* Ensure flash is out of power save mode. */
    if (M2M_SUCCESS != spi_flash_enable(1))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }
#endif

    /* Find region address and size. */
    if (false == _WDRV_WINC_NVMFindSection(region, &flashAddress, &flashRegionSize))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Check requested size fits within region size. */
    if (((offset + size) > flashRegionSize) || ((offset + size) < offset))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Find start address of area within requested region. */
    flashAddress += offset;

    /* Write data to flash. */
    if (M2M_SUCCESS != spi_flash_write(pBuffer, flashAddress, size))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

#ifdef WDRV_WINC_DEVICE_WINC1500
    /* Return flash to power save mode. */
    if (M2M_SUCCESS != spi_flash_enable(0))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }
#endif
    return WDRV_WINC_STATUS_OK;
}

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_NVMRead
    (
        DRV_HANDLE handle,
        WDRV_WINC_NVM_REGION region,
        void *pBuffer,
        uint32_t offset,
        uint32_t size
    );

  Summary:
    Reads data from an NVM region.

  Description:
    Reads the number of bytes specified from the NVM region.

  Remarks:
    See wdrv_winc_nvm.h for usage information.

*/
WDRV_WINC_STATUS WDRV_WINC_NVMRead
(
    DRV_HANDLE handle,
    WDRV_WINC_NVM_REGION region,
    void *pBuffer,
    uint32_t offset,
    uint32_t size
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;
    uint32_t flashAddress;
    uint32_t flashRegionSize;

    /* Ensure the driver handle, buffer pointer and region are valid. */
    if ((NULL == pDcpt) || (NULL == pBuffer) || (region >= NUM_WDRV_WINC_NVM_REGIONS))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Ensure the driver is opened for exclusive flash access. */
    if (0 == (pDcpt->intent & DRV_IO_INTENT_EXCLUSIVE))
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

#ifdef WDRV_WINC_DEVICE_WINC1500
    /* Ensure flash is out of power save mode. */
    if (M2M_SUCCESS != spi_flash_enable(1))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }
#endif

    /* Find region address and size. */
    if (false == _WDRV_WINC_NVMFindSection(region, &flashAddress, &flashRegionSize))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Check requested size fits within region size. */
    if ((offset + size) > flashRegionSize)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    /* Find start address of area within requested region. */
    flashAddress += offset;

    /* Read data from flash. */
    if (M2M_SUCCESS != spi_flash_read(pBuffer, flashAddress, size))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

#ifdef WDRV_WINC_DEVICE_WINC1500
    /* Return flash to power save mode. */
    if (M2M_SUCCESS != spi_flash_enable(0))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }
#endif

    return WDRV_WINC_STATUS_OK;
}
