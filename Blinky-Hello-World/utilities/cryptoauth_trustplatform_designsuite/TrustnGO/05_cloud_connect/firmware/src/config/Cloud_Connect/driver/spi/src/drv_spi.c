/*******************************************************************************
  SPI Driver Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    drv_spi.c

  Summary:
    Source code for the SPI driver implementation.

  Description:
    This file contains the source code for the implementation of the SPI driver.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <string.h>
#include "configuration.h"
#include "driver/spi/drv_spi.h"
#include "system/debug/sys_debug.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

/* This is the driver instance object array. */
static DRV_SPI_OBJ gDrvSPIObj[DRV_SPI_INSTANCES_NUMBER];
/* Dummy data being transmitted by TX DMA */
static CACHE_ALIGN uint8_t txDummyData[32];

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

void _DRV_SPI_TX_DMA_CallbackHandler(SYS_DMA_TRANSFER_EVENT event, uintptr_t context);
void _DRV_SPI_RX_DMA_CallbackHandler(SYS_DMA_TRANSFER_EVENT event, uintptr_t context);

static inline uint32_t  _DRV_SPI_MAKE_HANDLE(uint16_t token, uint8_t drvIndex, uint8_t index)
{
    return ((token << 16) | (drvIndex << 8) | index);
}

static inline uint16_t _DRV_SPI_UPDATE_TOKEN(uint16_t token)
{
    token++;

    if (token >= DRV_SPI_TOKEN_MAX)
    {
        token = 1;
    }

    return token;
}

static void _DRV_SPI_DisableInterrupts(DRV_SPI_OBJ* dObj)
{
    bool interruptStatus;
    const DRV_SPI_INTERRUPT_SOURCES* intInfo = dObj->interruptSources;
    const DRV_SPI_MULTI_INT_SRC* multiVector = &intInfo->intSources.multi;
    /* Disable DMA interrupt */
    if((dObj->txDMAChannel != SYS_DMA_CHANNEL_NONE) && (dObj->rxDMAChannel != SYS_DMA_CHANNEL_NONE))
    {
        if (intInfo->isSingleIntSrc == true)
        {
            dObj->dmaInterruptStatus = SYS_INT_SourceDisable((INT_SOURCE)intInfo->intSources.dmaInterrupt);
        }
        else
        {
            /* Disable DMA interrupt sources */
            interruptStatus = SYS_INT_Disable();

            dObj->dmaTxChannelIntStatus = SYS_INT_SourceDisable((INT_SOURCE)multiVector->dmaTxChannelInt);
            dObj->dmaRxChannelIntStatus = SYS_INT_SourceDisable((INT_SOURCE)multiVector->dmaRxChannelInt);

            SYS_INT_Restore(interruptStatus);
        }
    }
    else
    {
        /* Disable SPI interrupt */
        if (intInfo->isSingleIntSrc == true)
        {
            dObj->spiInterruptStatus = SYS_INT_SourceDisable((INT_SOURCE)intInfo->intSources.spiInterrupt);
        }
        else
        {
            interruptStatus = SYS_INT_Disable();
            if(multiVector->spiTxReadyInt != -1)
            {
                dObj->spiTxReadyIntStatus = SYS_INT_SourceDisable((INT_SOURCE)multiVector->spiTxReadyInt);
            }
            if(multiVector->spiTxCompleteInt != -1)
            {
                dObj->spiTxCompleteIntStatus = SYS_INT_SourceDisable((INT_SOURCE)multiVector->spiTxCompleteInt);
            }
            if(multiVector->spiRxInt != -1)
            {
                dObj->spiRxIntStatus = SYS_INT_SourceDisable((INT_SOURCE)multiVector->spiRxInt);
            }
            SYS_INT_Restore(interruptStatus);
        }
    }
}

static void _DRV_SPI_EnableInterrupts(DRV_SPI_OBJ* dObj)
{
    bool interruptStatus;
    const DRV_SPI_INTERRUPT_SOURCES* intInfo = dObj->interruptSources;
    const DRV_SPI_MULTI_INT_SRC* multiVector = &intInfo->intSources.multi;
    /* Enable DMA interrupt */
    if((dObj->txDMAChannel != SYS_DMA_CHANNEL_NONE) && (dObj->rxDMAChannel != SYS_DMA_CHANNEL_NONE))
    {
        if (intInfo->isSingleIntSrc == true)
        {
            SYS_INT_SourceRestore((INT_SOURCE)intInfo->intSources.dmaInterrupt, dObj->dmaInterruptStatus);
        }
        else
        {
            interruptStatus = SYS_INT_Disable();

            /* Enable DMA interrupt sources */
            SYS_INT_SourceRestore((INT_SOURCE)multiVector->dmaTxChannelInt, dObj->dmaTxChannelIntStatus);
            SYS_INT_SourceRestore((INT_SOURCE)multiVector->dmaRxChannelInt, dObj->dmaRxChannelIntStatus);

            SYS_INT_Restore(interruptStatus);
        }
    }
    else
    {
        /* Enable SPI interrupt */
        if (intInfo->isSingleIntSrc == true)
        {
            SYS_INT_SourceRestore((INT_SOURCE)intInfo->intSources.spiInterrupt, dObj->spiInterruptStatus);
        }
        else
        {
            interruptStatus = SYS_INT_Disable();
            if(multiVector->spiTxReadyInt != -1)
            {
                SYS_INT_SourceRestore((INT_SOURCE)multiVector->spiTxReadyInt, dObj->spiTxReadyIntStatus);
            }
            if(multiVector->spiTxCompleteInt != -1)
            {
                SYS_INT_SourceRestore((INT_SOURCE)multiVector->spiTxCompleteInt,dObj->spiTxCompleteIntStatus);
            }
            if(multiVector->spiRxInt != -1)
            {
                SYS_INT_SourceRestore((INT_SOURCE)multiVector->spiRxInt, dObj->spiRxIntStatus);
            }
            SYS_INT_Restore(interruptStatus);
        }
    }
}

