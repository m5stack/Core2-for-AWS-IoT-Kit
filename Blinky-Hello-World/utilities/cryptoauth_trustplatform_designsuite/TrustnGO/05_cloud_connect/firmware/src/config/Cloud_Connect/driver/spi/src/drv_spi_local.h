/*******************************************************************************
  SPI Driver Local Data Structures

  Company:
    Microchip Technology Inc.

  File Name:
    drv_spi_local.h

  Summary:
    SPI Driver Local Data Structures

  Description:
    Driver Local Data Structures
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

#ifndef _DRV_SPI_LOCAL_H
#define _DRV_SPI_LOCAL_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "driver/spi/drv_spi.h"
#include "osal/osal.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************

/* SPI Driver Handle Macros*/
#define DRV_SPI_INDEX_MASK                      (0x000000FF)

#define DRV_SPI_INSTANCE_MASK                   (0x0000FF00)

#define DRV_SPI_TOKEN_MAX                       (0xFFFF)


#define _USE_FREQ_CONFIGURED_IN_CLOCK_MANAGER       0
#define NULL_INDEX                                  0xFF

// *****************************************************************************
/* SPI Client-Specific Driver Status

  Summary:
    Defines the client-specific status of the SPI driver.

  Description:
    This enumeration defines the client-specific status codes of the SPI
    driver.

  Remarks:
    Returned by the DRV_SPI_ClientStatus function.
*/

typedef enum
{
    /* An error has occurred.*/
    DRV_SPI_CLIENT_STATUS_ERROR    = DRV_CLIENT_STATUS_ERROR,

    /* The driver is closed, no operations for this client are ongoing,
    and/or the given handle is invalid. */
    DRV_SPI_CLIENT_STATUS_CLOSED   = DRV_CLIENT_STATUS_CLOSED,

    /* The driver is currently busy and cannot start additional operations. */
    DRV_SPI_CLIENT_STATUS_BUSY     = DRV_CLIENT_STATUS_BUSY,

    /* The module is running and ready for additional operations */
    DRV_SPI_CLIENT_STATUS_READY    = DRV_CLIENT_STATUS_READY

} DRV_SPI_CLIENT_STATUS;

// *****************************************************************************
/* SPI Transfer Object State

  Summary:
    Defines the status of the SPI Transfer Object.

  Description:
    This enumeration defines the status of the SPI Transfer Object.

  Remarks:
    None.
*/

typedef enum
{
    DRV_SPI_TRANSFER_OBJ_IS_FREE,

    DRV_SPI_TRANSFER_OBJ_IS_IN_QUEUE,

    DRV_SPI_TRANSFER_OBJ_IS_PROCESSING,

}DRV_SPI_TRANSFER_OBJ_STATE;

// *****************************************************************************
/* SPI Driver Transfer Object

  Summary:
    Object used to keep track of a client's buffer.

  Description:
    None.

  Remarks:
    None.
*/

typedef struct _DRV_SPI_TRANSFER_OBJ
{
    /* True if object is allocated */
    bool                            inUse;

    /* Pointer to the receive data */
    void*                           pReceiveData;

    /* Pointer to the transmit data */
    void*                           pTransmitData;

    /* Number of bytes to be written */
    size_t                          txSize;

    /* Number of bytes to be read */
    size_t                          rxSize;


    /* Current status of the buffer */
    DRV_SPI_TRANSFER_EVENT          event;

    /* Current state of the buffer */
    DRV_SPI_TRANSFER_OBJ_STATE      currentState;

    /* Handle to the client that owns this buffer object when it was queued */
    DRV_HANDLE                      clientHandle;

    /* Buffer Handle object that was assigned to this buffer when it was added to
     * the queue */
    DRV_SPI_TRANSFER_HANDLE         transferHandle;

    /* Next buffer pointer */
    struct _DRV_SPI_TRANSFER_OBJ*   next;

} DRV_SPI_TRANSFER_OBJ;

