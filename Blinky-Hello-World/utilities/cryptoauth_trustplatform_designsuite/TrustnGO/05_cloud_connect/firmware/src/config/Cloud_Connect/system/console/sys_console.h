/*******************************************************************************
  Console System Service Interface Definition

  Company:
    Microchip Technology Inc.

  File Name:
    sys_console.h

  Summary:
    Console System Service interface definitions.

  Description:
    This file contains the interface definition for the Console system
    service.  It provides a way to interact with the Console subsystem to
    manage the timing requests supported by the system.
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

#ifndef SYS_CONSOLE_H
#define SYS_CONSOLE_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "configuration.h"
#include "system/system.h"
#include "driver/driver.h"
#include "system/console/src/sys_console_local.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: SYS CONSOLE Data Types
// *****************************************************************************
// *****************************************************************************

/* These are in unistd.h */
#define STDIN_FILENO     0
#define STDOUT_FILENO    1
#define STDERR_FILENO    2

#define SYS_CONSOLE_DEFAULT_INSTANCE    0

/* The SYS_CONSOLE_PRINT and SYS_CONSOLE_MESSAGE macros print on the default console instance 0 */
#ifdef SYS_CONSOLE_PRINT
    #undef SYS_CONSOLE_PRINT
	// Print formatted message on the default console instance
    #define SYS_CONSOLE_PRINT(fmt, ...)                 SYS_CONSOLE_Print(SYS_CONSOLE_DEFAULT_INSTANCE, fmt, ##__VA_ARGS__)
#else
    #define SYS_CONSOLE_PRINT(fmt, ...)                 SYS_CONSOLE_Print(SYS_CONSOLE_DEFAULT_INSTANCE, fmt, ##__VA_ARGS__)
#endif

#ifdef SYS_CONSOLE_MESSAGE
    #undef SYS_CONSOLE_MESSAGE
	// Print message on the default console instance
    #define SYS_CONSOLE_MESSAGE(message)                SYS_CONSOLE_Message(SYS_CONSOLE_DEFAULT_INSTANCE, message)
#else
    #define SYS_CONSOLE_MESSAGE(message)                SYS_CONSOLE_Message(SYS_CONSOLE_DEFAULT_INSTANCE, message)
#endif

typedef enum
{
    SYS_CONSOLE_STATUS_NOT_CONFIGURED,

    SYS_CONSOLE_STATUS_CONFIGURED,

    SYS_CONSOLE_STATUS_BUSY,

    SYS_CONSOLE_STATUS_ERROR

} SYS_CONSOLE_STATUS;

// *****************************************************************************
/*  Console device enumeration

  Summary:
    Lists the available console devices.

  Description:
    This enumeration lists all of the available console devices. A console
    device is a physical peripheral used by the console service to send
    and receive data.

  Remarks:
    None.
*/
typedef enum
{
    SYS_CONSOLE_DEV_USART,

    SYS_CONSOLE_DEV_USB_CDC,

    SYS_CONSOLE_DEV_MAX,

} SYS_CONSOLE_DEVICE;

// *****************************************************************************
/* Console System Service Instance Handle

  Summary:
    Handle to an instance of the console system service.

  Description:
    This data type is a handle to a specific instance of the console system service

  Remarks:
    Code outside of a specific module should consider this as an opaque type
    (much like a void *).  Do not make any assumptions about base type as it
    may change in the future or about the value stored in a variable of this
    type.
*/

typedef uintptr_t SYS_CONSOLE_HANDLE;

// *****************************************************************************
/* Console System Service Handle Invalid

  Summary:
    Invalid console handle

  Description:
    This is handle value is returned in case of an unsuccessful console operation.

  Remarks:
    Do not rely on the actual value of this constant.  It may change in future
    implementations.
*/

#define SYS_CONSOLE_HANDLE_INVALID      ((SYS_CONSOLE_HANDLE) -1 )

// DOM-IGNORE-BEGIN

// *****************************************************************************
/*  Console device descriptor function prototypes

  Summary:
    Function prototype for the device descriptor expected by the system console.

  Description:
    Defines the function prototypes for the device descriptor that each device
    must implement in order to plug in to the system console service.

  Remarks:
    None.
*/

typedef void (*SYS_CONSOLE_INIT_FPTR) (uint32_t index, const void* initData);

typedef ssize_t (*SYS_CONSOLE_READ_FPTR) (uint32_t index, void* buf, size_t count);