static bool _DRV_SPI_ResourceLock(DRV_SPI_OBJ * dObj)
{
    /* We will allow buffers to be added in the interrupt context of the SPI
     * driver. But we must make sure that if we are inside interrupt, then we
     * should not modify mutexes. */
    if(dObj->interruptNestingCount == 0)
    {
        /* Grab a mutex. This is okay because we are not in an interrupt context */
        if(OSAL_MUTEX_Lock(&(dObj->mutexTransferObjects), OSAL_WAIT_FOREVER) == OSAL_RESULT_FALSE)
        {
            return false;
        }
        /* We will disable interrupts so that the queue status does not get updated asynchronously */
        _DRV_SPI_DisableInterrupts(dObj);
    }

    return true;
}

static void _DRV_SPI_ResourceUnlock(DRV_SPI_OBJ * dObj)
{
    if(dObj->interruptNestingCount == 0)
    {
        _DRV_SPI_EnableInterrupts(dObj);

        /* Release mutex */
        OSAL_MUTEX_Unlock(&(dObj->mutexTransferObjects));
    }
}

static DRV_SPI_CLIENT_OBJ * _DRV_SPI_DriverHandleValidate(DRV_HANDLE handle)
{
    /* This function returns the pointer to the client object that is
       associated with this handle if the handle is valid. Returns NULL
       otherwise. */

    uint32_t drvInstance = 0;
    DRV_SPI_CLIENT_OBJ* clientObj = (DRV_SPI_CLIENT_OBJ*)NULL;

    if((handle != DRV_HANDLE_INVALID) && (handle != 0))
    {
        /* Extract the drvInstance value from the handle */
        drvInstance = ((handle & DRV_SPI_INSTANCE_MASK) >> 8);

        if (drvInstance >= DRV_SPI_INSTANCES_NUMBER)
        {
            return (NULL);
        }

        if ((handle & DRV_SPI_INDEX_MASK) >= gDrvSPIObj[drvInstance].nClientsMax)
        {
            return (NULL);
        }

        /* Extract the client index and obtain the client object */
        clientObj = &((DRV_SPI_CLIENT_OBJ *)gDrvSPIObj[drvInstance].clientObjPool)[handle & DRV_SPI_INDEX_MASK];

        if ((clientObj->clientHandle != handle) || (clientObj->inUse == false))
        {
            return (NULL);
        }
    }

    return(clientObj);
}

static DRV_SPI_TRANSFER_OBJ* _DRV_SPI_FreeTransferObjGet(DRV_SPI_CLIENT_OBJ* clientObj)
{
    uint32_t index;
    DRV_SPI_OBJ* dObj = (DRV_SPI_OBJ* )&gDrvSPIObj[clientObj->drvIndex];
    DRV_SPI_TRANSFER_OBJ* pTransferObj = (DRV_SPI_TRANSFER_OBJ*)dObj->transferObjPool;

    for (index = 0; index < dObj->transferObjPoolSize; index++)
    {
        if (pTransferObj[index].inUse == false)
        {
            pTransferObj[index].inUse = true;
            pTransferObj[index].next = NULL;

            /* Generate a unique buffer handle consisting of an incrementing
             * token counter, driver index and the buffer index.
             */
            pTransferObj[index].transferHandle = (DRV_SPI_TRANSFER_HANDLE)_DRV_SPI_MAKE_HANDLE(
                dObj->spiTokenCount, (uint8_t)clientObj->drvIndex, index);

            /* Update the token for next time */
            dObj->spiTokenCount = _DRV_SPI_UPDATE_TOKEN(dObj->spiTokenCount);

            return &pTransferObj[index];
        }
    }
    return NULL;
}

static bool _DRV_SPI_TransferObjAddToList(
    DRV_SPI_OBJ* dObj,
    DRV_SPI_TRANSFER_OBJ* transferObj
)
{
    DRV_SPI_TRANSFER_OBJ** pTransferObjList;
    bool isFirstTransferInList = false;

    pTransferObjList = (DRV_SPI_TRANSFER_OBJ**)&(dObj->transferObjList);

    // Is the buffer object list empty?
    if (*pTransferObjList == NULL)
    {
        *pTransferObjList = transferObj;
        isFirstTransferInList = true;
    }
    else
    {
        // List is not empty. Iterate to the end of the buffer object list.
        while (*pTransferObjList != NULL)
        {
            if ((*pTransferObjList)->next == NULL)
            {
                // End of the list reached, add the buffer here.
                (*pTransferObjList)->next = transferObj;
                break;
            }
            else
            {
                pTransferObjList = (DRV_SPI_TRANSFER_OBJ**)&((*pTransferObjList)->next);
            }
        }
    }

    return isFirstTransferInList;
}

static DRV_SPI_TRANSFER_OBJ* _DRV_SPI_TransferObjListGet( DRV_SPI_OBJ* dObj )
{
    DRV_SPI_TRANSFER_OBJ* pTransferObj = NULL;

    // Return the element at the head of the linked list
    pTransferObj = (DRV_SPI_TRANSFER_OBJ*)dObj->transferObjList;

    return pTransferObj;
}

