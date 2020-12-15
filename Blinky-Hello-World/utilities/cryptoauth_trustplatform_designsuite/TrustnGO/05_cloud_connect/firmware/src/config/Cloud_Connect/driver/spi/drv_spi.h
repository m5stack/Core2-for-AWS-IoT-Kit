/*******************************************************************************
  SPI Driver Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_spi.h

  Summary:
    SPI Driver Interface Header File

  Description:
    The SPI device driver provides a simple interface to manage the SPI modules
    on Microchip microcontrollers.  This file provides the interface definition
    for the SPI driver.
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

#ifndef DRV_SPI_H
#define DRV_SPI_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "drv_spi_definitions.h"
#include "driver/driver.h"
#include "system/system.h"

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
/* SPI Driver Transfer Handle

  Summary:
    Handle identifying the transfer request queued.

  Description:
    A transfer handle value is returned by a call to the DRV_SPI_ReadTransferAdd
    or DRV_SPI_WriteTransferAdd or DRV_SPI_WriteReadTransferAdd functions. This
    handle is associated with the transfer request passed into the function and it
    allows the application to track the completion of the transfer request.
    The transfer handle value returned from the "transfer add"
    function is returned back to the client by the "event handler callback"
    function registered with the driver.

    This handle can also be used to poll the transfer completion status using
    DRV_SPI_TransferStatusGet API.

    The transfer handle assigned to a client request expires when a new transfer
    request is made after the completion of the current request.

  Remarks:
    None
*/

typedef uintptr_t DRV_SPI_TRANSFER_HANDLE;

// *****************************************************************************
/* SPI Driver Invalid Transfer Handle

  Summary:
    Definition of an invalid transfer handle.

  Description:
    This is the definition of an invalid transfer handle. An invalid transfer
    handle is returned by DRV_SPI_WriteReadTransferAdd or DRV_SPI_WriteTransferAdd
    or DRV_SPI_ReadTransferAdd function if the buffer add request was not
    successful. It can happen due to invalid arguments or lack of space in the
    queue.

  Remarks:
    None
*/

#define DRV_SPI_TRANSFER_HANDLE_INVALID /*DOM-IGNORE-BEGIN*/((DRV_SPI_TRANSFER_HANDLE)(-1))/*DOM-IGNORE-END*/

// *****************************************************************************
/* SPI Driver Transfer Events

   Summary
    Identifies the possible events that can result from a transfer add request.

   Description
    This enumeration identifies the possible events that can result from a
    transfer add request caused by the client calling either
    DRV_SPI_ReadTransferAdd or DRV_SPI_WriteTransferAdd or
    DRV_SPI_WriteReadTransferAdd functions.

   Remarks:
    Either DRV_SPI_TRANSFER_EVENT_COMPLETE or DRV_SPI_TRANSFER_EVENT_ERROR
    is passed in the "event" parameter of the event handling callback
    function that the client registered with the driver by calling the
    DRV_SPI_TransferEventHandlerSet function when a transfer request is
    completed.

    When status polling is used, any one of these events is returned by
    DRV_SPI_TransferStatusGet function.
*/

typedef enum
{
    /* Transfer request is pending */
    DRV_SPI_TRANSFER_EVENT_PENDING /*DOM-IGNORE-BEGIN*/ = 0 /*DOM-IGNORE-END*/,

    /* All data were transfered successfully. */
    DRV_SPI_TRANSFER_EVENT_COMPLETE /*DOM-IGNORE-BEGIN*/ = 1 /*DOM-IGNORE-END*/,

    /* Transfer Handle given is expired. It means transfer
    is completed but with or without error is not known.
    In case of Non-DMA transfer, since there is no possibility
    of error, it can be assumed same as DRV_SPI_TRANSFER_EVENT_COMPLETE  */
    DRV_SPI_TRANSFER_EVENT_HANDLE_EXPIRED /*DOM-IGNORE-BEGIN*/ = 2 /*DOM-IGNORE-END*/,

    /* There was an error while processing transfer request. */
    DRV_SPI_TRANSFER_EVENT_ERROR /*DOM-IGNORE-BEGIN*/ = -1 /*DOM-IGNORE-END*/,

    /* Transfer Handle given is invalid */
    DRV_SPI_TRANSFER_EVENT_HANDLE_INVALID /*DOM-IGNORE-BEGIN*/ = -2 /*DOM-IGNORE-END*/

} DRV_SPI_TRANSFER_EVENT;