typedef ssize_t (*SYS_CONSOLE_READ_FREE_BUFF_COUNT_GET_FPTR) (uint32_t index);

typedef ssize_t (*SYS_CONSOLE_READ_COUNT_GET_FPTR) (uint32_t index);

typedef ssize_t (*SYS_CONSOLE_WRITE_FPTR) (uint32_t index, const void* buf, size_t count);

typedef ssize_t (*SYS_CONSOLE_WRITE_FREE_BUFF_COUNT_GET_FPTR) (uint32_t index);

typedef ssize_t (*SYS_CONSOLE_WRITE_COUNT_GET_FPTR) (uint32_t index);

typedef void (*SYS_CONSOLE_TASK_FPTR) (uint32_t index, SYS_MODULE_OBJ object);

typedef SYS_CONSOLE_STATUS (*SYS_CONSOLE_STATUS_FPTR) (uint32_t index);

typedef bool (*SYS_CONSOLE_FLUSH_FPTR) (uint32_t index);

// *****************************************************************************
/*  Console device descriptor

  Summary:
    The console device must provide the implementation for the APIs described in
    the console device descriptor structure.

  Description:
    Each device must register its capability to the console system
    service. The capability APIs must confirm to the interface expected by the
    console system service.

  Remarks:
    None.
*/
typedef struct
{
    SYS_CONSOLE_DEVICE consoleDevice;

    DRV_IO_INTENT intent;

    SYS_CONSOLE_INIT_FPTR init;

    SYS_CONSOLE_READ_FPTR read;

    SYS_CONSOLE_READ_COUNT_GET_FPTR readCountGet;

    SYS_CONSOLE_READ_FREE_BUFF_COUNT_GET_FPTR readFreeBufferCountGet;

    SYS_CONSOLE_WRITE_FPTR write;

    SYS_CONSOLE_WRITE_COUNT_GET_FPTR writeCountGet;

    SYS_CONSOLE_WRITE_FREE_BUFF_COUNT_GET_FPTR writeFreeBufferCountGet;

    SYS_CONSOLE_TASK_FPTR task;

    SYS_CONSOLE_STATUS_FPTR status;

    SYS_CONSOLE_FLUSH_FPTR flush;

} SYS_CONSOLE_DEV_DESC;

// *****************************************************************************
/* SYS CONSOLE OBJECT INSTANCE structure

  Summary
    System Console object instance structure.

  Description:
    This data type defines the System Console object instance.

  Remarks:
    None.
*/

typedef struct
{
    /* State of this instance */
    SYS_STATUS status;

    const SYS_CONSOLE_DEV_DESC* devDesc;

    CONSOLE_DEVICE_INDEX devIndex;

} SYS_CONSOLE_OBJECT_INSTANCE;


// *****************************************************************************
//
/* SYS Console Initialize structure

  Summary:
    Identifies the system console initialize structure.

  Description:
    This structure identifies the system console initialize structure.

  Remarks:
    None.
*/

typedef struct
{
    /* Initialization data for the underlying device */
    const void* deviceInitData;

    const SYS_CONSOLE_DEV_DESC* consDevDesc;

    uint32_t deviceIndex;

} SYS_CONSOLE_INIT;

// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: SYS CONSOLE CONTROL Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ SYS_CONSOLE_Initialize(
        const SYS_MODULE_INDEX index,
        const SYS_MODULE_INIT* const init
    )

  Summary:
    Initializes the console instance module and opens or initializes the
    specific module instance to which it is associated.

  Description:
    This function initializes the internal data structures used by the console
    module. It also initializes the associated I/O driver/PLIB.

  Precondition:
    None.

  Parameters:
    index           - Index for the instance to be initialized

    init            - Pointer to a data structure containing any data necessary
                      to initialize the Console System service. This pointer
                      may be null if no data is required because static
                      overrides have been provided.

  Returns:
    If successful, returns a valid handle to the console instance. Otherwise, it
    returns SYS_MODULE_OBJ_INVALID. The returned object must be passed as
    argument to SYS_CONSOLE_Tasks and SYS_CONSOLE_Status routines.

  Example:
    <code>
    SYS_MODULE_OBJ  objectHandle;

    // Populate the console initialization structure
    const SYS_CONSOLE_INIT sysConsole0Init =
    {
        .deviceInitData = (void*)&sysConsole0UARTInitData,
        .consDevDesc = &sysConsoleUARTDevDesc,
        .deviceIndex = 0,
    };

    objectHandle = SYS_CONSOLE_Initialize(SYS_CONSOLE_INDEX_0, (SYS_MODULE_INIT *)&sysConsole0Init);
    if (objectHandle == SYS_MODULE_OBJ_INVALID)
    {
        // Handle error
    }
    </code>

  Remarks:
    This routine should only be called once during system initialization.