static void _DRV_SPI_RemoveTransferObjFromList( DRV_SPI_OBJ* dObj )
{
    DRV_SPI_TRANSFER_OBJ** pTransferObjList;

    pTransferObjList = (DRV_SPI_TRANSFER_OBJ**)&(dObj->transferObjList);

    // Remove the element at the head of the linked list
    if (*pTransferObjList != NULL)
    {
        /* Save the buffer object to be removed. Set the next buffer object as
         * the new head of the linked list. Reset the removed buffer object. */

        DRV_SPI_TRANSFER_OBJ* temp = *pTransferObjList;
        *pTransferObjList = (*pTransferObjList)->next;
        temp->currentState = DRV_SPI_TRANSFER_OBJ_IS_FREE;
        temp->next = NULL;
        temp->inUse = false;
    }
}

static void _DRV_SPI_RemoveClientTransfersFromList(
    DRV_SPI_OBJ* dObj,
    DRV_SPI_CLIENT_OBJ* clientObj
)
{
    DRV_SPI_TRANSFER_OBJ** pTransferObjList;
    DRV_SPI_TRANSFER_OBJ* delTransferObj = NULL;

    pTransferObjList = (DRV_SPI_TRANSFER_OBJ**)&(dObj->transferObjList);

    while (*pTransferObjList != NULL)
    {
        // Do not remove the buffer object that is already in process
        if (((*pTransferObjList)->clientHandle == clientObj->clientHandle) && 
                ((*pTransferObjList)->currentState == DRV_SPI_TRANSFER_OBJ_IS_IN_QUEUE))
        {
            // Save the node to be deleted off the list
            delTransferObj = *pTransferObjList;

            // Update the current node to point to the deleted node's next
            *pTransferObjList = (DRV_SPI_TRANSFER_OBJ*)(*pTransferObjList)->next;

            // Reset the deleted node
            delTransferObj->currentState = DRV_SPI_TRANSFER_OBJ_IS_FREE;
            delTransferObj->event = DRV_SPI_TRANSFER_EVENT_COMPLETE;
            delTransferObj->next = NULL;
            delTransferObj->inUse = false;
        }
        else
        {
            // Move to the next node
            pTransferObjList = (DRV_SPI_TRANSFER_OBJ**)&((*pTransferObjList)->next);
        }
    }
}

static void _DRV_SPI_StartDMATransfer(DRV_SPI_TRANSFER_OBJ* transferObj)
{
    DRV_SPI_CLIENT_OBJ* clientObj;
    DRV_SPI_OBJ* dObj;
    uint32_t size = 0;
    /* To avoid unused build error */
    (void) size;

    /* Get the client object that owns this buffer */
    clientObj = &((DRV_SPI_CLIENT_OBJ *)gDrvSPIObj[((transferObj->clientHandle & DRV_SPI_INSTANCE_MASK) >> 8)].clientObjPool)
    [transferObj->clientHandle & DRV_SPI_INDEX_MASK];

    dObj = (DRV_SPI_OBJ*)&gDrvSPIObj[clientObj->drvIndex];

    dObj->txDummyDataSize = 0;
    dObj->rxDummyDataSize = 0;


    if (transferObj->rxSize >= transferObj->txSize)
    {
        /* Dummy data will be sent by the TX DMA */
        dObj->txDummyDataSize = (transferObj->rxSize - transferObj->txSize);
    }
    else
    {
        /* Dummy data will be received by the RX DMA */
        dObj->rxDummyDataSize = (transferObj->txSize - transferObj->rxSize);
    }

    /* Register callbacks for DMA */
    SYS_DMA_ChannelCallbackRegister(dObj->txDMAChannel, _DRV_SPI_TX_DMA_CallbackHandler, (uintptr_t)transferObj);
    SYS_DMA_ChannelCallbackRegister(dObj->rxDMAChannel, _DRV_SPI_RX_DMA_CallbackHandler, (uintptr_t)transferObj);

    if(clientObj->setup.dataBits == DRV_SPI_DATA_BITS_8)
    {
        SYS_DMA_DataWidthSetup(dObj->rxDMAChannel, SYS_DMA_WIDTH_8_BIT);
        SYS_DMA_DataWidthSetup(dObj->txDMAChannel, SYS_DMA_WIDTH_8_BIT);
    }
    else
    {
        SYS_DMA_DataWidthSetup(dObj->rxDMAChannel, SYS_DMA_WIDTH_16_BIT);
        SYS_DMA_DataWidthSetup(dObj->txDMAChannel, SYS_DMA_WIDTH_16_BIT);
    }

    if (transferObj->rxSize == 0)
    {
        /* Configure the RX DMA channel - to receive dummy data */
        SYS_DMA_AddressingModeSetup(dObj->rxDMAChannel, SYS_DMA_SOURCE_ADDRESSING_MODE_FIXED, SYS_DMA_DESTINATION_ADDRESSING_MODE_FIXED);
        size = dObj->rxDummyDataSize;
        dObj->rxDummyDataSize = 0;
        SYS_DMA_ChannelTransfer(dObj->rxDMAChannel, (const void*)dObj->rxAddress, (const void *)&dObj->rxDummyData, size);
    }
    else
    {
        /* Configure the RX DMA channel - to receive data in receive buffer */
        SYS_DMA_AddressingModeSetup(dObj->rxDMAChannel, SYS_DMA_SOURCE_ADDRESSING_MODE_FIXED, SYS_DMA_DESTINATION_ADDRESSING_MODE_INCREMENTED);
        SYS_DMA_ChannelTransfer(dObj->rxDMAChannel, (const void*)dObj->rxAddress, (const void *)transferObj->pReceiveData, transferObj->rxSize);
    }

    if (transferObj->txSize == 0)
    {
        /* Configure the TX DMA channel - to send dummy data */
        SYS_DMA_AddressingModeSetup(dObj->txDMAChannel, SYS_DMA_SOURCE_ADDRESSING_MODE_FIXED, SYS_DMA_DESTINATION_ADDRESSING_MODE_FIXED);
        size = dObj->txDummyDataSize;
        dObj->txDummyDataSize = 0;
        SYS_DMA_ChannelTransfer(dObj->txDMAChannel, (const void *)txDummyData, (const void*)dObj->txAddress, size);
    }
    else
    {
        /* Configure the transmit DMA channel - to send data from transmit buffer */
        SYS_DMA_AddressingModeSetup(dObj->txDMAChannel, SYS_DMA_SOURCE_ADDRESSING_MODE_INCREMENTED, SYS_DMA_DESTINATION_ADDRESSING_MODE_FIXED);

        /* The DMA transfer is split into two for the case where rxSize > 0 && rxSize < txSize */
        if (dObj->rxDummyDataSize > 0)
        {
            size = transferObj->rxSize;
        }
        else
        {
            size = transferObj->txSize;
        }
        SYS_DMA_ChannelTransfer(dObj->txDMAChannel, (const void *)transferObj->pTransmitData, (const void*)dObj->txAddress, size);
    }
}