// *****************************************************************************
/* SPI Driver Transfer Event Handler Function Pointer

   Summary
    Pointer to a SPI Driver Transfer Event handler function

   Description
    This data type defines the required function signature for the SPI driver
    transfer event handling callback function. A client must register a pointer
    using the transfer event handling function whose function signature (parameter
    and return value types) match the types specified by this function pointer
    in order to receive transfer related event calls back from the driver.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    event -             Identifies the type of event

    transferHandle -    Handle identifying the transfer to which the event relates

    context -           Value identifying the context of the application that
                        registered the event handling function.

  Returns:
    None.

  Example:
    <code>
    void APP_MyTransferEventHandler( DRV_SPI_TRANSFER_EVENT event,
                                   DRV_SPI_TRANSFER_HANDLE transferHandle,
                                   uintptr_t context )
    {
        MY_APP_DATA_STRUCT pAppData = (MY_APP_DATA_STRUCT) context;

        switch(event)
        {
            case DRV_SPI_TRANSFER_EVENT_COMPLETE:

                // Handle the completed transfer.
                break;

            case DRV_SPI_TRANSFER_EVENT_ERROR:

                // Handle error.
                break;
        }
    }
    </code>

  Remarks:
    - If the event is DRV_SPI_TRANSFER_EVENT_COMPLETE, it means that the data was
      transferred successfully.

    - If the event is DRV_SPI_TRANSFER_EVENT_ERROR, it means that the data was not
      transferred successfully.

    - The transferHandle parameter contains the transfer handle of the transfer
      request that is associated with the event.

    - The context parameter contains the a handle to the client context,
      provided at the time the event handling function was registered using the
      DRV_SPI_TransferEventHandlerSet function.  This context handle value is
      passed back to the client as the "context" parameter.  It can be any value
      necessary to identify the client context or instance (such as a pointer to
      the client's data) of the client that made the transfer add request.

    - The event handler function executes in interrupt context of the peripheral.
      Hence it is recommended of the application to not perform process
      intensive or blocking operations with in this function.

    - The DRV_SPI_ReadTransferAdd, DRV_SPI_WriteTransferAdd and
      DRV_SPI_WriteReadTransferAdd functions can be called in the event handler
      to add a transfer request to the driver queue. These functions can only
      be called to add transfers to the driver instance whose event handler is
      running. For example, SPI2 driver transfer requests cannot be added in SPI1
      driver event handler. Similarly, SPIx transfer requests should not be added
      in event handler of any other peripheral.
*/

typedef void ( *DRV_SPI_TRANSFER_EVENT_HANDLER )( DRV_SPI_TRANSFER_EVENT event, DRV_SPI_TRANSFER_HANDLE transferHandle, uintptr_t context );


// *****************************************************************************
// *****************************************************************************
// Section: SPI Driver System Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ DRV_SPI_Initialize
    (
        const SYS_MODULE_INDEX index,
        const SYS_MODULE_INIT * const init
    )

  Summary:
    Initializes the SPI instance for the specified driver index.

  Description:
    This routine initializes the SPI driver instance for the specified driver
    index, making it ready for clients to open and use it. The initialization
    data is specified by the init parameter. The initialization may fail if the
    number of driver objects allocated are insufficient or if the specified
    driver instance is already initialized. The driver instance index is
    independent of the SPI module ID. For example, driver instance 0 can be
    assigned to SPI2.

  Precondition:
    None.

  Parameters:
    index - Identifier for the instance to be initialized

    init  - Pointer to the init data structure containing any data necessary to
            initialize the driver.

  Returns:
    If successful, returns a valid handle to a driver instance object.
    Otherwise, returns SYS_MODULE_OBJ_INVALID.

  Example:
    <code>
    // The following code snippet shows an example SPI driver initialization.

    SYS_MODULE_OBJ   objectHandle;

    const DRV_SPI_PLIB_INTERFACE drvSPI0PlibAPI = {

        // SPI PLIB Setup
        .setup = (DRV_SPI_PLIB_SETUP)SPI0_TransferSetup,

        // SPI PLIB WriteRead function
        .writeRead = (DRV_SPI_PLIB_WRITE_READ)SPI0_WriteRead,

        // SPI PLIB Transfer Status function
        .isBusy = (DRV_SPI_PLIB_IS_BUSY)SPI0_IsBusy,

        // SPI PLIB Callback Register
        .callbackRegister = (DRV_SPI_PLIB_CALLBACK_REGISTER)SPI0_CallbackRegister,
    };

    const DRV_SPI_INIT drvSPI0InitData = {

        // SPI PLIB API
        .spiPlib = &drvSPI0PlibAPI,

        .remapDataBits = drvSPI0remapDataBits,
        .remapClockPolarity = drvSPI0remapClockPolarity,
        .remapClockPhase = drvSPI0remapClockPhase,


        /// SPI Number of clients
        .numClients = DRV_SPI_CLIENTS_NUMBER_IDX0,

        // SPI Client Objects Pool
        .clientObjPool = (uintptr_t)&drvSPI0ClientObjPool[0],

        // DMA Channel for Transmit
        .dmaChannelTransmit = DRV_SPI_XMIT_DMA_CH_IDX0,

        // DMA Channel for Receive
        .dmaChannelReceive  = DRV_SPI_RCV_DMA_CH_IDX0,

        // SPI Transmit Register
        .spiTransmitAddress =  (void *)&(SPI0_REGS->SPI_TDR),

        // SPI Receive Register
        .spiReceiveAddress  = (void *)&(SPI0_REGS->SPI_RDR),

        // Interrupt source is DMA

       .interruptSource = XDMAC_IRQn,


        // SPI Queue Size
        .queueSize = DRV_SPI_QUEUE_SIZE_IDX0,

        // SPI Transfer Objects Pool
        .transferObjPool = (uintptr_t)&drvSPI0TransferObjPool[0],
    };

    objectHandle = DRV_SPI_Initialize(DRV_SPI_INDEX_0,(SYS_MODULE_INIT*)&drvSPI0InitData);
    if (objectHandle == SYS_MODULE_OBJ_INVALID)
    {
        // Handle error
    }
    </code>

  Remarks:
    - This routine must be called before any other SPI routine is called.
    - This routine must only be called once during system initialization.
    - This routine will NEVER block for hardware access.
