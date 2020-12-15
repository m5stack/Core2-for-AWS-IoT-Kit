/*******************************************************************************
  File Name:
    nm_debug.h

  Summary:
    This module contains debug API declarations.

  Description:
    This module contains debug API declarations.
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
#ifndef _NM_DEBUG_H_
#define _NM_DEBUG_H_

#include "wdrv_winc_debug.h"

/**
 * @defgroup DebugDefines DebugDefines
 * @ingroup WlanDefines
 */

/**@{*/

#define M2M_ERR(...) WDRV_DBG_ERROR_PRINT(__VA_ARGS__)
#define M2M_INFO(...) WDRV_DBG_INFORM_PRINT(__VA_ARGS__)
#define M2M_DBG(...) WDRV_DBG_VERBOSE_PRINT(__VA_ARGS__)
#define M2M_PRINT(...) WDRV_DBG_VERBOSE_PRINT(__VA_ARGS__)

/**@}*/

#endif /* _NM_DEBUG_H_ */