static void _DRV_SPI_UpdateTransferSetupAndAssertCS(
    DRV_SPI_TRANSFER_OBJ* transferObj
)
{
    DRV_SPI_OBJ* dObj;
    DRV_SPI_CLIENT_OBJ* clientObj;

    /* Get the client object that owns this buffer */
    clientObj = &((DRV_SPI_CLIENT_OBJ *)gDrvSPIObj[((transferObj->clientHandle & DRV_SPI_INSTANCE_MASK) >> 8)].clientObjPool)
    [transferObj->clientHandle & DRV_SPI_INDEX_MASK];

    dObj = (DRV_SPI_OBJ*)&gDrvSPIObj[clientObj->drvIndex];

    /* Update the PLIB Setup if current request is from a different client or
     * setup has been changed dynamically for the client */
    if((transferObj->clientHandle != dObj->lastClientHandle) || (clientObj->setupChanged == true))
    {
        dObj->spiPlib->setup(&clientObj->setup, _USE_FREQ_CONFIGURED_IN_CLOCK_MANAGER);
        dObj->lastClientHandle = transferObj->clientHandle;
        clientObj->setupChanged = false;
    }

    /* Assert chip select if configured */
    if(clientObj->setup.chipSelect != SYS_PORT_PIN_NONE)
    {
        SYS_PORT_PinWrite(clientObj->setup.chipSelect, (bool)(clientObj->setup.csPolarity));
    }
}

static void _DRV_SPI_PlibCallbackHandler(uintptr_t contextHandle)
{
    DRV_SPI_OBJ* dObj                    = (DRV_SPI_OBJ*)contextHandle;
    DRV_SPI_CLIENT_OBJ* clientObj        = (DRV_SPI_CLIENT_OBJ*)NULL;
    DRV_SPI_TRANSFER_OBJ* transferObj    = (DRV_SPI_TRANSFER_OBJ*)NULL;
    DRV_SPI_TRANSFER_EVENT event;
    DRV_SPI_TRANSFER_HANDLE transferHandle;

    if((dObj->inUse == false) || (dObj->status != SYS_STATUS_READY))
    {
        /* This instance of the driver is not initialized. Don't
         * do anything */
        return;
    }

    /* Get the transfer object at the head of the list */
    transferObj = _DRV_SPI_TransferObjListGet(dObj);

    /* Get the client object that owns this buffer */
    clientObj = &((DRV_SPI_CLIENT_OBJ *)gDrvSPIObj[((transferObj->clientHandle & DRV_SPI_INSTANCE_MASK) >> 8)].clientObjPool)
    [transferObj->clientHandle & DRV_SPI_INDEX_MASK];

    /* De-assert Chip Select if it is defined by user */
    if(clientObj->setup.chipSelect != SYS_PORT_PIN_NONE)
    {
        SYS_PORT_PinWrite(clientObj->setup.chipSelect, !((bool)(clientObj->setup.csPolarity)));
    }

    /* Check if the client that submitted the request is active? */
    if (clientObj->clientHandle == transferObj->clientHandle)
    {
        transferObj->event = DRV_SPI_TRANSFER_EVENT_COMPLETE;

        /* Save the transfer handle and event locally before freeing the transfer object*/
        event = transferObj->event;
        transferHandle = transferObj->transferHandle;

        /* Free the completed buffer.
         * This is done before giving callback to allow application to use the freed
         * buffer and queue in a new request from within the callback */

        _DRV_SPI_RemoveTransferObjFromList(dObj);

        if(clientObj->eventHandler != NULL)
        {
            /* Call the event handler. We additionally increment the
            interrupt nesting count which lets the driver functions
            that are called from the event handler know that an
            interrupt context is active. */
            dObj->interruptNestingCount++;

            clientObj->eventHandler(event, transferHandle, clientObj->context);

            /* Event handler has completed, so decrement the nesting count now */
            dObj->interruptNestingCount--;
        }
    }
    else
    {
        /* Free the completed buffer */
        _DRV_SPI_RemoveTransferObjFromList(dObj);
    }

     /* Get the transfer object at the head of the list */
    transferObj = _DRV_SPI_TransferObjListGet(dObj);

    /* Process the next transfer buffer */
    if((transferObj != NULL) && (transferObj->currentState == DRV_SPI_TRANSFER_OBJ_IS_IN_QUEUE))
    {
        _DRV_SPI_UpdateTransferSetupAndAssertCS(transferObj);

        transferObj->currentState = DRV_SPI_TRANSFER_OBJ_IS_PROCESSING;

        dObj->spiPlib->writeRead(
            transferObj->pTransmitData,
            transferObj->txSize,
            transferObj->pReceiveData,
            transferObj->rxSize
        );
    }
}