*/

SYS_MODULE_OBJ SYS_CONSOLE_Initialize(
    const SYS_MODULE_INDEX index,
    const SYS_MODULE_INIT* const init
);

// *****************************************************************************
/* Function:
    void SYS_CONSOLE_Tasks ( SYS_MODULE_OBJ object )

  Summary:
    Maintains the console's state machine.

  Description:
    This function runs the console system service's internal state machine.

  Precondition:
    The SYS_CONSOLE_Initialize function must have been called for the specified
    CONSOLE driver instance.

  Parameters:
    object - SYS CONSOLE object handle, returned from SYS_CONSOLE_Initialize

  Returns:
    None

  Example:
    <code>
    SYS_MODULE_OBJ object;     // Returned from SYS_CONSOLE_Initialize

    while (true)
    {
        SYS_CONSOLE_Tasks (object);

        // Do other tasks
    }
    </code>

  Remarks:
    This function is normally not called directly by an application.  It is
    called by the system's Tasks routine (SYS_Tasks) or by the appropriate raw
    ISR.
*/

void SYS_CONSOLE_Tasks ( SYS_MODULE_OBJ object );

// *****************************************************************************
/* Function:
    SYS_STATUS SYS_CONSOLE_Status ( SYS_MODULE_OBJ object )

  Summary:
    Returns status of the specific instance of the Console module.

  Description:
    This function returns the status of the specific module instance.

  Precondition:
    The SYS_CONSOLE_Initialize function should have been called before calling
    this function.

  Parameters:
    object    - SYS CONSOLE object handle, returned from SYS_CONSOLE_Initialize

  Returns:
    * SYS_STATUS_READY          - Indicates that the driver is initialized and is
                                  ready to accept new requests from the clients.

    * SYS_STATUS_BUSY           - Indicates that the driver is busy with a
                                  previous requests from the clients. However,
                                  depending on the configured queue size for
                                  transmit and receive, it may be able to queue
                                  a new request.

    * SYS_STATUS_ERROR          - Indicates that the driver is in an error state.
                                  Any value less than SYS_STATUS_ERROR is
                                  also an error state.

    * SYS_STATUS_UNINITIALIZED  - Indicates that the driver is not initialized.

  Example:
    <code>
    // Given "object" returned from SYS_CONSOLE_Initialize

    SYS_STATUS          consStatus;

    consStatus = SYS_CONSOLE_Status (object);
    if (consStatus == SYS_STATUS_READY)
    {
        // Console is initialized and is ready to accept client requests.
    }
    </code>

  Remarks:
    Application must ensure that the SYS_CONSOLE_Status returns SYS_STATUS_READY
    before performing console read/write.
*/

SYS_STATUS SYS_CONSOLE_Status( SYS_MODULE_OBJ object );

// *****************************************************************************
/* Function:
    SYS_CONSOLE_HANDLE SYS_CONSOLE_HandleGet( const SYS_MODULE_INDEX index)

  Summary:
    Returns a handle to the requested console instance

  Description:
    This function returns a handle to the requested console instance.

  Preconditions:
    The SYS_CONSOLE_Initialize function should have been called before calling
    this function.

  Parameters:
    index       - index of the console instance

  Returns:
    SYS_CONSOLE_HANDLE - Handle to the requested console instance

  Example:
    <code>
    SYS_CONSOLE_HANDLE myConsoleHandle;
    myConsoleHandle = SYS_CONSOLE_HandleGet(SYS_CONSOLE_INDEX_0);
    if (myConsoleHandle != SYS_CONSOLE_HANDLE_INVALID)
    {
        // Found a valid handle to the console instance

        // Write some data over the USB console
        SYS_CONSOLE_Write(myConsoleHandle, data, 10);
    }
    </code>

  Remarks:
    None.
*/
SYS_CONSOLE_HANDLE SYS_CONSOLE_HandleGet( const SYS_MODULE_INDEX index);

