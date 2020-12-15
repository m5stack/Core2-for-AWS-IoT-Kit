/*******************************************************************************
  File Name:
    nm_common.c

  Summary:
    This module contains common APIs implementations.

  Description:
    This module contains common APIs implementations.
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

#include "nm_common.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc_gpio.h"

/*!
 *  @fn         nm_sleep
 *  @brief      Sleep in units of mSec
 *  @param[IN]  u32TimeMsec
 *              Time in milliseconds
 */
void nm_sleep(uint32_t u32TimeMsec)
{
    WDRV_MSDelay(u32TimeMsec);
}

/*!
 *  @fn     nm_reset
 *  @brief  Reset NMC1500 SoC by setting CHIP_EN and RESET_N signals low,
 *           CHIP_EN high then RESET_N high
 */
void nm_reset(void)
{
    WDRV_WINC_GPIOChipEnableDeassert();
    WDRV_WINC_GPIOResetAssert();
    nm_sleep(100);
    WDRV_WINC_GPIOChipEnableAssert();
    nm_sleep(10);
    WDRV_WINC_GPIOResetDeassert();
    nm_sleep(10);
}

//DOM-IGNORE-END