*/

SYS_MODULE_OBJ DRV_SPI_Initialize( const SYS_MODULE_INDEX index, const SYS_MODULE_INIT * const init );

// *****************************************************************************
/* Function:
    SYS_STATUS DRV_SPI_Status( SYS_MODULE_OBJ object )

  Summary:
    Gets the current status of the SPI driver module.

  Description:
    This routine provides the current status of the SPI driver module.

  Precondition:
    Function DRV_SPI_Initialize should have been called before calling this
    function.

  Parameters:
    object - Driver object handle, returned from the DRV_SPI_Initialize routine

  Returns:
    - SYS_STATUS_READY -  Initialization have succeeded and the SPI is
                          ready for additional operations
    - SYS_STATUS_DEINITIALIZED -  Indicates that the driver has been
                                  deinitialized

  Example:
    <code>
    SYS_MODULE_OBJ      object;     // Returned from DRV_SPI_Initialize
    SYS_STATUS          spiStatus;

    spiStatus = DRV_SPI_Status(object);
    if (spiStatus == SYS_STATUS_READY)
    {
        // This means now the driver can be opened using the
        // DRV_SPI_Open() function.
    }
    </code>

  Remarks:
    A driver can be opened only when its status is SYS_STATUS_READY.
*/

SYS_STATUS DRV_SPI_Status( SYS_MODULE_OBJ object);

// *****************************************************************************
// *****************************************************************************
// Section: SPI Driver Common Client Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    DRV_HANDLE DRV_SPI_Open
    (
        const SYS_MODULE_INDEX index,
        const DRV_IO_INTENT ioIntent
    )

  Summary:
    Opens the specified SPI driver instance and returns a handle to it.

  Description:
    This routine opens the specified SPI driver instance and provides a
    handle that must be provided to all other client-level operations to
    identify the caller and the instance of the driver. The ioIntent
    parameter defines how the client interacts with this driver instance.

    Specifying a DRV_IO_INTENT_EXCLUSIVE will cause the driver to provide
    exclusive access to this client. The driver cannot be opened by any
    other client.

  Precondition:
    Function DRV_SPI_Initialize must have been called before calling this
    function.

  Parameters:
    index  -    Identifier for the object instance to be opened

    intent -    Zero or more of the values from the enumeration DRV_IO_INTENT
                "ORed" together to indicate the intended use of the driver.
                See function description for details.

  Returns:
    If successful, the routine returns a valid open-instance handle (a number
    identifying both the caller and the module instance).

    If an error occurs, the return value is DRV_HANDLE_INVALID. Error can occur
    - if the number of client objects allocated via DRV_SPI_CLIENTS_NUMBER is
      insufficient.
    - if the client is trying to open the driver but driver has been opened
      exclusively by another client.
    - if the driver peripheral instance being opened is not initialized or is
      invalid.
    - if the client is trying to open the driver exclusively, but has already
      been opened in a non exclusive mode by another client.
    - if the driver is not ready to be opened, typically when the initialize
      routine has not completed execution.

  Example:
    <code>
    DRV_HANDLE handle;

    handle = DRV_SPI_Open(DRV_SPI_INDEX_0, DRV_IO_INTENT_EXCLUSIVE);
    if (handle == DRV_HANDLE_INVALID)
    {
        // Unable to open the driver
        // May be the driver is not initialized or the initialization
        // is not complete.
    }
    </code>

  Remarks:
    - The handle returned is valid until the DRV_SPI_Close routine is called.
    - This routine will NEVER block waiting for hardware.
*/

DRV_HANDLE DRV_SPI_Open(const SYS_MODULE_INDEX index, const DRV_IO_INTENT ioIntent);

// *****************************************************************************
/* Function:
    void DRV_SPI_Close( DRV_Handle handle )

  Summary:
    Closes an opened-instance of the SPI driver.

  Description:
    This routine closes an opened-instance of the SPI driver, invalidating the
    handle. User should make sure that there is no transfer request pending
    before calling this API. A new handle must be obtained by calling DRV_SPI_Open
    before the caller may use the driver again.

  Precondition:
    DRV_SPI_Open must have been called to obtain a valid opened device handle.

  Parameters:
    handle -    A valid open-instance handle, returned from the driver's
                open routine

  Returns:
    None.

  Example:
    <code>
    // 'handle', returned from the DRV_SPI_Open

    DRV_SPI_Close(handle);

    </code>

  Remarks:
    None.
*/

