/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_gpio.h

  Summary:
    WINC wireless driver GPIO APIs.

  Description:
    Provides an interface to control GPIOs used to control the WINC.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
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

#ifndef _WDRV_WINC_GPIO_H
#define _WDRV_WINC_GPIO_H

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_INTInitialize(void)

  Summary:
    Initializes interrupts for the WiFi driver.

  Description:
    This function initializes interrupts for the WiFi driver.

  Precondition:
    WiFi initialization must be complete.

  Parameters:
    None.

  Returns:
    None.

  Remarks:
    None.
 */
void WDRV_WINC_INTInitialize(void);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_INTDeinitialize(void)

  Summary:
    Deinitializes interrupts for WiFi driver.

  Description:
    This function deinitializes interrupts for the WiFi driver.

  Precondition:
    WiFi initialization must be complete.

  Returns:
    None.

  Remarks:
    None.
 */
void WDRV_WINC_INTDeinitialize(void);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_GPIOChipEnableAssert(void)

  Summary:
    Asserts the chip enable list.

  Description:
    Enables the WINC by asserting the chip enable line.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Remarks:
    None.
 */
void WDRV_WINC_GPIOChipEnableAssert(void);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_GPIOChipEnableDeassert(void)

  Summary:
    Deassert the chip enable line.

  Description:
    Disables the WINC by deasserting the chip enable line.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Remarks:
    None.
 */
void WDRV_WINC_GPIOChipEnableDeassert(void);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_GPIOResetAssert(void)

  Summary:
    Assert the reset line.

  Description:
    Reset the WINC by asserting the reset line.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Remarks:
    None.
 */
void WDRV_WINC_GPIOResetAssert(void);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_GPIOResetDeassert(void)

  Summary:
    Deassert the reset line.

  Description:
    Deassert the reset line to take the WINC out of reset.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Remarks:
    None.
 */
void WDRV_WINC_GPIOResetDeassert(void);

#endif /* _WDRV_WINC_GPIO_H */
