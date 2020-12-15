/*******************************************************************************
  This module contains WINC1500 flexible flash map implementation.

  File Name:
    flexible_flash.h

  Summary:
    This module contains WINC1500 flexible flash map implementation.

  Description:
    This module contains WINC1500 flexible flash map implementation.
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
// DOM-IGNORE-END

#ifndef __FLEXIBLE_FLASH_H__
#define __FLEXIBLE_FLASH_H__

#include <stdint.h>

typedef struct {
    uint32_t magic;
    uint32_t max_size;
}tstrFlashLUTHeader;

// NOTE: Don't use enums for id/status here,
// they need to be 16 bit but the enums end up as
// 32 bit even if the __packed__ attribute is used
typedef struct {
    uint16_t id;
    uint8_t sector;
    uint8_t size;
    uint32_t reserved;
}tstrFlashLUTEntry;

#define FLASHMAP_MAGIC_VALUE			0x1ABCDEF9
#define FLASHMAP_MAX_ENTRIES			32

// + 8 is for the number of entries value (uint32_t) and CRC (uint32_t)
// * 2 is for the new lookup table to apply
// + 48 is for the progress monitor
#define FLASHMAP_MAX_SIZE				(sizeof(tstrFlashLUTHeader) + (((sizeof(tstrFlashLUTEntry) * FLASHMAP_MAX_ENTRIES) + 8) * 2) + 48)


/** @defgroup SPiFlashRead spi_flexible_flash_find_section
 *  @ingroup SPIFLASHAPI
 */
  /**@{*/
/*!
 * @fn          int8_t spi_flexible_flash_find_section(uint16_t u16EntryIDToLookFor, uint32_t *pu32StartOffset, uint32_t *pu32Size);
 * @brief       Read the Flash Map to extract the host file starting offset.\n
 * @param [in]  u16EntryIDToLookFor
 *                  The ID of the location in Flash we are looking for. See @ref tenuFlashLUTEntryID.
 * @param [in]  pu32StartOffset
 *                  Pointer to the variable where the Flash section start address should be stored.
 * @param [in]  pu32Size
 *                  Pointer to the variable where the Flash section size should be stored.
 * @warning
 *              In case there is a running WINC firmware, it is required to pause the firmware
 *              first before any trial to access SPI flash to avoid any racing between host and
 *              running firmware on bus. @ref m2m_wifi_download_mode can be used to pause the firmware.
 * @sa          m2m_wifi_download_mode
 *              m2m_wifi_init_hold
 * @return      The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.

 */
int8_t spi_flexible_flash_find_section(uint16_t u16EntryIDToLookFor, uint32_t *pu32StartOffset, uint32_t *pu32Size);
 /**@}*/

#endif /* __FLEXIBLE_FLASH_H__ */
