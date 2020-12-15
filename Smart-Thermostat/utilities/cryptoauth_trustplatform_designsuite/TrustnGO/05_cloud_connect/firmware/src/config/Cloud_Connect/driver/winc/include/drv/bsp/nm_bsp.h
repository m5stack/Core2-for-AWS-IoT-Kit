/*******************************************************************************
  File Name:
    nm_common.h

  Summary:
    This module contains WINC1500 BSP APIs declarations.

  Description:
    This module contains WINC1500 BSP APIs declarations.
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

/** @defgroup nm_bsp BSP
    @brief
        Description of the BSP (<strong>B</strong>oard <strong>S</strong>upport <strong>P</strong>ackage) module.
    @{
        @defgroup   DataT       Data Types
        @defgroup   BSPDefine   Defines
        @defgroup   BSPAPI      Functions
        @brief
            Lists the available BSP (<strong>B</strong>oard <strong>S</strong>upport <strong>P</strong>ackage) APIs.
    @}
 */

/**@addtogroup BSPDefine
   @{
 */
#ifndef _NM_BSP_H_
#define _NM_BSP_H_


#define BSP_MIN(x,y) ((x)>(y)?(y):(x))
/*!<
*     Computes the minimum value between \b x and \b y.
*/
/**@}*/     //BSPDefine

 //@}

#ifdef _NM_BSP_BIG_END
#define NM_BSP_B_L_32(x)      \
((((x) & 0x000000FF) << 24) + \
(((x) & 0x0000FF00) << 8)   + \
(((x) & 0x00FF0000) >> 8)   + \
(((x) & 0xFF000000) >> 24))

#define NM_BSP_B_L_16(x) \
((((x) & 0x00FF) << 8) + \
(((x)  & 0xFF00) >> 8))
#else
#define NM_BSP_B_L_32(x)  (x)
#define NM_BSP_B_L_16(x)  (x)
#endif

#endif  /*_NM_BSP_H_*/