// *****************************************************************************
/* Function:
    SYS_CONSOLE_DEVICE SYS_CONSOLE_DeviceGet( const SYS_CONSOLE_HANDLE handle)

  Summary:
    Returns the device type for a given console instance

  Description:
    This function returns the device type supported by the given console instance

  Preconditions:
    The SYS_CONSOLE_Initialize function should have been called before calling
    this function.

  Parameters:
    handle       - A valid handle to the console instance

  Returns:
    SYS_CONSOLE_DEVICE - Returns console device type. Returns SYS_CONSOLE_DEV_MAX in
    case of an error.

  Example:
    <code>
    SYS_CONSOLE_HANDLE myConsoleHandle;
    SYS_CONSOLE_DEVICE myConsoleDevType
    // myConsoleHandle is assumed to be a valid console handle
    myConsoleDevType = SYS_CONSOLE_DeviceGet(myConsoleHandle);
    </code>

  Remarks:
    None.
*/
SYS_CONSOLE_DEVICE SYS_CONSOLE_DeviceGet( const SYS_CONSOLE_HANDLE handle);

// *****************************************************************************
/* Function:
    ssize_t SYS_CONSOLE_Read(
        const SYS_CONSOLE_HANDLE handle,
        void* buf,
        size_t count
    )

  Summary:
    Reads data from the console device.

  Description:
    This function reads the data from the console device.

  Preconditions:
    The SYS_CONSOLE_Initialize function should have been called before calling
    this function.

  Parameters:
    handle          - Handle to the console instance
    buf             - Buffer to hold the read data.
    count           - Number of bytes to read.

  Returns:
    Return value indicates the number of bytes actually read. Returns -1 in case
    of any error.

  Example:
    <code>
    ssize_t nr;     //indicates the actual number of bytes read
    char myBuffer[MY_BUFFER_SIZE];
    SYS_CONSOLE_HANDLE myConsoleHandle;

    // myConsoleHandle is assumed to be pointing to a valid console handle
    nr = SYS_CONSOLE_Read( myConsoleHandle, myBuffer, MY_BUFFER_SIZE );
    if (nr == -1)
    {
        // Handle error
    }
    </code>

  Remarks:
    If the data is not read out from the internal receive buffer by calling the
    SYS_CONSOLE_Read API at regular intervals, there is a possibility of the receive
    buffer becoming full. As a result, the new data may be lost. Hence, the
    application must call the SYS_CONSOLE_Read API at regular intervals to avoid
    buffer overflow condition. The SYS_CONSOLE_ReadCountGet() and the
    SYS_CONSOLE_ReadFreeBufferCountGet() APIs may be used to know the number of
    unread bytes available in the receive buffer and the amount of free space
    available in the receive buffer respectively.
*/

ssize_t SYS_CONSOLE_Read( const SYS_CONSOLE_HANDLE handle, void* buf, size_t count );

// *****************************************************************************
/* Function:
    ssize_t SYS_CONSOLE_Write(
        const SYS_CONSOLE_HANDLE handle,
        const void* buf,
        size_t count
    )

  Summary:
    Writes data to the console device.

  Description:
    This function writes data to the console device. The function performs a deep-copy
    of the data passed in buf.

  Preconditions:
    The SYS_CONSOLE_Initialize function should have been called before calling
    this function.

  Parameters:
    handle          - Handle to the console instance
    buf             - Buffer holding the data to be written.
    count           - Number of bytes to write.

  Returns:
    The return value indicates the number of bytes accepted by the function. In case
    the return value is less than the count value, the application must try to send the
    remaining bytes in next attempt(s). Returns -1 in case of any error.

  Example:
    <code>
    ssize_t nr;
    char myBuffer[] = "message";
    SYS_CONSOLE_HANDLE myConsoleHandle;

    // myConsoleHandle is assumed to be a valid console handle
    nr = SYS_CONSOLE_Write( myConsoleHandle, myBuffer, strlen(myBuffer) );
    if (nr == -1)
    {
        // Handle error
    }
    if (nr != strlen(myBuffer))
    {
        // Try send the remaining data after some time.
    }
    </code>

  Remarks:
    Application may check the free space available in the transmit buffer by
    calling the SYS_CONSOLE_WriteFreeBufferCountGet() API.
*/

ssize_t SYS_CONSOLE_Write( const SYS_CONSOLE_HANDLE handle, const void* buf, size_t count );

