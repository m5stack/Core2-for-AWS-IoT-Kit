/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_spi.h

  Summary:
    WINC wireless driver SPI APIs.

  Description:
    Provides interface for using the SPI bus.
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

#ifndef _WDRV_WINC_SPI_H
#define _WDRV_WINC_SPI_H

//*******************************************************************************
/*
  Function:
    bool WDRV_WINC_SPISend(unsigned char const *buf, uint32_t size)

  Summary:
    Sends data out to the module through the SPI bus.

  Description:
    This function sends data out to the module through the SPI bus.

  Precondition:
    WDRV_WINC_SPIInitialize must have been called.

  Parameters:
    buf  - buffer pointer of output data
    size - the output data size

  Returns:
    true  - Indicates success
    false - Indicates failure

  Remarks:
    None.
 */
bool WDRV_WINC_SPISend(unsigned char *const buf, uint32_t size);

//*******************************************************************************
/*
  Function:
    bool WDRV_WINC_SPIReceive(unsigned char *const buf, uint32_t size)

  Summary:
    Receives data from the module through the SPI bus.

  Description:
    This function receives data from the module through the SPI bus.

  Precondition:
    WDRV_WINC_SPIInitialize must have been called.

  Parameters:
    buf  - buffer pointer of input data
    size - the input data size

  Returns:
    true  - Indicates success
    false - Indicates failure

  Remarks:
    None.
 */
bool WDRV_WINC_SPIReceive(unsigned char *const buf, uint32_t size);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_SPIInitialize(void)

  Summary:
    Initializes the SPI object for the WiFi driver.

  Description:
    This function initializes the SPI object for the WiFi driver.

  Precondition:
    None.

  Returns:
    None.

  Remarks:
    None.
 */
void WDRV_WINC_SPIInitialize(void);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_SPIDeinitialize(void)

  Summary:
    Deinitializes the SPI object for the WiFi driver.

  Description:
    This function deinitializes the SPI object for the WiFi driver.

  Precondition:
    None.

  Returns:
    None.

  Remarks:
    None.
 */
void WDRV_WINC_SPIDeinitialize(void);

#endif /* _WDRV_WINC_SPI_H */