// *****************************************************************************
/* SPI Driver Instance Object

  Summary:
    Object used to keep any data required for an instance of the SPI driver.

  Description:
    None.

  Remarks:
    None.
*/

typedef struct
{
    /* Flag to indicate this object is in use  */
    bool                            inUse;

    /* Flag to indicate that driver has been opened Exclusively*/
    bool                            isExclusive;

    /* Keep track of the number of clients
     * that have opened this driver
     */
    size_t                          nClients;

    /* Maximum number of clients */
    size_t                          nClientsMax;

    /* Memory pool for Client Objects */
    uintptr_t                       clientObjPool;

    /* The status of the driver */
    SYS_STATUS                      status;

    /* PLIB API list that will be used by the driver to access the hardware */
    const DRV_SPI_PLIB_INTERFACE*   spiPlib;

    /* start of the memory pool for transfer objects */
    DRV_SPI_TRANSFER_OBJ*           transferObjPool;

    /* size/depth of the queue */
    uint32_t                        transferObjPoolSize;

    /* Linked list of transfer objects */
    uintptr_t                       transferObjList;

    /* Instance specific token counter used to generate unique client/transfer handles */
    uint16_t                        spiTokenCount;

    /* to identify if we are running from interrupt context or not */
    uint8_t                         interruptNestingCount;

    /* Last client handle. This is compared with the new client handle to
     * decide whether or not to update the client specific SPI parameters. */
    DRV_HANDLE                      lastClientHandle;

    /* Transmit DMA Channel */
    SYS_DMA_CHANNEL                 txDMAChannel;

    /* Receive DMA Channel */
    SYS_DMA_CHANNEL                 rxDMAChannel;

    /* This is the SPI transmit register address. Used for DMA operation. */
    void*                           txAddress;

    /* This is the SPI receive register address. Used for DMA operation. */
    void*                           rxAddress;

    /* Dummy data is read into this variable by RX DMA */
    uint32_t                        rxDummyData;

    /* This holds the number of dummy data to be transmitted */
    size_t                          txDummyDataSize;

    /* This holds the number of dummy data to be received */
    size_t                          rxDummyDataSize;

    const uint32_t*                 remapDataBits;

    const uint32_t*                 remapClockPolarity;

    const uint32_t*                 remapClockPhase;

    bool                            spiTxReadyIntStatus;
    bool                            spiTxCompleteIntStatus;
    bool                            spiRxIntStatus;
    bool                            dmaRxChannelIntStatus;
    bool                            dmaTxChannelIntStatus;
    bool                            spiInterruptStatus;
    bool                            dmaInterruptStatus;

    const DRV_SPI_INTERRUPT_SOURCES*      interruptSources;

    /* Mutex to protect access to the client objects */
    OSAL_MUTEX_DECLARE(mutexClientObjects);

    /* Mutex to protect access to the transfer objects */
    OSAL_MUTEX_DECLARE(mutexTransferObjects);

} DRV_SPI_OBJ;

// *****************************************************************************
/* SPI Driver Client Object

  Summary:
    Object used to track a single client.

  Description:
    This object is used to keep the data necessary to keep track of a single
    client.

  Remarks:
    None.
*/

typedef struct _DRV_SPI_CLIENT_OBJ
{
    /* The hardware instance index associated with the client */
    SYS_MODULE_INDEX                drvIndex;

    /* The IO intent with which the client was opened */
    DRV_IO_INTENT                   ioIntent;

    /* This flags indicates if the object is in use or is
     * available
     */
    bool                            inUse;

    /* Event handler for this function */
    DRV_SPI_TRANSFER_EVENT_HANDLER  eventHandler;

    /* Application Context associated with this client */
    uintptr_t                       context;

    /* Client specific setup */
    DRV_SPI_TRANSFER_SETUP          setup;

    /* Flag to save setup changed status */
    bool                            setupChanged;

    /* Client handle assigned to this client object when it was opened */
    DRV_HANDLE                      clientHandle;

} DRV_SPI_CLIENT_OBJ;

#endif //#ifndef _DRV_SPI_LOCAL_H