void _DRV_SPI_TX_DMA_CallbackHandler(
    SYS_DMA_TRANSFER_EVENT event,
    uintptr_t context
)
{
    DRV_SPI_TRANSFER_OBJ* transferObj = (DRV_SPI_TRANSFER_OBJ*)context;
    DRV_SPI_CLIENT_OBJ* clientObj;
    DRV_SPI_OBJ* dObj;

    /* Get the client object that owns this buffer */
    clientObj = &((DRV_SPI_CLIENT_OBJ *)gDrvSPIObj[((transferObj->clientHandle & DRV_SPI_INSTANCE_MASK) >> 8)].clientObjPool)
    [transferObj->clientHandle & DRV_SPI_INDEX_MASK];

    dObj = &gDrvSPIObj[clientObj->drvIndex];

    if (dObj->txDummyDataSize > 0)
    {
        /* Configure DMA channel to transmit (dummy data) from the same location
         * (Source address not incremented) */
        SYS_DMA_AddressingModeSetup(dObj->txDMAChannel, SYS_DMA_SOURCE_ADDRESSING_MODE_FIXED, SYS_DMA_DESTINATION_ADDRESSING_MODE_FIXED);

        /* Configure the transmit DMA channel */
        SYS_DMA_ChannelTransfer(dObj->txDMAChannel, (const void*)txDummyData, (const void*)dObj->txAddress, dObj->txDummyDataSize);

        dObj->txDummyDataSize = 0;
    }
}

void _DRV_SPI_RX_DMA_CallbackHandler(
    SYS_DMA_TRANSFER_EVENT event,
    uintptr_t context
)
{
    DRV_SPI_TRANSFER_OBJ* transferObj = (DRV_SPI_TRANSFER_OBJ*)context;
    DRV_SPI_CLIENT_OBJ* clientObj;
    DRV_SPI_OBJ* dObj;
    DRV_SPI_TRANSFER_EVENT transferEvent;
    DRV_SPI_TRANSFER_HANDLE transferHandle;

    /* Get the client object that owns this buffer */
    clientObj = &((DRV_SPI_CLIENT_OBJ *)gDrvSPIObj[((transferObj->clientHandle & DRV_SPI_INSTANCE_MASK) >> 8)].clientObjPool)
    [transferObj->clientHandle & DRV_SPI_INDEX_MASK];

    dObj = &gDrvSPIObj[clientObj->drvIndex];

    if (dObj->rxDummyDataSize > 0)
    {
        /* Configure DMA to receive dummy data */
        SYS_DMA_AddressingModeSetup(dObj->rxDMAChannel, SYS_DMA_SOURCE_ADDRESSING_MODE_FIXED, SYS_DMA_DESTINATION_ADDRESSING_MODE_FIXED);

        SYS_DMA_ChannelTransfer(dObj->rxDMAChannel, (const void*)dObj->rxAddress, (const void *)&dObj->rxDummyData, dObj->rxDummyDataSize);

        SYS_DMA_ChannelTransfer(dObj->txDMAChannel, (const void *)&((uint8_t*)transferObj->pTransmitData)[transferObj->rxSize], (const void*)dObj->txAddress, dObj->rxDummyDataSize);

        dObj->rxDummyDataSize = 0;
    }
    else
    {
        /* Make sure the shift register is empty before de-asserting the CS line */
        while (dObj->spiPlib->isBusy());

        /* De-assert Chip Select if it is defined by user */
        if(clientObj->setup.chipSelect != SYS_PORT_PIN_NONE)
        {
            SYS_PORT_PinWrite(clientObj->setup.chipSelect, !((bool)(clientObj->setup.csPolarity)));
        }

        /* Check if the client that submitted the request is active? */
        if (clientObj->clientHandle == transferObj->clientHandle)
        {
            /* Set the events */
            if(event == SYS_DMA_TRANSFER_COMPLETE)
            {
                transferObj->event = DRV_SPI_TRANSFER_EVENT_COMPLETE;
            }
            else if(event == SYS_DMA_TRANSFER_ERROR)
            {
                transferObj->event = DRV_SPI_TRANSFER_EVENT_ERROR;
            }

            /* Save the transfer handle and event locally before freeing the transfer object*/
            transferEvent = transferObj->event;
            transferHandle = transferObj->transferHandle;

            /* Free the completed buffer.
             * This is done before giving callback to allow application to use the freed
             * buffer and queue in a new request from within the callback */

            _DRV_SPI_RemoveTransferObjFromList(dObj);

            if(clientObj->eventHandler != NULL)
            {
                /* Call the event handler. We additionally increment the
                interrupt nesting count which lets the driver functions
                that are called from the event handler know that an
                interrupt context is active. */
                dObj->interruptNestingCount++;

                clientObj->eventHandler(transferEvent, transferHandle, clientObj->context);

                /* Event handler has completed, so decrement the nesting count now */
                dObj->interruptNestingCount--;
            }
        }
        else
        {
            /* Free the completed buffer */
            _DRV_SPI_RemoveTransferObjFromList(dObj);
        }

        /* Get the next transfer object at the head of the list */
        transferObj = _DRV_SPI_TransferObjListGet(dObj);

        if((transferObj != NULL) && (transferObj->currentState == DRV_SPI_TRANSFER_OBJ_IS_IN_QUEUE))
        {
            /* Process the next transfer buffer */
            _DRV_SPI_UpdateTransferSetupAndAssertCS(transferObj);
            transferObj->currentState = DRV_SPI_TRANSFER_OBJ_IS_PROCESSING;
            _DRV_SPI_StartDMATransfer(transferObj);
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: SPI Driver Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

SYS_MODULE_OBJ DRV_SPI_Initialize (
    const SYS_MODULE_INDEX drvIndex,
    const SYS_MODULE_INIT* const init
)
{
    DRV_SPI_OBJ* dObj = (DRV_SPI_OBJ*)NULL;
    DRV_SPI_INIT* spiInit = (DRV_SPI_INIT*)init;

    size_t  txDummyDataIdx;

    /* Validate the request */
    if(drvIndex >= DRV_SPI_INSTANCES_NUMBER)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Invalid driver instance");
        return SYS_MODULE_OBJ_INVALID;
    }

    if(gDrvSPIObj[drvIndex].inUse == true)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Instance already in use");
        return SYS_MODULE_OBJ_INVALID;
    }

    /* Allocate the driver object */
    dObj = &gDrvSPIObj[drvIndex];

    /* Create mutexes */
    if(OSAL_MUTEX_Create(&(dObj->mutexClientObjects)) != OSAL_RESULT_TRUE)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    if(OSAL_MUTEX_Create(&(dObj->mutexTransferObjects)) != OSAL_RESULT_TRUE)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    dObj->inUse = true;

    /* Update the driver parameters */
    dObj->spiPlib                   = spiInit->spiPlib;
    dObj->transferObjPool           = (DRV_SPI_TRANSFER_OBJ*)spiInit->transferObjPool;
    dObj->transferObjPoolSize       = spiInit->transferObjPoolSize;
    dObj->transferObjList           = (uintptr_t)NULL;
    dObj->clientObjPool             = spiInit->clientObjPool;
    dObj->nClientsMax               = spiInit->numClients;
    dObj->nClients                  = 0;
    dObj->spiTokenCount             = 1;
    dObj->lastClientHandle          = DRV_HANDLE_INVALID;
    dObj->interruptNestingCount     = 0;
    dObj->isExclusive               = false;
    dObj->txDMAChannel              = spiInit->dmaChannelTransmit;
    dObj->rxDMAChannel              = spiInit->dmaChannelReceive;
    dObj->txAddress                 = spiInit->spiTransmitAddress;
    dObj->rxAddress                 = spiInit->spiReceiveAddress;
    dObj->remapDataBits             = spiInit->remapDataBits;
    dObj->remapClockPolarity        = spiInit->remapClockPolarity;
    dObj->remapClockPhase           = spiInit->remapClockPhase;
    dObj->interruptSources          = spiInit->interruptSources;

    for (txDummyDataIdx = 0; txDummyDataIdx < sizeof(txDummyData); txDummyDataIdx++)
    {
        txDummyData[txDummyDataIdx] = 0xFF;
    }

    if((dObj->txDMAChannel == SYS_DMA_CHANNEL_NONE) || (dObj->rxDMAChannel == SYS_DMA_CHANNEL_NONE))
    {
        /* Register a callback with SPI PLIB.
         * dObj as a context parameter will be used to distinguish the events
         * from different instances. */
        dObj->spiPlib->callbackRegister(&_DRV_SPI_PlibCallbackHandler, (uintptr_t)dObj);
    }
    else
    {
        /* This means DMA has to be used for SPI transfer.
         * DMA Callbacks will be set for every transfer later. */
    }

    /* Update the status */
    dObj->status = SYS_STATUS_READY;

    /* Return the object structure */
    return ( (SYS_MODULE_OBJ)drvIndex );
}

SYS_STATUS DRV_SPI_Status( SYS_MODULE_OBJ object)
{
    /* Validate the request */
    if((object == SYS_MODULE_OBJ_INVALID) || (object >= DRV_SPI_INSTANCES_NUMBER))
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Invalid system object handle");
        return SYS_STATUS_UNINITIALIZED;
    }

    return (gDrvSPIObj[object].status);
}

