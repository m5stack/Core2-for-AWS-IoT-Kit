/*******************************************************************************
  File Name:
    flexible_flash.c

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

#include "spi_flash.h"
#include "m2m_types.h"
#include "flexible_flash.h"

#define FLASH_MAP_TABLE_ADDR        (FLASH_SECTOR_SZ+sizeof(tstrOtaControlSec)+8)
#define N_ENTRIES_MAX               32

int8_t spi_flexible_flash_find_section(uint16_t u16EntryIDToLookFor, uint32_t *pu32StartOffset, uint32_t *pu32Size)
{
    int8_t s8Ret = M2M_ERR_INVALID_ARG;
    if((NULL == pu32StartOffset) || (NULL == pu32Size)) goto EXIT;

    uint8_t au8buff[8];
    uint8_t u8CurrEntry = 0;
    s8Ret = spi_flash_read(&au8buff[0], FLASH_MAP_TABLE_ADDR, 4);
    if(M2M_SUCCESS != s8Ret) goto EXIT;

    uint8_t u8nEntries = au8buff[0];     // Max number is 32, reading one byte will suffice
    if(u8nEntries > N_ENTRIES_MAX)
    {
        s8Ret = M2M_ERR_FAIL;
        goto EXIT;
    }

    while(u8nEntries > u8CurrEntry)
    {
        s8Ret = spi_flash_read(&au8buff[0], FLASH_MAP_TABLE_ADDR + 4 + (u8CurrEntry*8), 8);
        u8CurrEntry++;
        if(M2M_SUCCESS != s8Ret) break;
        uint16_t u16EntryID = (au8buff[1] << 8) | au8buff[0];
        if(u16EntryID != u16EntryIDToLookFor) continue;
        *pu32StartOffset = au8buff[2] * FLASH_SECTOR_SZ;
        *pu32Size        = au8buff[3] * FLASH_SECTOR_SZ;
        break;
    }
EXIT:
    return s8Ret;
}