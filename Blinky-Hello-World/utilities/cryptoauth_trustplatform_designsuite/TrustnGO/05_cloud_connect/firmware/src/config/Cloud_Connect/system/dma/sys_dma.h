/*******************************************************************************
  DMA System Service Library Interface Header File

  Company
    Microchip Technology Inc.

  File Name
    sys_dma.h

  Summary
    DMA system service library interface.

  Description
    This file defines the interface to the DMA system service library.
    This library provides access to and control of the DMA controller.

  Remarks:
    DMA controller initialize will be done from within the MCC.

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

#ifndef SYS_DMA_H    // Guards against multiple inclusion
#define SYS_DMA_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

/*  This section lists the other files that are included in this file.
*/
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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

// *****************************************************************************
/* DMA Channels

  Summary:
    This lists the set of channels available for data transfer using DMA.

  Description:
    This lists the set of channels available for data transfer using DMA.

  Remarks:
    None.
*/

typedef enum
{
    SYS_DMA_CHANNEL_0,
    SYS_DMA_CHANNEL_1,
    SYS_DMA_CHANNEL_2,
    SYS_DMA_CHANNEL_3,
    SYS_DMA_CHANNEL_4,
    SYS_DMA_CHANNEL_5,
    SYS_DMA_CHANNEL_6,
    SYS_DMA_CHANNEL_7,
    SYS_DMA_CHANNEL_8,
    SYS_DMA_CHANNEL_9,
    SYS_DMA_CHANNEL_10,
    SYS_DMA_CHANNEL_11,
    SYS_DMA_CHANNEL_NONE = -1

} SYS_DMA_CHANNEL;

// *****************************************************************************
/* DMA Transfer Events

   Summary:
    Enumeration of possible DMA transfer events.

   Description:
    This data type provides an enumeration of all possible DMA transfer
    events.

   Remarks:
    None.

*/
typedef enum
{
    /* Data was transferred successfully. */
    SYS_DMA_TRANSFER_COMPLETE,

    /* Error while processing the request */
    SYS_DMA_TRANSFER_ERROR

} SYS_DMA_TRANSFER_EVENT;

// *****************************************************************************
/* DMA Source addressing modes

   Summary:
    Enumeration of possible DMA source addressing modes.

   Description:
    This data type provides an enumeration of all possible DMA source
    addressing modes.

   Remarks:
    None.
*/
typedef enum
{
    /* Source address is always fixed */
    SYS_DMA_SOURCE_ADDRESSING_MODE_FIXED = 0x0,

    /* Source address is incremented after every transfer */
    SYS_DMA_SOURCE_ADDRESSING_MODE_INCREMENTED = 0x400,

    SYS_DMA_SOURCE_ADDRESSING_MODE_NONE = -1

} SYS_DMA_SOURCE_ADDRESSING_MODE;

// *****************************************************************************
/* DMA destination addressing modes

   Summary:
    Enumeration of possible DMA destination addressing modes.

   Description:
    This data type provides an enumeration of all possible DMA destination
    addressing modes.

   Remarks:
    None.
*/
typedef enum
{
    /* Destination address is always fixed */
    SYS_DMA_DESTINATION_ADDRESSING_MODE_FIXED = 0x0,

    /* Destination address is incremented after every transfer */
    SYS_DMA_DESTINATION_ADDRESSING_MODE_INCREMENTED = 0x800,

    SYS_DMA_DESTINATION_ADDRESSING_MODE_NONE = -1

} SYS_DMA_DESTINATION_ADDRESSING_MODE;

// *****************************************************************************
/* DMA data width

   Summary:
    Enumeration of possible DMA data width

   Description:
    This data type provides an enumeration of all possible DMA data width.

   Remarks:
    None.
*/
typedef enum
{
    /* DMA data width 8 bit */
    SYS_DMA_WIDTH_8_BIT = 0x0,

    /* DMA data width 16 bit */
    SYS_DMA_WIDTH_16_BIT = 0x100,

    /* DMA data width 32 bit */
    SYS_DMA_WIDTH_32_BIT =  0x200,

    SYS_DMA_WIDTH_NONE = -1

} SYS_DMA_WIDTH;

// *****************************************************************************
/* DMA Transfer Event Handler Function

   Summary:
    Pointer to a DMA Transfer Event handler function.

   Description:
    This data type defines a DMA Transfer Event Handler Function.

    A DMA client must register a transfer event handler function of this
    type to receive transfer related events from the DMA System Service.

    If the event is SYS_DMA_TRANSFER_COMPLETE, this means that the data
    was transferred successfully.

    If the event is SYS_DMA_TRANSFER_ERROR, this means that the data was
    not transferred successfully.

    The contextHandle parameter contains the context handle that was provided by
    the client at the time of registering the event handler. This context handle
    can be anything that the client consider helpful or necessary to identify
    the client context object associated with the channel of the DMA that
    generated the event.

    The event handler function executes in an interrupt context of DMA.
    It is recommended to the application not to perform process intensive
    operations with in this function.

   Remarks:
    None.

*/
typedef void (*SYS_DMA_CHANNEL_CALLBACK) (SYS_DMA_TRANSFER_EVENT event, uintptr_t contextHandle);


// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************
/* The following functions make up the methods (set of possible operations) of
   this interface.
*/

//******************************************************************************
/*
  Function:
    void SYS_DMA_ChannelCallbackRegister
    (
        SYS_DMA_CHANNEL channel,
        const SYS_DMA_CHANNEL_CALLBACK eventHandler,
        const uintptr_t contextHandle
    )

  Summary:
    This function allows a DMA client to set an event handler.

  Description:
    This function allows a client to set an event handler. The client may want
    to receive transfer related events in cases when it submits a DMA
    transfer request. The event handler should be set before the client
    intends to perform operations that could generate events.

    This function accepts a contextHandle parameter. This parameter could be
    set by the client to contain (or point to) any client specific data object
    that should be associated with this DMA channel.

  Precondition:
    DMA Controller should have been initialized.

  Parameters:
    channel - A specific DMA channel from which the events are expected.

    eventHandler - Pointer to the event handler function.

    contextHandle - Value identifying the context of the
    application/driver/middleware that registered the event handling function.

  Returns:
    None.

  Example:
    <code>
    MY_APP_OBJ myAppObj;

    void APP_DMA_TransferEventHandler(SYS_DMA_TRANSFER_EVENT event,
            uintptr_t contextHandle)
    {
        switch(event)
        {
            case SYS_DMA_TRANSFER_COMPLETE:
                // This means the data was transferred.
                break;

            case SYS_DMA_TRANSFER_ERROR:
                // Error handling here.
                break;

            default:
                break;
        }
    }

    // User registers an event handler with DMA channel. This is done once.
    SYS_DMA_ChannelCallbackRegister(channel, APP_DMA_TransferEventHandler,
            (uintptr_t)&myAppObj);
    </code>

  Remarks:
    None.
 */

void SYS_DMA_ChannelCallbackRegister (SYS_DMA_CHANNEL channel, const SYS_DMA_CHANNEL_CALLBACK eventHandler, const uintptr_t contextHandle);


//******************************************************************************
/* Function:
    bool SYS_DMA_ChannelTransfer
    (
        SYS_DMA_CHANNEL channel,
        const void *srcAddr,
        const void *destAddr,
        size_t blockSize
    )

  Summary:
    Adds a data transfer to a DMA channel and enables the channel to start
    data transfer.

  Description:
    This function adds a single block data transfer characteristics for a
    specific XDMAC channel id it is not busy already. It also enables the
    channel to start data transfer.

    If the requesting client registered an event callback with the PLIB,
    the PLIB will issue a SYS_DMA_TRANSFER_COMPLETE event if the transfer was
    processed successfully and SYS_DMA_TRANSFER_ERROR event if the transfer was
    not processed successfully.

  Precondition:
    DMA Controller should have been initialized.

  Parameters:
    channel - A specific DMA channel

    srcAddr - Source of the DMA transfer

    destAddr - Destination of the DMA transfer

    blockSize - Size of the transfer block

  Returns:
    True - If transfer request is accepted.
    False - If previous transfer is in progress and the request is rejected.

  Example:
    <code>
    // Transfer 10 bytes of data to UART TX using DMA channel 1
    MY_APP_OBJ myAppObj;
    uint8_t buf[10] = {0,1,2,3,4,5,6,7,8,9};
    void *srcAddr = (uint8_t *) buf;
    void *destAddr = (uin8_t*) &U1TXREG;
    size_t size = 10;

    // User registers an event handler with PLIB. This is done once.
    SYS_DMA_ChannelCallbackRegister(APP_DMA_TransferEventHandler,
        (uintptr_t)&myAppObj);

    if (SYS_DMA_ChannelTransfer(SYS_DMA_CHANNEL_1, srcAddr, destAddr, size) == true)
    {
        // do something else
    }
    else
    {
        // try again?
    }
    </code>

  Remarks:
    When DMA transfer buffers are placed in cache-able memory, cache maintenance
    operation must be performed by cleaning and invalidating cache for DMA
    buffers located in cache-able SRAM region using CMSIS APIs. The buffer start
    address must be aligned to cache line and buffer size must be multiple of
    cache line. Refer to device documentation to find the cache line size.

    Invalidate cache lines having received buffer before using it to load the
    latest data in the actual memory to the cache
    SCB_InvalidateDCache_by_Addr((uint32_t *)&readBuffer, sizeof(readBuffer));

    Clean cache lines having source buffer before submitting a transfer request
    to XDMAC to load the latest data in the cache to the actual memory
    SCB_CleanDCache_by_Addr((uint32_t *)&writeBuffer, sizeof(writeBuffer));
*/