void DRV_SPI_Close( const DRV_HANDLE handle);

// *****************************************************************************
/*
  Function:
    bool DRV_SPI_TransferSetup ( DRV_HANDLE handle, DRV_SPI_TRANSFER_SETUP * setup )

  Summary:
    Sets the dynamic configuration of the driver including chip select pin.

  Description:
    This function is used to update any of the DRV_SPI_TRANSFER_SETUP
    parameters for the selected client of the driver dynamically. For single
    client scenario, if GPIO has to be used for chip select, then calling this
    API with appropriate GPIO pin information becomes mandatory. For multi
    client scenario where different clients need different setup like baud rate,
    clock settings, chip select etc, then also calling this API is mandatory.

    Note that all the elements of setup structure must be filled appropriately
    before using this API.

  Preconditions:
    DRV_SPI_Open must have been called to obtain a valid opened device handle.

  Parameters:
    handle      - A valid open-instance handle, returned from the driver's
                   open routine
    *setup       - A structure containing the new configuration settings

  Returns:
    None.

  Example:
    <code>
        // mySPIHandle is the handle returned by the DRV_SPI_Open function.
        DRV_SPI_TRANSFER_SETUP setup;

        setup.baudRateInHz = 10000000;
        setup.clockPhase = DRV_SPI_CLOCK_PHASE_TRAILING_EDGE;
        setup.clockPolarity = DRV_SPI_CLOCK_POLARITY_IDLE_LOW;
        setup.dataBits = DRV_SPI_DATA_BITS_16;
        setup.chipSelect = SYS_PORT_PIN_PC5;
        setup.csPolarity = DRV_SPI_CS_POLARITY_ACTIVE_LOW;

        DRV_SPI_TransferSetup ( mySPIHandle, &setup );
    </code>

  Remarks:
    None.

*/
bool DRV_SPI_TransferSetup ( DRV_HANDLE handle, DRV_SPI_TRANSFER_SETUP * setup );

// *****************************************************************************
// *****************************************************************************
// Section: SPI Driver Transfer Queuing Model Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void DRV_SPI_WriteReadTransferAdd
    (
        const DRV_HANDLE handle,
        void*       pTransmitData,
        size_t      txSize,
        void*       pReceiveData,
        size_t      rxSize,
        DRV_SPI_TRANSFER_HANDLE * const transferHandle
    );

  Summary:
    Queues a write-read transfer operation.

  Description:
    This function schedules a non-blocking write-read operation. The function
    returns with a valid transfer handle in the transferHandle argument if
    the request was scheduled successfully. The function adds the request to
    the instance specific software queue and returns immediately. While the
    request is in the queue, the application buffer is owned by the driver
    and should not be modified.
    This API will write txSize and at the same time counting of rxSize to be
    read will start. If user wants 'n' bytes to be read after txSize has been
    written, then he should keep rxSize value as 'txSize + n'.

    The function returns DRV_SPI_TRANSFER_HANDLE_INVALID in the
    transferHandle argument:
    - if neither of the transmit or receive arguments are valid.
    - if the transfer handle is NULL.
    - if the queue size is full or queue depth is insufficient.
    - if the driver handle is invalid.

    If the requesting client registered an event callback with the driver, the
    driver will issue a DRV_SPI_TRANSFER_EVENT_COMPLETE event if the transfer
    was processed successfully or DRV_SPI_TRANSFER_EVENT_ERROR event if the
    transfer was not processed successfully.

  Precondition:
    - DRV_SPI_Open must have been called to obtain a valid opened device handle.
    - DRV_SPI_TransferSetup must have been called if GPIO pin has to be used for
    chip select or any of the setup parameters has to be changed dynamically.

  Parameters:
    handle -    Handle of the communication channel as returned by the
                DRV_SPI_Open function.

    *pTransmitData- Pointer to the data which has to be transmitted. if it is
                    NULL, that means only data receiving is expected. For 9
                    to 15bit mode, data should be right aligned in the 16 bit
                    memory location.
    txSize-         Number of bytes to be transmitted. Always, size should be
                    given in terms of bytes. For example, if 10 15-bit data
                    are to be transmitted, the transmit size should be 20 bytes.
    *pReceiveData-  Pointer to the location where received data has to be stored.
                    It is user's responsibility to ensure pointed location has
                    sufficient memory to store the read data.
                    if it is NULL, that means only data transmission is expected.
                    For 9 to 15bit mode, received data will be right aligned in
                    the 16 bit memory location.
    rxSize-         Number of bytes to be received. Always, size should be
                    given in terms of bytes. For example, if 10 15-bit data
                    are to be received, the receive size should be 20 bytes.
                    If "n" number of bytes has to be received AFTER transmitting
                    "m" number of bytes, then "txSize" should be set as "m" and
                    "rxSize" should be set as "m+n".

    transferHandle - Handle which is returned by transfer add function.

  Returns:
    None.

  Example:
    <code>

    MY_APP_OBJ myAppObj;
    uint8_t myTxBuffer[MY_TX_BUFFER_SIZE];
    uint8_t myRxBuffer[MY_RX_BUFFER_SIZE];
    DRV_SPI_TRANSFER_HANDLE transferHandle;

    // mySPIHandle is the handle returned by the DRV_SPI_Open function.

    DRV_SPI_WriteReadTransferAdd(mySPIhandle, myTxBuffer, MY_TX_BUFFER_SIZE,
                                    myRxBuffer, MY_RX_BUFFER_SIZE, &transferHandle);

    if(transferHandle == DRV_SPI_TRANSFER_HANDLE_INVALID)
    {
        // Error handling here
    }

    // Event is received when the transfer is processed.
    </code>

  Remarks:
    - This function can be called from within the SPI Driver Transfer Event
      Handler that is registered by the client.
    - It should not be called in the event handler associated with another SPI
      driver instance or event handler of any other peripheral.
    - It should not be called directly in any ISR.
