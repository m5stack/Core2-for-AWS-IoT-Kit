/*******************************************************************************
  WINC Wireless Driver GPIO Interface

  File Name:
    wdrv_winc_gpio.c

  Summary:
    GPIO interface for WINC wireless driver.

  Description:
    GPIO interface for WINC wireless driver.
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

#include <stdlib.h>
#include <stdint.h>
#include "configuration.h"
#include "definitions.h"

/****************************************************************************
 * Function:        WDRV_WINC_GPIOResetAssert
 * Summary: Reset the WINC by asserting the reset line.
 *****************************************************************************/
void WDRV_WINC_GPIOResetAssert(void)
{
    WDRV_WINC_RESETN_Clear();
}

/****************************************************************************
 * Function:        WDRV_WINC_GPIOResetDeassert
 * Summary: Deassert the reset line.
 *****************************************************************************/
void WDRV_WINC_GPIOResetDeassert(void)
{
    WDRV_WINC_RESETN_Set();
}

/****************************************************************************
 * Function:        WDRV_WINC_GPIOChipEnableAssert
 * Summary: Asserts the chip enable list.
 *****************************************************************************/
void WDRV_WINC_GPIOChipEnableAssert(void)
{
    WDRV_WINC_CHIP_EN_Set();
}

/****************************************************************************
 * Function:        WDRV_WINC_GPIOChipEnableDeassert
 * Summary: Deassert the chip enable line.
 *****************************************************************************/
void WDRV_WINC_GPIOChipEnableDeassert(void)
{
    WDRV_WINC_CHIP_EN_Clear();
}

//DOM-IGNORE-END
