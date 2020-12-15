/*******************************************************************************
  File Name:
    nm_common.h

  Summary:
    This module contains common APIs declarations.

  Description:
    This module contains common APIs declarations.
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

#ifndef _NM_COMMON_H_
#define _NM_COMMON_H_

#include "nm_bsp.h"
#include "nm_debug.h"

/**@addtogroup COMMONDEF
 */
/**@{*/

/*states*/
#define M2M_SUCCESS                 ((int8_t)0)
#define M2M_ERR_SEND                ((int8_t)-1)
#define M2M_ERR_RCV                 ((int8_t)-2)
#define M2M_ERR_MEM_ALLOC           ((int8_t)-3)
#define M2M_ERR_TIME_OUT            ((int8_t)-4)
#define M2M_ERR_INIT                ((int8_t)-5)
#define M2M_ERR_BUS_FAIL            ((int8_t)-6)
#define M2M_NOT_YET                 ((int8_t)-7)
#define M2M_ERR_FIRMWARE            ((int8_t)-8)
#define M2M_SPI_FAIL                ((int8_t)-9)
#define M2M_ERR_FIRMWARE_bURN       ((int8_t)-10)
#define M2M_ACK                     ((int8_t)-11)
#define M2M_ERR_FAIL                ((int8_t)-12)
#define M2M_ERR_FW_VER_MISMATCH     ((int8_t)-13)
#define M2M_ERR_SCAN_IN_PROGRESS    ((int8_t)-14)
#define M2M_ERR_INVALID_ARG         ((int8_t)-15)
#define M2M_ERR_INVALID             ((int8_t)-16)

#define NBIT31                      (0x80000000)
#define NBIT30                      (0x40000000)
#define NBIT29                      (0x20000000)
#define NBIT28                      (0x10000000)
#define NBIT27                      (0x08000000)
#define NBIT26                      (0x04000000)
#define NBIT25                      (0x02000000)
#define NBIT24                      (0x01000000)
#define NBIT23                      (0x00800000)
#define NBIT22                      (0x00400000)
#define NBIT21                      (0x00200000)
#define NBIT20                      (0x00100000)
#define NBIT19                      (0x00080000)
#define NBIT18                      (0x00040000)
#define NBIT17                      (0x00020000)
#define NBIT16                      (0x00010000)
#define NBIT15                      (0x00008000)
#define NBIT14                      (0x00004000)
#define NBIT13                      (0x00002000)
#define NBIT12                      (0x00001000)
#define NBIT11                      (0x00000800)
#define NBIT10                      (0x00000400)
#define NBIT9                       (0x00000200)
#define NBIT8                       (0x00000100)
#define NBIT7                       (0x00000080)
#define NBIT6                       (0x00000040)
#define NBIT5                       (0x00000020)
#define NBIT4                       (0x00000010)
#define NBIT3                       (0x00000008)
#define NBIT2                       (0x00000004)
#define NBIT1                       (0x00000002)
#define NBIT0                       (0x00000001)

#ifndef BIG_ENDIAN
#define BYTE_0(word)                    ((uint8_t)(((word) >> 0 ) & 0x000000FFUL))
#define BYTE_1(word)                    ((uint8_t)(((word) >> 8 ) & 0x000000FFUL))
#define BYTE_2(word)                    ((uint8_t)(((word) >> 16) & 0x000000FFUL))
#define BYTE_3(word)                    ((uint8_t)(((word) >> 24) & 0x000000FFUL))
#else
#define BYTE_0(word)                    ((uint8_t)(((word) >> 24) & 0x000000FFUL))
#define BYTE_1(word)                    ((uint8_t)(((word) >> 16) & 0x000000FFUL))
#define BYTE_2(word)                    ((uint8_t)(((word) >> 8 ) & 0x000000FFUL))
#define BYTE_3(word)                    ((uint8_t)(((word) >> 0 ) & 0x000000FFUL))
#endif

/**@}*/     //COMMONDEF
#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @fn         void nm_sleep(uint32_t);
 *  @brief
 *  @param[in]  u32TimeMsec
 *              Time unit in milliseconds
 *  @pre
 *  @warning    Maximum value must nor exceed 4294967295 milliseconds which is equal to 4294967.295 seconds.\n
 *  @note       Implementation of this function is host dependent.
 *  @see
 *  @return     None
 */
void nm_sleep(uint32_t u32TimeMsec);

/*!
 *  @fn     nm_reset
 *  @brief  Reset NMC1500 SoC by setting CHIP_EN and RESET_N signals low,
 *           CHIP_EN high then RESET_N high
 */
void nm_reset(void);

#ifdef __cplusplus
}
#endif
#endif  /*_NM_COMMON_H_*/