DRV_HANDLE DRV_SPI_Open(
    const SYS_MODULE_INDEX drvIndex,
    const DRV_IO_INTENT ioIntent
)
{
    DRV_SPI_CLIENT_OBJ* clientObj;
    DRV_SPI_OBJ* dObj = NULL;
    uint32_t iClient;

    /* Validate the request */
    if (drvIndex >= DRV_SPI_INSTANCES_NUMBER)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Invalid Driver Instance");
        return DRV_HANDLE_INVALID;
    }

    dObj = &gDrvSPIObj[drvIndex];

    /* Guard against multiple threads trying to open the driver */
    if (OSAL_MUTEX_Lock(&dObj->mutexClientObjects , OSAL_WAIT_FOREVER ) == OSAL_RESULT_FALSE)
    {
        return DRV_HANDLE_INVALID;
    }

    if((dObj->status != SYS_STATUS_READY) || (dObj->inUse == false))
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Was the driver initialized?");
        OSAL_MUTEX_Unlock( &dObj->mutexClientObjects);
        return DRV_HANDLE_INVALID;
    }

    if(dObj->isExclusive == true)
    {
        /* Driver is already opened with exclusive access. Hence, cannot be opened again*/
        OSAL_MUTEX_Unlock( &dObj->mutexClientObjects);
        return DRV_HANDLE_INVALID;
    }

    if((dObj->nClients > 0) && (ioIntent & DRV_IO_INTENT_EXCLUSIVE))
    {
        /* Exclusive access is requested while the driver is already opened by other client */
        OSAL_MUTEX_Unlock( &dObj->mutexClientObjects);
        return(DRV_HANDLE_INVALID);
    }

    for(iClient = 0; iClient != dObj->nClientsMax; iClient++)
    {
        clientObj = &((DRV_SPI_CLIENT_OBJ *)dObj->clientObjPool)[iClient];

        if(clientObj->inUse == false)
        {
            /* This means we have a free client object to use */
            clientObj->inUse = true;

            if(ioIntent & DRV_IO_INTENT_EXCLUSIVE)
            {
                /* Set the driver exclusive flag */
                dObj->isExclusive = true;
            }

            dObj->nClients ++;

            /* Generate the client handle */
            clientObj->clientHandle = (DRV_HANDLE)_DRV_SPI_MAKE_HANDLE(dObj->spiTokenCount, 
                    (uint8_t)drvIndex, iClient);

            /* Increment the instance specific token counter */
            dObj->spiTokenCount = _DRV_SPI_UPDATE_TOKEN(dObj->spiTokenCount);

            /* We have found a client object, now release the mutex */
            OSAL_MUTEX_Unlock(&(dObj->mutexClientObjects));

            /* This driver will always work in Non-Blocking mode */
            clientObj->ioIntent             = (DRV_IO_INTENT)(ioIntent | DRV_IO_INTENT_NONBLOCKING);

            /* Initialize other elements in Client Object */
            clientObj->eventHandler         = NULL;
            clientObj->context              = (uintptr_t)NULL;
            clientObj->setup.chipSelect     = SYS_PORT_PIN_NONE;
            clientObj->setupChanged         = false;
            clientObj->drvIndex             = drvIndex;

            return clientObj->clientHandle;
        }
    }

    /* Could not find a client object. Release the mutex and return with an invalid handle. */
    OSAL_MUTEX_Unlock(&(dObj->mutexClientObjects));

    return DRV_HANDLE_INVALID;
}

