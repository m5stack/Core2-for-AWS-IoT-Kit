/*******************************************************************************
  SPI Driver Definitions Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_spi_definitions.h

  Summary:
    SPI Driver Definitions Header File

  Description:
    This file provides implementation-specific definitions for the SPI
    driver's system interface.
*******************************************************************************/

//DOM-IGNORE-BEGIN
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
//DOM-IGNORE-END

#ifndef DRV_SPI_DEFINITIONS_H
#define DRV_SPI_DEFINITIONS_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <device.h>
#include "system/int/sys_int.h"
#include "system/ports/sys_ports.h"
#include "system/dma/sys_dma.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef enum
{
    DRV_SPI_CLOCK_PHASE_VALID_TRAILING_EDGE = 0,
    DRV_SPI_CLOCK_PHASE_VALID_LEADING_EDGE = 1,

    /* Force the compiler to reserve 32-bit memory space for each enum */
    DRV_SPI_CLOCK_PHASE_INVALID = 0xFFFFFFFF

} DRV_SPI_CLOCK_PHASE;

typedef enum
{
    DRV_SPI_CLOCK_POLARITY_IDLE_LOW = 0,
    DRV_SPI_CLOCK_POLARITY_IDLE_HIGH = 1,

    /* Force the compiler to reserve 32-bit memory space for each enum */
    DRV_SPI_CLOCK_POLARITY_INVALID = 0xFFFFFFFF

} DRV_SPI_CLOCK_POLARITY;

typedef enum
{
    DRV_SPI_DATA_BITS_8 = 0,
    DRV_SPI_DATA_BITS_9 = 1,
    DRV_SPI_DATA_BITS_10 = 2,
    DRV_SPI_DATA_BITS_11 = 3,
    DRV_SPI_DATA_BITS_12 = 4,
    DRV_SPI_DATA_BITS_13 = 5,
    DRV_SPI_DATA_BITS_14 = 6,
    DRV_SPI_DATA_BITS_15 = 7,
    DRV_SPI_DATA_BITS_16 = 8,

    /* Force the compiler to reserve 32-bit memory space for each enum */
    DRV_SPI_DATA_BITS_INVALID = 0xFFFFFFFF

} DRV_SPI_DATA_BITS;

typedef enum
{
    DRV_SPI_CS_POLARITY_ACTIVE_LOW = 0,
    DRV_SPI_CS_POLARITY_ACTIVE_HIGH = 1

} DRV_SPI_CS_POLARITY;

// *****************************************************************************
/* SPI Driver Setup Data

  Summary:
    Defines the data required to setup the SPI transfer

  Description:
    This data type defines the data required to setup the SPI transfer. The
    data is passed to the DRV_SPI_TransferSetup API to setup the SPI peripheral
    settings dynamically.

  Remarks:
    None.
*/

typedef struct
{
    uint32_t                        baudRateInHz;

    DRV_SPI_CLOCK_PHASE             clockPhase;

    DRV_SPI_CLOCK_POLARITY          clockPolarity;

    DRV_SPI_DATA_BITS               dataBits;

    SYS_PORT_PIN                    chipSelect;

    DRV_SPI_CS_POLARITY             csPolarity;

} DRV_SPI_TRANSFER_SETUP;

typedef void (*DRV_SPI_PLIB_CALLBACK)( uintptr_t );

typedef bool (*DRV_SPI_PLIB_SETUP) (DRV_SPI_TRANSFER_SETUP *, uint32_t);

typedef bool (*DRV_SPI_PLIB_WRITE_READ)(void*, size_t, void *, size_t);

typedef bool (*DRV_SPI_PLIB_IS_BUSY)(void);

typedef void (* DRV_SPI_PLIB_CALLBACK_REGISTER)(DRV_SPI_PLIB_CALLBACK, uintptr_t);


typedef struct
{
    int32_t         spiTxReadyInt;
    int32_t         spiTxCompleteInt;
    int32_t         spiRxInt;
    int32_t         dmaTxChannelInt;
    int32_t         dmaRxChannelInt;
} DRV_SPI_MULTI_INT_SRC;

typedef union
{
    DRV_SPI_MULTI_INT_SRC               multi;
    int32_t                             spiInterrupt;
    int32_t                             dmaInterrupt;
} DRV_SPI_INT_SRC;

typedef struct
{
    bool                        isSingleIntSrc;
    DRV_SPI_INT_SRC             intSources;
} DRV_SPI_INTERRUPT_SOURCES;


// *****************************************************************************
/* SPI Driver PLIB Interface Data

  Summary:
    Defines the data required to initialize the SPI driver PLIB Interface.

  Description:
    This data type defines the data required to initialize the SPI driver
    PLIB Interface.

  Remarks:
    None.
*/

typedef struct
{
    /* SPI PLIB Setup API */
    DRV_SPI_PLIB_SETUP                   setup;

    /* SPI PLIB writeRead API */
    DRV_SPI_PLIB_WRITE_READ              writeRead;

    /* SPI PLIB Transfer status API */
    DRV_SPI_PLIB_IS_BUSY                 isBusy;

    /* SPI PLIB callback register API */
    DRV_SPI_PLIB_CALLBACK_REGISTER       callbackRegister;

} DRV_SPI_PLIB_INTERFACE;

// *****************************************************************************
/* SPI Driver Initialization Data

  Summary:
    Defines the data required to initialize the SPI driver

  Description:
    This data type defines the data required to initialize or the SPI driver.

  Remarks:
    None.
*/

typedef struct
{
    /* Identifies the PLIB API set to be used by the driver to access the
     * peripheral. */
    const DRV_SPI_PLIB_INTERFACE*   spiPlib;

    /* SPI transmit DMA channel. */
    SYS_DMA_CHANNEL                 dmaChannelTransmit;

    /* SPI receive DMA channel. */
    SYS_DMA_CHANNEL                 dmaChannelReceive;

    /* SPI transmit register address used for DMA operation. */
    void*                           spiTransmitAddress;

    /* SPI receive register address used for DMA operation. */
    void*                           spiReceiveAddress;
    /* Memory Pool for Client Objects */
    uintptr_t                       clientObjPool;

    /* Number of clients */
    size_t                          numClients;

    const uint32_t*                 remapDataBits;

    const uint32_t*                 remapClockPolarity;

    const uint32_t*                 remapClockPhase;

    /* Size of buffer objects queue */
    uint32_t                        transferObjPoolSize;

    /* Pointer to the buffer pool */
    uintptr_t                       transferObjPool;

    const DRV_SPI_INTERRUPT_SOURCES*      interruptSources;
} DRV_SPI_INIT;


//DOM-IGNORE-BEGIN
#ifdef __cplusplus

    }

#endif
//DOM-IGNORE-END

#endif // #ifndef DRV_SPI_DEFINITIONS_H

/*******************************************************************************
 End of File
*/

