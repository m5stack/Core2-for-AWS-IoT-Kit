/*******************************************************************************
  SERCOM_SPI(SERIAL COMMUNICATION SERIAL PERIPHERAL INTERFACE) PLIB

  Company
    Microchip Technology Inc.

  File Name
    plib_sercom_spi_master_common.h

  Summary
   SERCOM_SPI PLIB Master Local Header File.

  Description
    This file defines the interface to the SERCOM SPI peripheral library.
    This library provides access to and control of the associated
    peripheral instance.

  Remarks:
    None.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
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

#ifndef PLIB_SERCOM_SPI_MASTER_COMMON_H  // Guards against multiple inclusion
#define PLIB_SERCOM_SPI_MASTER_COMMON_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <device.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* SPI Clock Phase

  Summary:
    Identifies SPI Clock Phase Options

  Description:
    This enumeration identifies possible SPI Clock Phase Options.

  Remarks:
    None.
*/

typedef enum
{
    /* Input data is sampled on clock trailing edge and changed on
       leading edge */
    SPI_CLOCK_PHASE_TRAILING_EDGE = SERCOM_SPIM_CTRLA_CPHA_TRAILING_EDGE,

    /* Input data is sampled on clock leading edge and changed on
       trailing edge */
    SPI_CLOCK_PHASE_LEADING_EDGE = SERCOM_SPIM_CTRLA_CPHA_LEADING_EDGE,

    /* Force the compiler to reserve 32-bit space for each enum value */
    SPI_CLOCK_PHASE_INVALID = 0xFFFFFFFF

} SPI_CLOCK_PHASE;

// *****************************************************************************
/* SPI Clock Polarity

  Summary:
    Identifies SPI Clock Polarity Options

  Description:
    This enumeration identifies possible SPI Clock Polarity Options.

  Remarks:
    None.
*/

typedef enum
{
    /* The inactive state value of clock is logic level zero */
    SPI_CLOCK_POLARITY_IDLE_LOW = SERCOM_SPIM_CTRLA_CPOL_IDLE_LOW,

    /* The inactive state value of clock is logic level one */
    SPI_CLOCK_POLARITY_IDLE_HIGH = SERCOM_SPIM_CTRLA_CPOL_IDLE_HIGH,

    /* Force the compiler to reserve 32-bit space for each enum value */
    SPI_CLOCK_POLARITY_INVALID = 0xFFFFFFFF

} SPI_CLOCK_POLARITY;

// *****************************************************************************
/* SPI Data Bits

  Summary:
    Identifies SPI bits per transfer

  Description:
    This enumeration identifies number of bits per SPI transfer.

  Remarks:
    For 9 bit mode, data should be right aligned in the 16 bit
    memory location.
*/

typedef enum
{
    /* 8 bits per transfer */
    SPI_DATA_BITS_8 = SERCOM_SPIM_CTRLB_CHSIZE_8_BIT,

    /* 9 bits per transfer */
    SPI_DATA_BITS_9 = SERCOM_SPIM_CTRLB_CHSIZE_9_BIT,

    /* Force the compiler to reserve 32-bit space for each enum value */
    SPI_DATA_BITS_INVALID = 0xFFFFFFFF

} SPI_DATA_BITS;

// *****************************************************************************
/* SPI Transfer Setup Parameters

  Summary:
    Identifies the setup parameters which can be changed dynamically.

  Description
    This structure identifies the possible setup parameters for SPI
    which can be changed dynamically if needed.

  Remarks:
    None.
*/

typedef struct
{
    /* Baud Rate or clock frequency */
    uint32_t            clockFrequency;

    /* Clock Phase */
    SPI_CLOCK_PHASE     clockPhase;

    /* Clock Polarity */
    SPI_CLOCK_POLARITY  clockPolarity;

    /* Number of bits per transfer */
    SPI_DATA_BITS       dataBits;

} SPI_TRANSFER_SETUP;

// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* SPI CallBack Function Pointer

  Summary:
    Pointer to a SPI Call back function.

  Description:
    This data type defines the required function signature for the
    SPI event handling callback function. Application must register
    a pointer to an event handling function whose function signature (parameter
    and return value types) match the types specified by this function pointer
    in order to receive event calls back from the PLIB.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    context         - Value identifying the context of the application that
                      registered the event handling function

  Returns:
    None.

  Example:
    <code>
    <code>
        SPI1_CallbackRegister(&APP_SPICallBack, NULL);
        void APP_SPICallBack(uintptr_t contextHandle)
        {
            if( SPI_ERROR_NONE == SPI1_ErrorGet())
            {
                Exchange was completed without error, do something else now.
            }
        }
    </code>

  Remarks:
    The context parameter contains the a handle to the client context,
    provided at the time the event handling function was registered using the
    SPIx_CallbackRegister function. This context handle value is
    passed back to the client as the "context" parameter. It can be any value
    (such as a pointer to the client's data) necessary to identify the client
    context or instance of the client that made the data exchange
    request.

    The event handler function executes in the PLIB's interrupt context. It is
    recommended of the application to not perform process intensive or blocking
    operations with in this function.
*/

typedef void (*SERCOM_SPI_CALLBACK)(uintptr_t context);

// *****************************************************************************
// *****************************************************************************
// Section: Local: **** Local SPI Object****
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* SPI Object

  Summary:
    Defines the data type for the data structures used for peripheral
    operations.

  Description:
    This may be for used for peripheral operations.

  Remarks:
    None.
*/

typedef struct
{
    /* Pointer to the transmitter buffer */
    void *                   txBuffer;

    /* Pointer to the received buffer */
    void *                   rxBuffer;

    size_t                   txSize;

    size_t                   rxSize;

    size_t                   dummySize;

    /* Size of the receive processed exchange size */
    size_t                   rxCount;

    /* Size of the transmit processed exchange size */
    size_t                   txCount;

    /* Exchange busy status of the SPI */
    bool                     transferIsBusy;

    /* SPI Event handler */
    SERCOM_SPI_CALLBACK      callback;

    /* Context */
    uintptr_t                context;

    uint32_t                 status;

} SPI_OBJECT;

#ifdef __cplusplus // Provide C++ Compatibility

    }

#endif

#endif //PLIB_SERCOM_SPI_MASTER_COMMON_H