// *****************************************************************************
/* Function:
    bool SYS_CONSOLE_Flush(const SYS_CONSOLE_HANDLE handle)

  Summary:
    Flushes the read and write queues for the given console instance.

  Description:
    This function flushes the read and write buffers. Any on-going transfers
    will not be flushed.

  Preconditions:
    The SYS_CONSOLE_Initialize function should have been called before calling
    this function.

  Parameters:
    handle               - Handle to the console instance

  Returns:
    true                 - If the operation is successful
    false                - In case of failure

  Example:
    <code>
    SYS_CONSOLE_HANDLE myConsoleHandle;
    bool status;

    // myConsoleHandle is assumed to be a valid console handle

    status = SYS_CONSOLE_Flush(myConsoleHandle);
    if (status == false)
    {
        // Handle error
    }
    </code>

  Remarks:
    This API may do nothing and return true, where the read and write are not buffered.
*/

bool SYS_CONSOLE_Flush(const SYS_CONSOLE_HANDLE handle);

// *****************************************************************************
/* Function:
    ssize_t SYS_CONSOLE_ReadFreeBufferCountGet(const SYS_CONSOLE_HANDLE handle)

  Summary:
    Returns the amount of free space in bytes available in the receive buffer.

  Description:
    This function indicates the number of free space available in the receive buffer,
    thereby indicating the number of bytes that can be received without overflowing
    the receive buffer.

  Preconditions:
    The SYS_CONSOLE_Initialize function should have been called before calling
    this function.

  Parameters:
    handle           - Handle to a valid console instance

  Returns:
    The return value indicates the number of bytes of free space available in the receive
    buffer. Returns -1 in case of any error.

  Example:
    <code>
    ssize_t nr;
    SYS_CONSOLE_HANDLE myConsoleHandle;

    // myConsoleHandle is assumed to be a valid console handle
    nr = SYS_CONSOLE_ReadFreeBufferCountGet(myConsoleHandle);
    if (nr == -1)
    {
        // Handle error
    }
    </code>

  Remarks:
    None.
*/

ssize_t SYS_CONSOLE_ReadFreeBufferCountGet(const SYS_CONSOLE_HANDLE handle);

// *****************************************************************************
/* Function:
    ssize_t SYS_CONSOLE_ReadCountGet(const SYS_CONSOLE_HANDLE handle)

  Summary:
    Returns number of unread bytes available in the receive buffer.

  Description:
    This function indicates the number of unread bytes in the receive buffer.

  Preconditions:
    The SYS_CONSOLE_Initialize function should have been called before calling
    this function.

  Parameters:
    handle           - Handle to the console instance

  Returns:
    The return value indicates the number of bytes of unread data available in the
    receive buffer. Returns -1 in case of any error.

  Example:
    <code>
    ssize_t nUnreadBytes;
    ssize_t nBytesRead;
    char myBuffer[100];
    SYS_CONSOLE_HANDLE myConsoleHandle;

    // Get the number of bytes available in the receive buffer.
    nUnreadBytes = SYS_CONSOLE_ReadCountGet(myConsoleHandle);
    if (nUnreadBytes == -1)
    {
        // Handle error
    }
    // Read the available data into the application buffer.
    SYS_CONSOLE_Read( myConsoleHandle, 0, myBuffer, nUnreadBytes );
    </code>

  Remarks:
    None.
*/

ssize_t SYS_CONSOLE_ReadCountGet(const SYS_CONSOLE_HANDLE handle);

// *****************************************************************************
/* Function:
    ssize_t SYS_CONSOLE_WriteFreeBufferCountGet(const SYS_CONSOLE_HANDLE handle)

  Summary:
    Returns the amount of free space in bytes in the transmit buffer.

  Description:
    This function indicates the number of bytes of free space available in the
    transmit buffer.

  Preconditions:
    The SYS_CONSOLE_Initialize function should have been called before calling
    this function. In some cases, application may call this API to know the
    amount of free buffer space available in the transmit buffer before
    calling SYS_CONSOLE_Write() API.

  Parameters:
    handle           - Handle to the console instance

  Returns:
    The return value indicates the number of bytes of free space available in the
    transmit buffer. Returns -1 in case of any error.

  Example:
    <code>
    ssize_t nFreeSpace;
    char myBuffer[100];
    SYS_CONSOLE_HANDLE myConsoleHandle;

    // Get the number of bytes of free space available in the transmit buffer.
    nFreeSpace = SYS_CONSOLE_WriteFreeBufferCountGet(myConsoleHandle);
    if ((nFreeSpace >= sizeof(myBuffer)) && (nFreeSpace!= -1))
    {
        // Write the application buffer
        SYS_CONSOLE_Write( myConsoleHandle, myBuffer, sizeof(myBuffer) );
    }
    </code>

  Remarks:
    None.
*/
ssize_t SYS_CONSOLE_WriteFreeBufferCountGet(const SYS_CONSOLE_HANDLE handle);