*/

void DRV_SPI_WriteReadTransferAdd(
    const   DRV_HANDLE  handle,
    void*   pTransmitData,
    size_t  txSize,
    void*   pReceiveData,
    size_t  rxSize,
    DRV_SPI_TRANSFER_HANDLE * const transferHandle);

// *****************************************************************************
/* Function:
    void DRV_SPI_WriteTransferAdd
    (
        const DRV_HANDLE handle,
        void*       pTransmitData,
        size_t      txSize,
        DRV_SPI_TRANSFER_HANDLE * const transferHandle
    );

  Summary:
    Queues a write operation.

  Description:
    This function schedules a non-blocking write operation. The function
    returns with a valid transfer handle in the transferHandle argument if
    the request was scheduled successfully. The function adds the request to
    the instance specific software queue and returns immediately. While the
    request is in the queue, the application buffer is owned by the driver
    and should not be modified.
    This API will write txSize bytes of data and the dummy data received will
    be ignored.

    The function returns DRV_SPI_TRANSFER_HANDLE_INVALID in the
    transferHandle argument:
    - if pTransmitData is NULL.
    - if txSize is zero.
    - if the transfer handle is NULL.
    - if the queue size is full or queue depth is insufficient.
    - if the driver handle is invalid.

    If the requesting client registered an event callback with the driver, the
    driver will issue a DRV_SPI_TRANSFER_EVENT_COMPLETE event if the transfer
    was processed successfully or DRV_SPI_TRANSFER_EVENT_ERROR event if the
    transfer was not processed successfully.

  Precondition:
    - DRV_SPI_Open must have been called to obtain a valid opened device handle.
    - DRV_SPI_TransferSetup must have been called if GPIO pin has to be used for
      chip select or any of the setup parameters has to be changed dynamically.

  Parameters:
    handle -    Handle of the communication channel as returned by the
                DRV_SPI_Open function.

    *pTransmitData- Pointer to the data which has to be transmitted. For 9
                    to 15bit mode, data should be right aligned in the 16 bit
                    memory location.
    txSize-         Number of bytes to be transmitted. Always, size should be
                    given in terms of bytes. For example, if 10 15-bit data
                    are to be transmitted, the transmit size should be 20 bytes.
    transferHandle - Handle which is returned by transfer add function.

  Returns:
    None.

  Example:
    <code>

    MY_APP_OBJ myAppObj;
    uint8_t myTxBuffer[MY_TX_BUFFER_SIZE];
    DRV_SPI_TRANSFER_HANDLE transferHandle;

    // mySPIHandle is the handle returned by the DRV_SPI_Open function.

    DRV_SPI_WriteTransferAdd(mySPIhandle, myTxBuffer, MY_TX_BUFFER_SIZE, &transferHandle);

    if(transferHandle == DRV_SPI_TRANSFER_HANDLE_INVALID)
    {
        // Error handling here
    }

    // Event is received when the transfer is processed.
    </code>

  Remarks:
    - This function can be called from within the SPI Driver Transfer Event
      Handler that is registered by the client.
    - It should NOT be called in the event handler associated with another SPI
      driver instance or event handler of any other peripheral.
    - It should not be called directly in any ISR.
*/

void DRV_SPI_WriteTransferAdd(
    const   DRV_HANDLE  handle,
    void*   pTransmitData,
    size_t  txSize,
    DRV_SPI_TRANSFER_HANDLE * const transferHandle);