bool SYS_DMA_ChannelTransfer (SYS_DMA_CHANNEL channel, const void *srcAddr, const void *destAddr, size_t blockSize);


//******************************************************************************
/* Function:
    bool SYS_DMA_ChannelIsBusy (SYS_DMA_CHANNEL channel)

  Summary:
    Returns the busy status of a specific DMA Channel.

  Description:
    This function returns the busy status of the DMA channel.
    DMA channel will be busy if any transfer is in progress.

    This function can be used to check the status of the channel prior to
    submitting a transfer request. And this can also be used to check the status
    of the submitted request if callback mechanism is not preferred.

  Precondition:
    DMA Controller should have been initialized.

  Parameters:
    channel - A specific DMA channel

  Returns:
    Busy status of the specific channel.
    True - Channel is busy
    False - Channel is free

  Example:
    <code>
    // Transfer 10 bytes of data to UART TX using DMA channel 1
    MY_APP_OBJ myAppObj;
    uint8_t buf[10] = {0,1,2,3,4,5,6,7,8,9};
    void *srcAddr = (uint8_t *) buf;
    void *destAddr = (uin8_t*) &U1TXREG;
    size_t size = 10;

    if(false == SYS_DMA_ChannelIsBusy(SYS_DMA_CHANNEL_1))
    {
        SYS_DMA_ChannelTransfer(SYS_DMA_CHANNEL_1, srcAddr, destAddr, size);
    }
    </code>

  Remarks:
    None.
*/

bool SYS_DMA_ChannelIsBusy (SYS_DMA_CHANNEL channel);


//******************************************************************************
/* Function:
    void SYS_DMA_ChannelDisable (SYS_DMA_CHANNEL channel)

  Summary:
    Disables the specified channel.

  Description:
    This function disables the specified channel and resets it.
    Ongoing transfer will be aborted.

  Precondition:
    DMA Controller should have been initialized.

  Parameters:
    channel - A specific DMA channel

  Returns:
    None.

  Example:
    <code>
    SYS_DMA_ChannelDisable(SYS_DMA_CHANNEL_1);
    </code>

  Remarks:
    None.
*/

void SYS_DMA_ChannelDisable (SYS_DMA_CHANNEL channel);

//******************************************************************************
/* Function:
    void SYS_DMA_AddressingModeSetup(SYS_DMA_CHANNEL channel, SYS_DMA_SOURCE_ADDRESSING_MODE sourceAddrMode, SYS_DMA_DESTINATION_ADDRESSING_MODE destAddrMode);

  Summary:
    Setup addressing mode of selected DMA channel.

  Description:
    This function sets the addressing mode of selected DMA channel.

    Any ongoing transaction of the specified XDMAC channel will be aborted when
    this function is called.

  Precondition:
    DMA Controller should have been initialized.

  Parameters:
    channel - A specific DMA channel
    sourceAddrMode -  Source addressing mode of type SYS_DMA_SOURCE_ADDRESSING_MODE
    destAddrMode - Destination addressing mode of type SYS_DMA_DESTINATION_ADDRESSING_MODE

  Returns:
    None.

  Example:
    <code>
        SYS_DMA_AddressingModeSetup(SYS_DMA_CHANNEL_1, SYS_DMA_SOURCE_ADDRESSING_MODE_FIXED, SYS_DMA_DESTINATION_ADDRESSING_MODE_FIXED);
    </code>

  Remarks:
    None.
*/
void SYS_DMA_AddressingModeSetup(SYS_DMA_CHANNEL channel, SYS_DMA_SOURCE_ADDRESSING_MODE sourceAddrMode, SYS_DMA_DESTINATION_ADDRESSING_MODE destAddrMode);

//******************************************************************************
/* Function:
    void SYS_DMA_DataWidthSetup(SYS_DMA_CHANNEL channel, SYS_DMA_WIDTH dataWidth);

  Summary:
    Setup data width of selected DMA channel.

  Description:
    This function sets data width of selected DMA channel.

    Any ongoing transaction of the specified XDMAC channel will be aborted when
    this function is called.

  Precondition:
    DMA Controller should have been initialized.

  Parameters:
    channel - A specific DMA channel
    dataWidth -  Data width of DMA transfer of type SYS_DMA_WIDTH

  Returns:
    None.

  Example:
    <code>
        SYS_DMA_DataWidthSetup(SYS_DMA_CHANNEL_1, SYS_DMA_WIDTH_16_BIT);
    </code>

  Remarks:
    None.
*/
void SYS_DMA_DataWidthSetup(SYS_DMA_CHANNEL channel, SYS_DMA_WIDTH dataWidth);

#include "sys_dma_mapping.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //SYS_DMA_H