// *****************************************************************************
/* Function:
    ssize_t SYS_CONSOLE_WriteCountGet(const SYS_CONSOLE_HANDLE handle)

  Summary:
    Returns the number of bytes pending for transmission in the transmit buffer.

  Description:
    This function indicates the number of bytes pending for transmission in the
    transmit buffer.

  Preconditions:
    The SYS_CONSOLE_Initialize function should have been called before calling
    this function.

  Parameters:
    handle           - Handle to the console instance

  Returns:
    The return value indicates the number of bytes present in the transmit buffer
    waiting to be transmitted. Returns -1 in case of any error.

  Example:
    <code>
    ssize_t nTxBytesPending;
    SYS_CONSOLE_HANDLE myConsoleHandle;

    nTxBytesPending = SYS_CONSOLE_WriteCountGet(myConsoleHandle);
    if (nTxBytesPending == -1)
    {
        // API reported error
    }
    if (nTxBytesPending == 0)
    {
        // All the data has been written to the console
    }
    </code>

  Remarks:
    None.
*/
ssize_t SYS_CONSOLE_WriteCountGet(const SYS_CONSOLE_HANDLE handle);

// *****************************************************************************
/* Function:
    void SYS_CONSOLE_Print(const SYS_CONSOLE_HANDLE handle, const char *format, ...)

  Summary:
    Formats and prints a message with a variable number of arguments to the
    console

  Description:
    This function formats and prints a message with a variable number of
    arguments to the console.

  Precondition:
    SYS_CONSOLE_Initialize must have returned a valid object handle.

  Parameters:
    handle			- Handle to a console instance
    format          - Pointer to a buffer containing the format string for
                      the message to be displayed.
    ...             - Zero or more optional parameters to be formated as
                      defined by the format string.

  Returns:
    None.

  Example:
    <code>
	SYS_CONSOLE_HANDLE myConsoleHandle;

	myConsoleHandle = SYS_CONSOLE_HandleGet(SYS_CONSOLE_INDEX_0);

	uint8_t num_bytes_to_enter = 10;

	if (myConsoleHandle != SYS_CONSOLE_HANDLE_INVALID)
    {
        // Found a valid handle to the console instance
        SYS_CONSOLE_Print(myConsoleHandle, "Enter %d characters", num_bytes_to_enter);
    }
    </code>

  Remarks:
    The format string and arguments follow the printf convention.
    Call SYS_CONSOLE_PRINT macro to print on the default console instance 0
*/

void SYS_CONSOLE_Print(const SYS_CONSOLE_HANDLE handle, const char *format, ...);

// *****************************************************************************
/* Function:
    void SYS_CONSOLE_Message(const SYS_CONSOLE_HANDLE handle, const char *message)

  Summary:
    Prints a message to the console

  Description:
    This function prints a message to the console

  Precondition:
    SYS_CONSOLE_Initialize must have returned a valid object handle.

  Parameters:
	handle			- Handle to a console instance
    message 		- Pointer to a message string to be displayed.

  Returns:
    None.

  Example:
    <code>

	SYS_CONSOLE_HANDLE myConsoleHandle;

	myConsoleHandle = SYS_CONSOLE_HandleGet(SYS_CONSOLE_INDEX_0);

	if (myConsoleHandle != SYS_CONSOLE_HANDLE_INVALID)
	{
		SYS_CONSOLE_Message(myConsoleHandle, "Printing using SYS_Message");
	}
    </code>

  Remarks:
    Call SYS_CONSOLE_MESSAGE macro to print on the default console instance 0
*/
void SYS_CONSOLE_Message(const SYS_CONSOLE_HANDLE handle, const char *message);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif //SYS_CONSOLE_H

/*******************************************************************************
 End of File
*/