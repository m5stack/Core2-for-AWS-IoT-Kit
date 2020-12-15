/*******************************************************************************
  SERIAL COMMUNICATION SERIAL PERIPHERAL INTERFACE (SERCOM1_SPI ) PLIB

  Company
    Microchip Technology Inc.

  File Name
    plib_sercom1_spi_master.h

  Summary
   SERCOM1_SPI Master PLIB Header File.

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

#ifndef PLIB_SERCOM1_SPI_MASTER_H // Guards against multiple inclusion
#define PLIB_SERCOM1_SPI_MASTER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/* This section lists the other files that are included in this file.
*/

#include "plib_sercom_spi_master_common.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility

extern "C" {

#endif

// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************

/* The following functions make up the methods (set of possible operations) of
this interface.
*/

// *****************************************************************************
/* Function:
    void SERCOM1_SPI_Initialize (void);

  Summary:
    Initializes instance SERCOM1 operating in SPI mode.

  Description:
    This function initializes instance SERCOM1 operating in SPI mode.
    This function should be called before any other library function. The SERCOM
    module will be configured as per the MHC settings.

  Precondition:
    MCC GUI should be configured with the right values. The Generic Clock
    configuration and the SERCOM Peripheral Clock channel should have been
    configured in the clock manager GUI.The function will itself enable the
    required peripheral clock channel and main clock.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
        SERCOM1_SPI_Initialize();
    </code>

  Remarks:
    This function must be called once before any other SPI function is called.
*/

void SERCOM1_SPI_Initialize (void);


// *****************************************************************************
/* Function:
    bool SERCOM1_SPI_TransferSetup(SPI_TRANSFER_SETUP *setup,
                                                uint32_t spiSourceClock);

 Summary:
    Configure SERCOM SPI operational parameters at run time.

  Description:
    This function allows the application to change the SERCOM SPI operational
    parameter at run time. The application can thus override the MHC defined
    configuration for these parameters. The parameter are specified via the
    SPI_TRANSFER_SETUP type setup parameter. Each member of this parameter
    should be initialized to the desired value.

    The application may feel need to call this function in situation where
    multiple SPI slaves, each with different operation paramertes, are connected
    to one SPI master. This function can thus be used to setup the SPI Master to
    meet the communication needs of the slave.

    Calling this function will affect any ongoing communication. The application
    must thus ensure that there is no on-going communication on the SPI before
    calling this function.

  Precondition:
    SERCOM SERCOM1 SPI must first be initialized using SERCOM1_SPI_Initialize().

  Parameters :
    setup - pointer to the data structure of type SPI_TRANSFER_SETUP containing
    the operation parameters. Each operation parameter must be specified even if
    the parameter does not need to change.

    spiSourceClock - Current value of GCLK frequency feeding the SERCOM1 core.

  Returns:
    true - setup was successful.

    false - if spiSourceClock and spi clock frequencies are such that resultant
    baud value is out of the possible range.

  Example:
    <code>
        SPI_TRANSFER_SETUP setup;
        setup.clockFrequency = 1000000;
        setup.clockPhase = SPI_CLOCK_PHASE_TRAILING_EDGE;
        setup.clockPolarity = SPI_CLOCK_POLARITY_IDLE_LOW;
        setup.dataBits = SPI_DATA_BITS_8;

        // Assuming 20 MHz as peripheral Master clock frequency
        if (SERCOM1_SPI_TransferSetup (&setup, 20000000) == false)
        {
            // this means setup could not be done, debug the reason.
        }

    </code>

  Remarks:
    The application would need to call this function only if the operational
    parameter need to be different than the ones configured in MHC.
*/

bool SERCOM1_SPI_TransferSetup(SPI_TRANSFER_SETUP *setup, uint32_t spiSourceClock);


// *****************************************************************************
/* Function:
    bool SERCOM1_SPI_WriteRead (void* pTransmitData, size_t txSize
                                        void* pReceiveData, size_t rxSize);

  Summary:
    Write and Read data on SERCOM SERCOM1 SPI peripheral.

  Description:
    This function transmits "txSize" number of bytes and receives "rxSize"
    number of bytes on SERCOM SERCOM1 SPI module. Data pointed by pTransmitData is
    transmitted and received data is saved in the location pointed by
    pReceiveData. The function will transfer the maximum of "txSize" or "rxSize"
    data units towards completion.

    When "Interrupt Mode" option is unchecked in MHC, this function will be
    blocking in nature.  In this mode, the function will not return until all
    the requested data is transferred.  The function returns true after
    transferring all the data.  This indicates that the operation has been
    completed.

    When "Interrupt Mode" option is selected in MHC, the function will be
    non-blocking in nature.  The function returns immediately. The data transfer
    process continues in the peripheral interrupt.  The application specified
    transmit and receive buffer  are ownerd by the library until the data
    transfer is complete and should not be modified by the application till the
    transfer is complete.  Only one transfer is allowed at any time. The
    Application can use a callback function or a polling function to check for
    completion of the transfer. If a callback is required, this should be
    registered prior to calling the SERCOM1_SPI_WriteRead() function. The
    application can use the SERCOM1_SPI_IsBusy() to poll for completion.

  Precondition:
    The SERCOM1_SPI_Initialize function must have been called.  If the
    peripheral instance has been configured for Interrupt mode and transfer
    completion status needs to be communicated back to application via callback,
    a callback should have been registered using SERCOM1_SPI_CallbackRegister()
    function.

  Parameters:
    pTransmitData - Pointer to the data which has to be transmitted. In a case
    where only data reception is required, this pointer can be set to NULL. If
    the module is configured for 9 bit data length, the data should be right
    aligned in a 16 bit memory location. The size of this buffer should be
    txSize.

    txSize - Number of bytes to be transmitted. For 9 but data length, a count
    of 1 counts 2 bytes. This value can be different from rxSize.

    pReceiveData - Pointer to the location where the received data has to be
    stored.  It is user's responsibility to ensure that this location has
    sufficient memory to store rxSize amount of data. In a case where only data
    transmission is required, this pointer can be set to NULL.  If the module is
    configured for 9 bit data length, received data will be right aligned and
    will be stored in a 16 bit memory location.

    rxSize - Number of bytes to be received. This value can be different from
    txSize. For 9 bit data length, a size count of 1 indicates 2 bytes required
    to store 9 bits of data.

  Returns:
    true - If configured for Non-interrupt mode, the function has recevied and
    transmitted the requested number of bytes. If configured for Interrupt mode,
    the request was accepted successfully and will be processed in the
    interrupt.

    false - If both pTransmitData and pReceiveData are NULL, or if both txSize
    and rxSize are 0 or if txSize is non-zero but the pTransmitData is set to
    NULL or rxSize is non-zero but pReceiveData is NULL. In Interrupt mode, the
    function returns false if there is an on-going data transfer at the time of
    calling the function.

  Example:
    <code>

    // The following code snippet shows an example using the
    // SERCOM1_SPI_WriteRead() function in interrupt mode operation using the
    // callback function.

    uint8_t     txBuffer[4];
    uint8_t     rxBuffer[10];
    size_t      txSize = 4;
    size_t      rxSize = 10;

    void APP_SPITransferHandler(uintptr_t context)
    {
       //Transfer was completed without error, do something else now.
    }

    SERCOM1_SPI_Initialize();
    SERCOM1_SPI_CallbackRegister(&APP_SPITransferHandler, (uintptr_t)NULL);
    if(SERCOM1_SPI_WriteRead(&txBuffer, txSize, &rxBuffer, rxSize))
    {
        // request got accepted
    }
    else
    {
        // request didn't get accepted, try again later with correct arguments
    }
    // The following code snippet shows non-interrupt or blocking mode
    // operation.

    uint8_t txBuffer[4];
    uint8_t rxBuffer[10];
    size_t txSize = 4;
    size_t rxSize = 10;

    SERCOM1_SPI_Initialize();

    // This function call will block.
    SERCOM1_SPI_WriteRead(&txBuffer, txSize, &rxBuffer, rxSize);

    </code>

  Remarks:
    None.
*/

bool SERCOM1_SPI_WriteRead (void* pTransmitData, size_t txSize, void* pReceiveData, size_t rxSize);

// *****************************************************************************
/* Function:
    bool SERCOM1_SPI_Write(void* pTransmitData, size_t txSize);

  Summary:
    Writes data to SERCOM SERCOM1 SPI peripheral.

  Description:
    This function writes "txSize" number of bytes on SERCOM SERCOM1 SPI module. Data
    pointed by pTransmitData is transmitted.

    When "Interrupt Mode" option is unchecked in MHC, this function will be
    blocking in nature.  In this mode, the function will not return until all
    the requested data is transferred.  The function returns true after
    transferring all the data.  This indicates that the operation has been
    completed.

    When "Interrupt Mode" option is selected in MHC, the function will be
    non-blocking in nature.  The function returns immediately. The data transfer
    process continues in the peripheral interrupt.  The application specified
    transmit buffer  is ownerd by the library until the data transfer is
    complete and should not be modified by the application till the transfer is
    complete.  Only one transfer is allowed at any time. The application can use
    a callback function or a polling function to check for completion of the
    transfer. If a callback is required, this should be registered prior to
    calling the SERCOM1_SPI_WriteRead() function. The application can use the
    SERCOM1_SPI_IsBusy() to poll for completion.

  Precondition:
    The SERCOM1_SPI_Initialize function must have been called.

    Callback has to be registered using SERCOM1_SPI_CallbackRegister API if the
    peripheral instance has been configured in Interrupt mode and
    transfer completion status needs to be communicated back to application via
    callback.

  Parameters:
    pTransmitData - Pointer to the buffer containing the data which has to be
    transmitted.  For 9 bit mode, data should be right aligned in the 16 bit
    memory location. In "Interrupt Mode", this buffer should not be modified
    after calling the function and before the callback function has been called
    or the SERCOM1_SPI_IsBusy() function returns false.

    txSize - Number of bytes to be transmitted. For 9 bit mode, 2 bytes make up
    a count of 1.

  Returns:
    true - If configured for Non-interrupt mode, the function has transmitted
    the requested number of bytes. If configured for Interrupt mode, the request
    was accepted successfully and will be processed in the interrupt.

    false - If pTransmitData is NULL. In Interrupt mode, the function will
    additionally return false if there is an on-going data transfer at the time
    of calling the function.

  Example:
    <code>
    uint8_t txBuffer[4];
    size_t txSize = 4;

    void APP_SPITransferHandler(uintptr_t context)
    {
        //Transfer was completed without error, do something else now.
    }

    SERCOM1_SPI_Initialize();
    SERCOM1_SPI_CallbackRegister(&APP_SPITransferHandler, (uintptr_t)NULL);
    if(SERCOM1_SPI_Write(&txBuffer, txSize))
    {
        // request got accepted
    }
    else
    {
        // request didn't get accepted, try again later with correct arguments
    }

    </code>

  Remarks:
    None.

*/

bool SERCOM1_SPI_Write(void* pTransmitData, size_t txSize);

// *****************************************************************************
/* Function:
    bool SERCOM1_SPI_Read(void* pReceiveData, size_t rxSize);

  Summary:
    Reads data on the SERCOM SERCOM1 SPI peripheral.

  Description:
    This function reads "rxSize" number of bytes on SERCOM SERCOM1 SPI module. The
    received data is stored in the buffer pointed by pReceiveData.

    When "Interrupt Mode" option is unchecked in MHC, this function will be
    blocking in nature.  In this mode, the function will not return until all
    the requested data is transferred.  The function returns true after
    receiving "rxSize" number of bytes.  This indicates that the operation has
    been completed.

    When "Interrupt Mode" option is selected in MHC, the function will be
    non-blocking in nature.  The function returns immediately. The data transfer
    process continues in the peripheral interrupt.  The application specified
    receive buffer  is ownerd by the library until the data transfer is
    complete and should not be modified by the application till the transfer is
    complete.  Only one transfer is allowed at any time. The application can use
    a callback function or a polling function to check for completion of the
    transfer. If a callback is required, this should be registered prior to
    calling the SERCOM1_SPI_WriteRead() function. The application can use the
    SERCOM1_SPI_IsBusy() to poll for completion.

  Precondition:
    The SERCOM1_SPI_Initialize function must have been called.

    Callback has to be registered using SERCOM1_SPI_CallbackRegister API if the
    peripheral instance has been configured in Interrupt mode and
    transfer completion status needs to be communicated back to application via
    callback.

  Parameters:
    pReceiveData - Pointer to the buffer where the received data will be stored.
    For 9 bit mode, data should be right aligned in the 16 bit memory location.
    In "Interrupt Mode", this buffer should not be modified after calling the
    function and before the callback function has been called or the
    SERCOM1_SPI_IsBusy() function returns false.

    rxSize - Number of bytes to be received. For 9 bit mode, 2 bytes make up a
    count of 1.

  Returns:
    true - If configured for Non-interrupt mode, the function has received the
    requested number of bytes. If configured for Interrupt mode, the request was
    accepted successfully and will be processed in the interrupt.

    false - If pReceiveData is NULL. In Interrupt mode, the function will
    additionally return false if there is an on-going data transfer at the time
    of calling the function.

  Example:
    <code>
    uint8_t     rxBuffer[10];
    size_t      rxSize = 10;

    void APP_SPITransferHandler(uintptr_t context)
    {
        //Transfer was completed without error, do something else now.
    }

    SERCOM1_SPI_Initialize();
    SERCOM1_SPI_CallbackRegister(&APP_SPITransferHandler, (uintptr_t)NULL);
    if(SERCOM1_SPI_Read(&rxBuffer, rxSize))
    {
        // request got accepted
    }
    else
    {
        // request didn't get accepted, try again later with correct arguments
    }
    </code>

  Remarks:
    None.
*/

bool SERCOM1_SPI_Read(void* pReceiveData, size_t rxSize);

// *****************************************************************************
/* Function:
    void SERCOM1_SPI_CallbackRegister(const SERCOM_SPI_CALLBACK* callBack,
                                                    uintptr_t context);

  Summary:
    Allows application to register callback with PLIB.

  Description:
    This function allows application to register an event handling function
    for the PLIB to call back when requested data exchange operation has
    completed or any error has occurred.
    The callback should be registered before the client performs exchange
    operation.
    At any point if application wants to stop the callback, it can use this
    function with "callBack" value as NULL.

  Precondition:
    The SERCOM1_SPI_Initialize function must have been called.

  Parameters:
    callBack - Pointer to the event handler function implemented by the
               user .

    context - The value of parameter will be passed back to the application
              unchanged, when the callBack function is called. It can
              be used to identify any application specific data object that
              identifies the instance of the client module (for example,
              it may be a pointer to the client module's state structure).

  Returns:
    None.

  Example:
    <code>
    uint8_t txBuffer[10];
    uint8_t rxBuffer[10];
    size_t txSize = 10;
    size_t rxSize = 10;

    SERCOM1_SPI_Initialize();

    SERCOM1_SPI_CallbackRegister(&APP_SPICallBack, (uintptr_t)NULL);

    if(SERCOM1_SPI_WriteRead(&txBuffer, txSize, &rxBuffer, rxSize ))
    {
        // request got accepted
    }
    else
    {
        // request didn't get accepted, try again later with correct arguments
    }

    void APP_SPICallBack(uintptr_t contextHandle)
        {
            //Exchange was completed without error, do something else.
        }
    </code>

  Remarks:
    If the client does not want to be notified when the queued operation
    has completed, it does not need to register a callback.
*/

void SERCOM1_SPI_CallbackRegister(SERCOM_SPI_CALLBACK callBack, uintptr_t context);

// *****************************************************************************
/* Function:
    bool SERCOM1_SPI_IsBusy (void);

  Summary:
    Returns transfer status of SERCOM SERCOM1SPI.

  Description:
    This function ture if the SERCOM SERCOM1SPI module is busy with a transfer. The
    application can use the function to check if SERCOM SERCOM1SPI module is busy
    before calling any of the data transfer functions. The library does not
    allow a data transfer operation if another transfer operation is already in
    progress.

    This function can be used as an alternative to the callback function when
    the library is operating interrupt mode. The allow the application to
    implement a synchronous interface to the library.

  Precondition:
    The SERCOM1_SPI_Initialize() should have been called once. The module should
    have been configured for interrupt mode operation in MHC.

  Parameters:
    None.

  Returns:
    true -  Transfer is still in progress
    false - Transfer is completed or no transfer is currently in progress.

  Example:
    <code>
        // The following code example demonstrates the use of the
        // SERCOM1_SPI_IsBusy() function. This example shows a blocking while
        // loop. The function can also be called periodically.

        uint8_t dataBuffer[20];

        SERCOM1_SPI_Initialize();
        SERCOM1_SPI_Write(dataBuffer, 20);

        while (SERCOM1_SPI_IsBusy() == true)
        {
            // Wait here till the transfer is done.
        }
    </code>

  Remarks:
    None.
*/

bool SERCOM1_SPI_IsBusy (void);


#ifdef __cplusplus // Provide C++ Compatibility
}
#endif

#endif /* PLIB_SERCOM1_SPI_MASTER_H */