void DRV_SPI_Close( DRV_HANDLE handle )
{
    /* This function closes the client, The client objects are deallocated and
     * returned to the free pool. */

    DRV_SPI_CLIENT_OBJ* clientObj;
    DRV_SPI_OBJ* dObj;

    /* Validate the driver handle */
    clientObj = _DRV_SPI_DriverHandleValidate(handle);

    if(clientObj == NULL)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Invalid Driver Handle");
        return;
    }

    dObj = (DRV_SPI_OBJ *)&gDrvSPIObj[clientObj->drvIndex];

    /* Guard against multiple threads trying to open/close the driver */
    if (OSAL_MUTEX_Lock(&dObj->mutexClientObjects , OSAL_WAIT_FOREVER ) == OSAL_RESULT_FALSE)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Failed to get client mutex lock");
        return;
    }
    /* We will be removing the transfers queued by the client. Guard the linked list
     * against interrupts */
    if(_DRV_SPI_ResourceLock(dObj) == false)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Failed to get resource lock");
        return;
    }

    /* Remove all buffers that this client owns from the driver queue */
    _DRV_SPI_RemoveClientTransfersFromList(dObj, clientObj);

    _DRV_SPI_ResourceUnlock(dObj);

    /* Reduce the number of clients */
    dObj->nClients--;

    /* Reset the exclusive flag */
    dObj->isExclusive = false;

    /* Invalidate the client handle */
    clientObj->clientHandle = DRV_HANDLE_INVALID;

    /* De-allocate the client object */
    clientObj->inUse = false;

    OSAL_MUTEX_Unlock(&(dObj->mutexClientObjects));

    return;
}

void DRV_SPI_TransferEventHandlerSet(
    const DRV_HANDLE handle,
    const DRV_SPI_TRANSFER_EVENT_HANDLER eventHandler,
    uintptr_t context
)
{
    DRV_SPI_CLIENT_OBJ* clientObj = NULL;
    DRV_SPI_OBJ* dObj = (DRV_SPI_OBJ*)NULL;

    /* Validate the driver handle */
    clientObj = _DRV_SPI_DriverHandleValidate(handle);

    if(clientObj == NULL)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Invalid Driver Handle");
        return;
    }

    dObj = (DRV_SPI_OBJ *)&gDrvSPIObj[clientObj->drvIndex];

    if(_DRV_SPI_ResourceLock(dObj) == false)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Failed to get resource lock");
        return;
    }

    /* Save the event handler and context */
    clientObj->eventHandler = eventHandler;
    clientObj->context = context;

    _DRV_SPI_ResourceUnlock(dObj);
}

bool DRV_SPI_TransferSetup (
    const DRV_HANDLE handle,
    DRV_SPI_TRANSFER_SETUP* setup
)
{
    DRV_SPI_CLIENT_OBJ* clientObj = NULL;
    DRV_SPI_OBJ* dObj = (DRV_SPI_OBJ*)NULL;
    DRV_SPI_TRANSFER_SETUP setupRemap;
    bool isSuccess = false;

    /* Validate the driver handle */
    clientObj = _DRV_SPI_DriverHandleValidate(handle);

    if((clientObj != NULL) && (setup != NULL))
    {
        dObj = (DRV_SPI_OBJ*)&gDrvSPIObj[clientObj->drvIndex];

        setupRemap = *setup;

        setupRemap.clockPolarity = (DRV_SPI_CLOCK_POLARITY)dObj->remapClockPolarity[setup->clockPolarity];
        setupRemap.clockPhase = (DRV_SPI_CLOCK_PHASE)dObj->remapClockPhase[setup->clockPhase];
        setupRemap.dataBits = (DRV_SPI_DATA_BITS)dObj->remapDataBits[setup->dataBits];

        if ((setupRemap.clockPhase != DRV_SPI_CLOCK_PHASE_INVALID) && (setupRemap.clockPolarity != DRV_SPI_CLOCK_POLARITY_INVALID) 
                && (setupRemap.dataBits != DRV_SPI_DATA_BITS_INVALID))
        {
            /* Save the required setup in client object which can be used while
            processing queue requests. */
            clientObj->setup = setupRemap;

            /* Update the flag denoting that setup has been changed dynamically */
            clientObj->setupChanged = true;

            isSuccess = true;
        }
    }
    return isSuccess;
}

