/*******************************************************************************
  SYS UART CONSOLE Support Layer

  File Name:
    sys_console_uart.c

  Summary:
    SYS UART CONSOLE Support Layer

  Description:
    This file contains the SYS UART CONSOLE support layer logic.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "system/console/sys_console.h"
#include "sys_console_uart.h"
#include "configuration.h"
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Variable Definitions
// *****************************************************************************
// *****************************************************************************

const SYS_CONSOLE_DEV_DESC sysConsoleUARTDevDesc =
{
    .consoleDevice              = SYS_CONSOLE_DEV_USART,
    .intent                     = DRV_IO_INTENT_READWRITE,
    .init                       = Console_UART_Initialize,
    .read                       = Console_UART_Read,
    .readFreeBufferCountGet     = Console_UART_ReadFreeBufferCountGet,
    .readCountGet               = Console_UART_ReadCountGet,
    .write                      = Console_UART_Write,
    .writeFreeBufferCountGet    = Console_UART_WriteFreeBufferCountGet,
    .writeCountGet              = Console_UART_WriteCountGet,
    .task                       = Console_UART_Tasks,
    .status                     = Console_UART_Status,
    .flush                      = Console_UART_Flush,
};

static CONSOLE_UART_DATA gConsoleUartData[SYS_CONSOLE_UART_MAX_INSTANCES];

#define CONSOLE_UART_GET_INSTANCE(index)    (index >= SYS_CONSOLE_UART_MAX_INSTANCES)? NULL : &gConsoleUartData[index]

static bool Console_UART_ResourceLock(CONSOLE_UART_DATA* pConsoleUartData)
{
    if(OSAL_MUTEX_Lock(&(pConsoleUartData->mutexTransferObjects), OSAL_WAIT_FOREVER) == OSAL_RESULT_FALSE)
    {
        return false;
    }
    else
    {
        return true;
    }
}

static void Console_UART_ResourceUnlock(CONSOLE_UART_DATA* pConsoleUartData)
{
    /* Release mutex */
    OSAL_MUTEX_Unlock(&(pConsoleUartData->mutexTransferObjects));
}

void Console_UART_Initialize(uint32_t index, const void* initData)
{
    CONSOLE_UART_DATA* pConsoleUartData = CONSOLE_UART_GET_INSTANCE(index);
    const SYS_CONSOLE_UART_INIT_DATA* consoleUsartInitData = (const SYS_CONSOLE_UART_INIT_DATA*)initData;

    if (pConsoleUartData == NULL)
    {
        return;
    }

    if(OSAL_MUTEX_Create(&(pConsoleUartData->mutexTransferObjects)) != OSAL_RESULT_TRUE)
    {
        return;
    }

    /* Assign the USART PLIB instance APIs to use */
    pConsoleUartData->uartPLIB = consoleUsartInitData->uartPLIB;

    pConsoleUartData->status = SYS_CONSOLE_STATUS_CONFIGURED;
}

/* Read out the data from the RX Ring Buffer */
ssize_t Console_UART_Read(uint32_t index, void* pRdBuffer, size_t count)
{
    ssize_t nBytesRead = 0;
    CONSOLE_UART_DATA* pConsoleUartData = CONSOLE_UART_GET_INSTANCE(index);

    if (pConsoleUartData == NULL)
    {
        return -1;
    }

    if (Console_UART_ResourceLock(pConsoleUartData) == false)
    {
        return -1;
    }

    nBytesRead = pConsoleUartData->uartPLIB->read(pRdBuffer, count);

    Console_UART_ResourceUnlock(pConsoleUartData);

    return nBytesRead;
}

/* Return number of unread bytes available in the RX Ring Buffer */
ssize_t Console_UART_ReadCountGet(uint32_t index)
{
    ssize_t nUnreadBytesAvailable = 0;

    CONSOLE_UART_DATA* pConsoleUartData = CONSOLE_UART_GET_INSTANCE(index);

    if (pConsoleUartData == NULL)
    {
        return -1;
    }

    if (Console_UART_ResourceLock(pConsoleUartData) == false)
    {
        return -1;
    }

    nUnreadBytesAvailable = pConsoleUartData->uartPLIB->readCountGet();

    Console_UART_ResourceUnlock(pConsoleUartData);

    return nUnreadBytesAvailable;
}

/* Return free space available in the RX Ring Buffer */
ssize_t Console_UART_ReadFreeBufferCountGet(uint32_t index)
{
    ssize_t nFreeBufferAvailable = 0;

    CONSOLE_UART_DATA* pConsoleUartData = CONSOLE_UART_GET_INSTANCE(index);

    if (pConsoleUartData == NULL)
    {
        return -1;
    }

    if (Console_UART_ResourceLock(pConsoleUartData) == false)
    {
        return -1;
    }

    nFreeBufferAvailable = pConsoleUartData->uartPLIB->readFreeBufferCountGet();

    Console_UART_ResourceUnlock(pConsoleUartData);

    return nFreeBufferAvailable;
}

ssize_t Console_UART_Write(uint32_t index, const void* pWrBuffer, size_t count )
{
    ssize_t nBytesWritten = 0;

    CONSOLE_UART_DATA* pConsoleUartData = CONSOLE_UART_GET_INSTANCE(index);

    if (pConsoleUartData == NULL)
    {
        return -1;
    }

    if (Console_UART_ResourceLock(pConsoleUartData) == false)
    {
        return -1;
    }

    nBytesWritten = pConsoleUartData->uartPLIB->write((uint8_t*)pWrBuffer, count);

    Console_UART_ResourceUnlock(pConsoleUartData);

    return nBytesWritten;
}

ssize_t Console_UART_WriteFreeBufferCountGet(uint32_t index)
{
    ssize_t nFreeBufferAvailable = 0;

    CONSOLE_UART_DATA* pConsoleUartData = CONSOLE_UART_GET_INSTANCE(index);

    if (pConsoleUartData == NULL)
    {
        return -1;
    }

    if (Console_UART_ResourceLock(pConsoleUartData) == false)
    {
        return -1;
    }

    nFreeBufferAvailable = pConsoleUartData->uartPLIB->writeFreeBufferCountGet();

    Console_UART_ResourceUnlock(pConsoleUartData);

    return nFreeBufferAvailable;
}

ssize_t Console_UART_WriteCountGet(uint32_t index)
{
    ssize_t nPendingTxBytes = 0;

    CONSOLE_UART_DATA* pConsoleUartData = CONSOLE_UART_GET_INSTANCE(index);

    if (pConsoleUartData == NULL)
    {
        return -1;
    }

    if (Console_UART_ResourceLock(pConsoleUartData) == false)
    {
        return -1;
    }

    nPendingTxBytes = pConsoleUartData->uartPLIB->writeCountGet();

    Console_UART_ResourceUnlock(pConsoleUartData);

    return nPendingTxBytes;
}

bool Console_UART_Flush(uint32_t index)
{
    /* Data is not buffered, nothing to flush */
    return true;
}

SYS_CONSOLE_STATUS Console_UART_Status(uint32_t index)
{
    CONSOLE_UART_DATA* pConsoleUartData = CONSOLE_UART_GET_INSTANCE(index);

    if (pConsoleUartData == NULL)
    {
        return SYS_CONSOLE_STATUS_ERROR;
    }
    else
    {
        return pConsoleUartData->status;
    }
}

void Console_UART_Tasks(uint32_t index, SYS_MODULE_OBJ object)
{
    /* Do nothing. */
}