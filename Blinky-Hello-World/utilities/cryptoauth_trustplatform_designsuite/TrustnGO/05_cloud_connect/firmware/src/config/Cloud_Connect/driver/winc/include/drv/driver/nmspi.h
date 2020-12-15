/*******************************************************************************
  This module contains WINC1500 SPI protocol bus APIs implementation.

  File Name:
    nmspi.h

  Summary:
    This module contains WINC1500 SPI protocol bus APIs implementation.

  Description:
    This module contains WINC1500 SPI protocol bus APIs implementation.
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

#ifndef _NMSPI_H_
#define _NMSPI_H_

#include "nm_common.h"

#ifdef __cplusplus
     extern "C" {
#endif

/**
*   @fn     nm_spi_init
*   @brief  Initialize the SPI
*   @return ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
int8_t nm_spi_init(void);

/**
*   @fn     nm_spi_lock_init
*   @brief  Initialize the SPI lock
*   @return None
*/
void nm_spi_lock_init(void);

/**
*   @fn     nm_spi_reset
*   @brief  reset the SPI
*   @return ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
int8_t nm_spi_reset(void);

/**
*   @fn     nm_spi_deinit
*   @brief  DeInitialize the SPI
*   @return ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
int8_t nm_spi_deinit(void);

/**
*   @fn     nm_spi_read_reg
*   @brief  Read register
*   @param [in] u32Addr
*               Register address
*   @return Register value
*/
uint32_t nm_spi_read_reg(uint32_t u32Addr);

/**
*   @fn     nm_spi_read_reg_with_ret
*   @brief  Read register with error code return
*   @param [in] u32Addr
*               Register address
*   @param [out]    pu32RetVal
*               Pointer to u32 variable used to return the read value
*   @return ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
int8_t nm_spi_read_reg_with_ret(uint32_t u32Addr, uint32_t* pu32RetVal);

/**
*   @fn     nm_spi_write_reg
*   @brief  write register
*   @param [in] u32Addr
*               Register address
*   @param [in] u32Val
*               Value to be written to the register
*   @return ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
int8_t nm_spi_write_reg(uint32_t u32Addr, uint32_t u32Val);

/**
*   @fn     nm_spi_read_block
*   @brief  Read block of data
*   @param [in] u32Addr
*               Start address
*   @param [out]    puBuf
*               Pointer to a buffer used to return the read data
*   @param [in] u16Sz
*               Number of bytes to read. The buffer size must be >= u16Sz
*   @return ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
int8_t nm_spi_read_block(uint32_t u32Addr, uint8_t *puBuf, uint16_t u16Sz);

/**
*   @fn     nm_spi_write_block
*   @brief  Write block of data
*   @param [in] u32Addr
*               Start address
*   @param [in] puBuf
*               Pointer to the buffer holding the data to be written
*   @param [in] u16Sz
*               Number of bytes to write. The buffer size must be >= u16Sz
*   @return ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
int8_t nm_spi_write_block(uint32_t u32Addr, uint8_t *puBuf, uint16_t u16Sz);

#ifdef __cplusplus
     }
#endif

#endif /* _NMSPI_H_ */
