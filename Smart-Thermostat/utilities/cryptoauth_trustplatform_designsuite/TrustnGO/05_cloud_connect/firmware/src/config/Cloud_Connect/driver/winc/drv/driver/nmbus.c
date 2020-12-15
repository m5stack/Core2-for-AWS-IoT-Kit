/*******************************************************************************
  This module contains WINC1500 bus APIs implementation.

  File Name:
    nmbus.c

  Summary:
    This module contains WINC1500 bus APIs implementation.

  Description:
    This module contains WINC1500 bus APIs implementation.
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

#include "nmbus.h"
#include "nmspi.h"

#define MAX_TRX_CFG_SZ      8
#define NM_BUS_MAX_TRX_SZ   256

/**
*   @struct tstrNmBusCapabilities
*   @brief  Structure holding bus capabilities information
*   @sa NM_BUS_TYPE_I2C, NM_BUS_TYPE_SPI
*/
typedef struct
{
    uint16_t    u16MaxTrxSz;    /*!< Maximum transfer size. Must be >= 16 bytes*/
} tstrNmBusCapabilities;

tstrNmBusCapabilities egstrNmBusCapabilities =
{
    NM_BUS_MAX_TRX_SZ
};

/*
*   @fn     nm_bus_init
*   @brief  Initialize the bus wrapper
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
static int8_t nm_bus_init(void *pvinit)
{
    nm_reset();
    nm_sleep(1);
    return M2M_SUCCESS;
}

/*
*   @fn     nm_bus_deinit
*   @brief  De-initialize the bus wrapper
*/
static int8_t nm_bus_deinit(void)
{
    return M2M_SUCCESS;
}

/**
*   @fn     nm_bus_iface_init
*   @brief  Initialize bus interface
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_bus_iface_init(void *pvInitVal)
{
    int8_t ret = M2M_SUCCESS;
    ret = nm_bus_init(pvInitVal);
    return ret;
}

/**
*   @fn     nm_bus_iface_deinit
*   @brief  Deinitialize bus interface
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author Samer Sarhan
*   @date   07 April 2014
*   @version    1.0
*/
int8_t nm_bus_iface_deinit(void)
{
    int8_t ret = M2M_SUCCESS;
    ret = nm_bus_deinit();

    return ret;
}

/**
*   @fn     nm_bus_reset
*   @brief  reset bus interface
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @version    1.0
*/
int8_t nm_bus_reset(void)
{
    return nm_spi_reset();
}

/**
*   @fn     nm_bus_iface_reconfigure
*   @brief  reconfigure bus interface
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author Viswanathan Murugesan
*   @date   22 Oct 2014
*   @version    1.0
*/
int8_t nm_bus_iface_reconfigure(void *ptr)
{
    return M2M_SUCCESS;
}

/*
*   @fn     nm_read_reg
*   @brief  Read register
*   @param [in] u32Addr
*               Register address
*   @return Register value
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
uint32_t nm_read_reg(uint32_t u32Addr)
{
    return nm_spi_read_reg(u32Addr);
}

/*
*   @fn     nm_read_reg_with_ret
*   @brief  Read register with error code return
*   @param [in] u32Addr
*               Register address
*   @param [out]    pu32RetVal
*               Pointer to u32 variable used to return the read value
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_read_reg_with_ret(uint32_t u32Addr, uint32_t* pu32RetVal)
{
    return nm_spi_read_reg_with_ret(u32Addr,pu32RetVal);
}

/*
*   @fn     nm_write_reg
*   @brief  write register
*   @param [in] u32Addr
*               Register address
*   @param [in] u32Val
*               Value to be written to the register
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_write_reg(uint32_t u32Addr, uint32_t u32Val)
{
    return nm_spi_write_reg(u32Addr,u32Val);
}

static int8_t p_nm_read_block(uint32_t u32Addr, uint8_t *puBuf, uint16_t u16Sz)
{
    return nm_spi_read_block(u32Addr,puBuf,u16Sz);
}
/*
*   @fn     nm_read_block
*   @brief  Read block of data
*   @param [in] u32Addr
*               Start address
*   @param [out]    puBuf
*               Pointer to a buffer used to return the read data
*   @param [in] u32Sz
*               Number of bytes to read. The buffer size must be >= u32Sz
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_read_block(uint32_t u32Addr, uint8_t *puBuf, uint32_t u32Sz)
{
    uint16_t u16MaxTrxSz = egstrNmBusCapabilities.u16MaxTrxSz - MAX_TRX_CFG_SZ;
    uint32_t off = 0;
    int8_t s8Ret = M2M_SUCCESS;

    for(;;)
    {
        if(u32Sz <= u16MaxTrxSz)
        {
            s8Ret += p_nm_read_block(u32Addr, &puBuf[off], (uint16_t)u32Sz);
            break;
        }
        else
        {
            s8Ret += p_nm_read_block(u32Addr, &puBuf[off], u16MaxTrxSz);
            if(M2M_SUCCESS != s8Ret) break;
            u32Sz -= u16MaxTrxSz;
            off += u16MaxTrxSz;
            u32Addr += u16MaxTrxSz;
        }
    }

    return s8Ret;
}

static int8_t p_nm_write_block(uint32_t u32Addr, uint8_t *puBuf, uint16_t u16Sz)
{
    return nm_spi_write_block(u32Addr,puBuf,u16Sz);
}
/**
*   @fn     nm_write_block
*   @brief  Write block of data
*   @param [in] u32Addr
*               Start address
*   @param [in] puBuf
*               Pointer to the buffer holding the data to be written
*   @param [in] u32Sz
*               Number of bytes to write. The buffer size must be >= u32Sz
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_write_block(uint32_t u32Addr, uint8_t *puBuf, uint32_t u32Sz)
{
    uint16_t u16MaxTrxSz = egstrNmBusCapabilities.u16MaxTrxSz - MAX_TRX_CFG_SZ;
    uint32_t off = 0;
    int8_t s8Ret = M2M_SUCCESS;

    for(;;)
    {
        if(u32Sz <= u16MaxTrxSz)
        {
            s8Ret += p_nm_write_block(u32Addr, &puBuf[off], (uint16_t)u32Sz);
            break;
        }
        else
        {
            s8Ret += p_nm_write_block(u32Addr, &puBuf[off], u16MaxTrxSz);
            if(M2M_SUCCESS != s8Ret) break;
            u32Sz -= u16MaxTrxSz;
            off += u16MaxTrxSz;
            u32Addr += u16MaxTrxSz;
        }
    }

    return s8Ret;
}

//DOM-IGNORE-END