// *****************************************************************************
/* Function:
    void DRV_SPI_ReadTransferAdd
    (
        const DRV_HANDLE handle,
        void*       pReceiveData,
        size_t      rxSize,
        DRV_SPI_TRANSFER_HANDLE * const transferHandle
    );

  Summary:
    Queues a read operation.

  Description:
    This function schedules a non-blocking read operation. The function
    returns with a valid transfer handle in the transferHandle argument if
    the request was scheduled successfully. The function adds the request to
    the instance specific software queue and returns immediately. While the
    request is in the queue, the application buffer is owned by the driver
    and should not be modified.
    This API will write rxSize bytes of dummy data and  will read rxSize bytes
    of data in the memory location pointed by pReceiveData.

    The function returns DRV_SPI_TRANSFER_HANDLE_INVALID in the
    transferHandle argument:
    - if pReceiveData is NULL.
    - if rxSize is zero.
    - if the transfer handle is NULL.
    - if the queue size is full or queue depth is insufficient.
    - if the driver handle is invalid.

    If the requesting client registered an event callback with the driver, the
    driver will issue a DRV_SPI_TRANSFER_EVENT_COMPLETE event if the transfer
    was processed successfully or DRV_SPI_TRANSFER_EVENT_ERROR event if the
    transfer was not processed successfully.

  Precondition:
    - DRV_SPI_Open must have been called to obtain a valid opened device handle.
    - DRV_SPI_TransferSetup must have been called if GPIO pin has to be used for
      chip select or any of the setup parameters has to be changed dynamically.

  Parameters:
    handle -    Handle of the communication channel as returned by the
                DRV_SPI_Open function.
    *pReceiveData-  Pointer to the location where received data has to be stored.
                    It is user's responsibility to ensure pointed location has
                    sufficient memory to store the read data.
                    For 9 to 15bit mode, received data will be right aligned in
                    the 16 bit memory location.
    rxSize-         Number of bytes to be received. Always, size should be
                    given in terms of bytes. For example, if 10 15-bit data
                    are to be received, the receive size should be 20 bytes.

    transferHandle - Handle which is returned by transfer add function.

  Returns:
    None.

  Example:
    <code>

    MY_APP_OBJ myAppObj;
    uint8_t myRxBuffer[MY_RX_BUFFER_SIZE];
    DRV_SPI_TRANSFER_HANDLE transferHandle;

    // mySPIHandle is the handle returned by the DRV_SPI_Open function.

    DRV_SPI_ReadTransferAdd(mySPIhandle, myRxBuffer, MY_RX_BUFFER_SIZE, &transferHandle);

    if(transferHandle == DRV_SPI_TRANSFER_HANDLE_INVALID)
    {
        // Error handling here
    }

    // Event is received when the transfer is processed.
    </code>

  Remarks:
    - This function can be called from within the SPI Driver Transfer Event
      Handler that is registered by the client.
    - It should not be called in the event handler associated with another SPI
      driver instance or event handler of any other peripheral.
    - It should not be called directly in any ISR.
*/

void DRV_SPI_ReadTransferAdd(
    const   DRV_HANDLE  handle,
    void*   pReceiveData,
    size_t  rxSize,
    DRV_SPI_TRANSFER_HANDLE * const transferHandle);

// *****************************************************************************
/* Function:
    void DRV_SPI_TransferEventHandlerSet
    (
        const DRV_HANDLE handle,
        const DRV_SPI_TRANSFER_EVENT_HANDLER eventHandler,
        const uintptr_t context
    )

  Summary:
    Allows a client to set a transfer event handling function for the driver
    to call back when queued transfer has finished.

  Description:
    This function allows a client to register a transfer event handling function
    with the driver to call back when queued transfer has finished.
    When a client calls either the DRV_SPI_ReadTransferAdd or
    DRV_SPI_WriteTransferAdd or DRV_SPI_WriteReadTransferAdd function, it is
    provided with a handle identifying the transfer request that was added to the
    driver's queue.  The driver will pass this handle back to the
    client by calling "eventHandler" function when the transfer has
    completed.

    The event handler should be set before the client performs any "transfer add"
    operations that could generate events. The event handler once set, persists
    until the client closes the driver or sets another event handler (which
    could be a "NULL" pointer to indicate no callback).

  Precondition:
    DRV_SPI_Open must have been called to obtain a valid open instance handle.

  Parameters:
    handle -    A valid open-instance handle, returned from the driver's open
                routine.

    eventHandler - Pointer to the event handler function.

    context -   The value of parameter will be passed back to the client
                unchanged, when the eventHandler function is called. It can be
                used to identify any client specific data object that
                identifies the instance of the client module (for example, it
                may be a pointer to the client module's state structure).

  Returns:
    None.

  Example:
    <code>
    // myAppObj is an application specific state data object.
    MY_APP_OBJ myAppObj;

    uint8_t myTxBuffer[MY_TX_BUFFER_SIZE];
    uint8_t myRxBuffer[MY_RX_BUFFER_SIZE];
    DRV_SPI_TRANSFER_HANDLE transferHandle;

    // mySPIHandle is the handle returned by the DRV_SPI_Open function.

    // Client registers an event handler with driver. This is done once

    DRV_SPI_TransferEventHandlerSet( mySPIHandle, APP_SPITransferEventHandler,
                                     (uintptr_t)&myAppObj );

    DRV_SPI_WriteReadTransferAdd(mySPIhandle, myTxBuffer,
                                MY_TX_BUFFER_SIZE, myRxBuffer,
                                MY_RX_BUFFER_SIZE, &transferHandle);

    if(transferHandle == DRV_SPI_TRANSFER_HANDLE_INVALID)
    {
        // Error handling here
    }

    // Event is received when the transfer is completed.

    void APP_SPITransferEventHandler(DRV_SPI_TRANSFER_EVENT event,
            DRV_SPI_TRANSFER_HANDLE handle, uintptr_t context)
    {
        // The context handle was set to an application specific
        // object. It is now retrievable easily in the event handler.
        MY_APP_OBJ myAppObj = (MY_APP_OBJ *) context;

        switch(event)
        {
            case DRV_SPI_TRANSFER_EVENT_COMPLETE:
                // This means the data was transferred.
                break;

            case DRV_SPI_TRANSFER_EVENT_ERROR:
                // Error handling here.
                break;

            default:
                break;
        }
    }
    </code>

  Remarks:
    If the client does not want to be notified when the queued transfer request
    has completed, it does not need to register a callback.
*/