void DRV_SPI_WriteReadTransferAdd (
    const DRV_HANDLE handle,
    void* pTransmitData,
    size_t txSize,
    void* pReceiveData,
    size_t rxSize,
    DRV_SPI_TRANSFER_HANDLE* const transferHandle
)
{
    DRV_SPI_CLIENT_OBJ* clientObj = (DRV_SPI_CLIENT_OBJ*)NULL;
    DRV_SPI_OBJ* dObj = (DRV_SPI_OBJ*)NULL;
    DRV_SPI_TRANSFER_OBJ* transferObj = (DRV_SPI_TRANSFER_OBJ*)NULL;

    if (transferHandle == NULL)
    {
        return;
    }

    *transferHandle = DRV_SPI_TRANSFER_HANDLE_INVALID;

    /* Validate the driver handle */
    clientObj = _DRV_SPI_DriverHandleValidate(handle);
    if (clientObj == NULL)
    {
        return;
    }

    if( ((txSize > 0) && (pTransmitData != NULL)) || ((rxSize > 0) && (pReceiveData != NULL)) )
    {
        dObj = (DRV_SPI_OBJ *)&gDrvSPIObj[clientObj->drvIndex];

        if(_DRV_SPI_ResourceLock(dObj) == false)
        {
            SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Failed to get resource lock");
            return;
        }

        /* Get a free transfer object */
        transferObj = _DRV_SPI_FreeTransferObjGet(clientObj);

        if (transferObj == NULL)
        {
            /* This means we could not find a buffer. This will happen if the the
             * transfer queue size parameter is configured to be less */

            SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Insufficient Queue Depth");
            _DRV_SPI_ResourceUnlock(dObj);
            return;
        }

        /* Configure the object */
        transferObj->pReceiveData   = pReceiveData;
        transferObj->pTransmitData  = pTransmitData;
        transferObj->currentState   = DRV_SPI_TRANSFER_OBJ_IS_IN_QUEUE;
        transferObj->event          = DRV_SPI_TRANSFER_EVENT_PENDING;
        transferObj->clientHandle   = handle;

        if((dObj->txDMAChannel != SYS_DMA_CHANNEL_NONE) && (dObj->rxDMAChannel != SYS_DMA_CHANNEL_NONE) && (clientObj->setup.dataBits != DRV_SPI_DATA_BITS_8))
        {
            /* If its DMA mode and SPI data bits is other than 8 bit, then divide transmit sizes by 2 */
            transferObj->txSize = txSize >> 1;
            transferObj->rxSize = rxSize >> 1;
        }
        else
        {
            transferObj->txSize = txSize;
            transferObj->rxSize = rxSize;
        }

        /* Update the unique transfer handle in output parameter.This handle can
         * be used by user to poll the status of transfer operation */
        *transferHandle = transferObj->transferHandle;

        /* Add the buffer object to the transfer buffer list */
        if (_DRV_SPI_TransferObjAddToList(dObj, transferObj) == true)
        {
            transferObj->currentState = DRV_SPI_TRANSFER_OBJ_IS_PROCESSING;

             /* This is the first request in the queue, hence initiate a transfer */
            _DRV_SPI_UpdateTransferSetupAndAssertCS(transferObj);

            if((dObj->txDMAChannel != SYS_DMA_CHANNEL_NONE) && (dObj->rxDMAChannel != SYS_DMA_CHANNEL_NONE))
            {
                _DRV_SPI_StartDMATransfer(transferObj);
            }
            else
            {
                dObj->spiPlib->writeRead(transferObj->pTransmitData, transferObj->txSize, transferObj->pReceiveData, transferObj->rxSize);
            }
        }

        _DRV_SPI_ResourceUnlock(dObj);
    }
}

void DRV_SPI_WriteTransferAdd (
    const   DRV_HANDLE  handle,
    void*   pTransmitData,
    size_t  txSize,
    DRV_SPI_TRANSFER_HANDLE* const transferHandle
)
{
    DRV_SPI_WriteReadTransferAdd(handle, pTransmitData, txSize, NULL, 0, transferHandle);
}

void DRV_SPI_ReadTransferAdd (
    const   DRV_HANDLE  handle,
    void*   pReceiveData,
    size_t  rxSize,
    DRV_SPI_TRANSFER_HANDLE* const transferHandle
)
{
    DRV_SPI_WriteReadTransferAdd(handle, NULL, 0, pReceiveData, rxSize, transferHandle);
}

DRV_SPI_TRANSFER_EVENT DRV_SPI_TransferStatusGet(const DRV_SPI_TRANSFER_HANDLE transferHandle)
{
    DRV_SPI_OBJ* dObj = NULL;
    uint32_t drvInstance = 0;
    uint8_t transferIndex;

    /* Extract driver instance value from the transfer handle */
    drvInstance = ((transferHandle & DRV_SPI_INSTANCE_MASK) >> 8);

    if(drvInstance >= DRV_SPI_INSTANCES_NUMBER)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Transfer Handle Invalid");
        return DRV_SPI_TRANSFER_EVENT_HANDLE_INVALID;
    }

    dObj = (DRV_SPI_OBJ*)&gDrvSPIObj[drvInstance];

    /* Extract transfer buffer index value from the transfer handle */
    transferIndex = transferHandle & DRV_SPI_INDEX_MASK;

    /* Validate the transferIndex and corresponding request */
    if(transferIndex >= dObj->transferObjPoolSize)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Transfer Handle Invalid");
        return DRV_SPI_TRANSFER_EVENT_HANDLE_INVALID;
    }
    else if(transferHandle != dObj->transferObjPool[transferIndex].transferHandle)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Transfer Handle Expired");
        return DRV_SPI_TRANSFER_EVENT_HANDLE_EXPIRED;
    }
    else
    {
        return dObj->transferObjPool[transferIndex].event;
    }
}