void DRV_SPI_TransferEventHandlerSet( const DRV_HANDLE handle, const DRV_SPI_TRANSFER_EVENT_HANDLER eventHandler, uintptr_t context );

// *****************************************************************************
/* Function:
    DRV_SPI_TRANSFER_EVENT DRV_SPI_TransferStatusGet(const DRV_SPI_TRANSFER_HANDLE transferHandle)

  Summary:
    Returns transfer add request status.

  Description:
    This function can be used to poll the status of the queued transfer request
    if the application doesn't prefer to use the event handler (callback)
    function to get notified.

  Precondition:
    Either DRV_SPI_ReadTransferAdd or DRV_SPI_WriteTransferAdd or
    DRV_SPI_WriteReadTransferAdd function must have been called and a valid
    transfer handle must have been returned.

  Parameters:
    transferHandle - Handle of the transfer request of which status has to be
                     obtained.

  Returns:
    One of the elements of the enum "DRV_SPI_TRANSFER_EVENT".

  Example:
  <code>
    // myAppObj is an application specific object.
    MY_APP_OBJ myAppObj;

    uint8_t mybuffer[MY_BUFFER_SIZE];
    DRV_SPI_TRANSFER_HANDLE transferHandle;
    DRV_SPI_TRANSFER_EVENT event;

    // mySPIHandle is the handle returned by the DRV_SPI_Open function.

    DRV_SPI_ReadTransferAdd( mySPIhandle, myBuffer, MY_RECEIVE_SIZE, &transferHandle);

    if(transferHandle == DRV_SPI_TRANSFER_HANDLE_INVALID)
    {
        // Error handling here
    }

    //Check the status of the transfer request
    //This call can be used to wait until the transfer is completed.
    event  = DRV_SPI_TransferStatusGet(transferHandle);
  </code>

  Remarks:
    None.
*/

DRV_SPI_TRANSFER_EVENT DRV_SPI_TransferStatusGet(const DRV_SPI_TRANSFER_HANDLE transferHandle );

// *****************************************************************************
// *****************************************************************************
// Section: SPI Driver Synchronous(Blocking Model) Transfer Interface Routines
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
/* Function:
    void DRV_SPI_WriteTransfer
    (
        const DRV_HANDLE handle,
        void*       pTransmitData,
        size_t      txSize
    );

  Summary:
    This is a blocking function that transmits data over SPI.

  Description:
    This function does a blocking write operation. The function blocks till
    the data transmit is complete.
    Function will return true if the transmit is successful or false in case of an error.
    The failure will occur for the following reasons:
    - if the handle is invalid
    - if the pointer to the transmit buffer is NULL
    - if the transmit size is 0

  Precondition:
    - DRV_SPI_Open must have been called to obtain a valid opened device handle.
    - DRV_SPI_TransferSetup must have been called if GPIO pin has to be used for
      chip select or any of the setup parameters has to be changed dynamically.

  Parameters:
    handle -    Handle of the communication channel as returned by the
                DRV_SPI_Open function.

    *pTransmitData- Pointer to the data which has to be transmitted. For 9
                    to 15bit mode, data should be right aligned in the 16 bit
                    memory location.

    txSize-         Number of bytes to be transmitted. Always, size should be
                    given in terms of bytes. For example, if 10 15-bit data
                    are to be transmitted, the transmit size should be 20 bytes.

  Returns:
    - true - transfer is successful
    - false - error has occurred

  Example:
    <code>

    MY_APP_OBJ myAppObj;
    uint8_t myTxBuffer[MY_TX_BUFFER_SIZE];

    // mySPIHandle is the handle returned by the DRV_SPI_Open function.

    if (DRV_SPI_WriteTransfer(mySPIhandle, myTxBuffer, MY_TX_BUFFER_SIZE) == false)
    {
        // Handle error here
    }
    </code>

  Remarks:
    - This function is thread safe in a RTOS application.
    - This function should not be called from an interrupt context.
*/
bool DRV_SPI_WriteTransfer(const DRV_HANDLE handle, void* pTransmitData,  size_t txSize );

// *****************************************************************************
/* Function:
    void DRV_SPI_ReadTransfer
    (
        const DRV_HANDLE handle,
        void*       pReceiveData,
        size_t      rxSize
    );

  Summary:
    This is a blocking function that receives data over SPI.

  Description:
    This function does a blocking read operation. The function blocks till
    the data receive is complete.
    Function will return true if the receive is successful or false in case of an error.
    The failure will occur for the following reasons:
    - if the handle is invalid
    - if the pointer to the receive buffer is NULL
    - if the receive size is 0

  Precondition:
    - DRV_SPI_Open must have been called to obtain a valid opened device handle.
    - DRV_SPI_TransferSetup must have been called if GPIO pin has to be used for
      chip select or any of the setup parameters has to be changed dynamically.

  Parameters:
    handle -    Handle of the communication channel as returned by the
                DRV_SPI_Open function.

    *pReceiveData-  Pointer to the buffer where the data is to be received. For
                    9 to 15bit mode, data should be right aligned in the 16 bit
                    memory location.

    rxSize-         Number of bytes to be received. Always, size should be
                    given in terms of bytes. For example, if 10 15-bit data
                    are to be received, the receive size should be 20 bytes.

  Returns:
    - true - receive is successful
    - false - error has occurred

  Example:
    <code>

    MY_APP_OBJ myAppObj;
    uint8_t myRxBuffer[MY_RX_BUFFER_SIZE];

    // mySPIHandle is the handle returned by the DRV_SPI_Open function.

    if (DRV_SPI_ReadTransfer(mySPIhandle, myRxBuffer, MY_RX_BUFFER_SIZE) == false)
    {
        // Handle error here
    }
    </code>

  Remarks:
    - This function is thread safe in a RTOS application.
    - This function should not be called from an interrupt context.
*/
bool DRV_SPI_ReadTransfer(const DRV_HANDLE handle, void* pReceiveData,  size_t rxSize );

// *****************************************************************************
/* Function:
    void DRV_SPI_WriteReadTransfer
    (
        const DRV_HANDLE handle,
        void*       pTransmitData,
        size_t      txSize,
        void*       pReceiveData,
        size_t      rxSize
    );

  Summary:
    This is a blocking function that transmits and receives data over SPI.

  Description:
    This function does a blocking write-read operation. The function blocks till
    the data receive is complete.
    Function will return true if the receive is successful or false in case of an error.
    The failure will occur for the following reasons:
    - if the handle is invalid
    - if the transmit size is non-zero and pointer to the transmit buffer is NULL
    - if the receive size is non-zero and pointer to the receive buffer is NULL

  Precondition:
    - DRV_SPI_Open must have been called to obtain a valid opened device handle.
    - DRV_SPI_TransferSetup must have been called if GPIO pin has to be used for
      chip select or any of the setup parameters has to be changed dynamically.

  Parameters:
    handle -    Handle of the communication channel as returned by the
                DRV_SPI_Open function.
    *pTransmitData- Pointer to the data which has to be transmitted. For 9
                    to 15bit mode, data should be right aligned in the 16 bit
                    memory location.

    txSize-         Number of bytes to be transmitted. Always, size should be
                    given in terms of bytes. For example, if 10 15-bit data
                    are to be transmitted, the transmit size should be 20 bytes.

    *pReceiveData-  Pointer to the buffer where the data is to be received. For 9
                    to 15bit mode, data should be right aligned in the 16 bit
                    memory location.

    rxSize-         Number of bytes to be received. Always, size should be
                    given in terms of bytes. For example, if 10 15-bit data
                    are to be received, the receive size should be 20 bytes.
                    If "n" number of bytes has to be received AFTER transmitting
                    "m" number of bytes, then "txSize" should be set as "m" and
                    "rxSize" should be set as "m+n".

  Returns:
    - true - write-read is successful
    - false - error has occurred

  Example:
    <code>

    MY_APP_OBJ myAppObj;
    uint8_t myTxBuffer[MY_TX_BUFFER_SIZE];
    uint8_t myRxBuffer[MY_RX_BUFFER_SIZE];

    // mySPIHandle is the handle returned by the DRV_SPI_Open function.

    if (DRV_SPI_WriteReadTransfer(mySPIhandle, myTxBuffer, MY_TX_BUFFER_SIZE,
                                    myRxBuffer, MY_RX_BUFFER_SIZE) == false)
    {
        // Handle error here
    }

    </code>

  Remarks:
    - This function is thread safe in a RTOS application.
    - This function should not be called from an interrupt context.
*/
bool DRV_SPI_WriteReadTransfer(
    const DRV_HANDLE handle,
    void* pTransmitData,
    size_t txSize,
    void* pReceiveData,
    size_t rxSize);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#include "driver/spi/src/drv_spi_local.h"

#endif // #ifndef DRV_SPI_H
