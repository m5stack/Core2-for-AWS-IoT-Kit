/*******************************************************************************
  WINC Wireless Driver SPI Communication Support

  File Name:
    wdrv_winc_spi.c

  Summary:
    WINC Wireless Driver SPI Communications Support

  Description:
    Supports SPI communications to the WINC module.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019-20 Microchip Technology Inc. and its subsidiaries.
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

#include "configuration.h"
#include "definitions.h"
#include "osal/osal.h"
#include "wdrv_winc_common.h"

#if defined(__PIC32MZ__)
#include "system/cache/sys_cache.h"
#include "sys/kmem.h"

#define WDRV_DCACHE_CLEAN(addr, size) _DataCacheClean(addr, size)
#else /* !defined(__PIC32MZ__) */
#define WDRV_DCACHE_CLEAN(addr, size) do { } while (0)
#endif /* defined(__PIC32MZ__) */

#ifdef DRV_SPI_DMA_MODE
#define SPI_DMA_DCACHE_CLEAN(addr, size) WDRV_DCACHE_CLEAN(addr, size)
#define SPI_DMA_MAX_TX_SIZE 1024
#define SPI_DMA_MAX_RX_SIZE 1024
#else /* (DRV_SPI_DMA_MODE != 0) */
#define SPI_DMA_DCACHE_CLEAN(addr, size) do { } while (0)
#endif /* (DRV_SPI_DMA != 0) */

static DRV_HANDLE spiHandle = DRV_HANDLE_INVALID;
static OSAL_SEM_HANDLE_TYPE txSyncSem;
static OSAL_SEM_HANDLE_TYPE rxSyncSem;

#if defined(__PIC32MZ__)
/****************************************************************************
 * Function:        _DataCacheClean
 * Summary: Used in Cache management to clean cache based on address.
 * Cache Management to be enabled in core & system components of MHC.
 * If not enabled in the project graph, below code segment should be disabled.
 *****************************************************************************/
static void _DataCacheClean(unsigned char *address, uint32_t size)
{
    if (IS_KVA0(address))
    {
        uint32_t a = (uint32_t)address & 0xfffffff0;
        uint32_t r = (uint32_t)address & 0x0000000f;
        uint32_t s = ((size + r + 15) >> 4) << 4;

        SYS_CACHE_CleanDCache_by_Addr((uint32_t *)a, s);
    }
}
#endif /* defined(__PIC32MZ__) */

static DRV_SPI_TRANSFER_HANDLE transferTxHandle;
static DRV_SPI_TRANSFER_HANDLE transferRxHandle;

static bool _SPI_Tx(unsigned char *buf, uint32_t size)
{
    SPI_DMA_DCACHE_CLEAN(buf, size);
    DRV_SPI_WriteTransferAdd(spiHandle, buf, size, &transferTxHandle);

    if(transferTxHandle == DRV_SPI_TRANSFER_HANDLE_INVALID)
    {
        // Error handling here
        return false;
    }

    while (OSAL_RESULT_FALSE == OSAL_SEM_Pend(&txSyncSem, OSAL_WAIT_FOREVER))
    {

    }

    return true;
}

static bool _SPI_Rx(unsigned char *const buf, uint32_t size)
{
    static uint8_t dummy = 0;

    SPI_DMA_DCACHE_CLEAN(buf, size);

    DRV_SPI_WriteReadTransferAdd(spiHandle, &dummy, 1, buf, size, &transferRxHandle);

    if(transferRxHandle == DRV_SPI_TRANSFER_HANDLE_INVALID)
    {
        // Error handling here
        return false;
    }
    while (OSAL_RESULT_FALSE == OSAL_SEM_Pend(&rxSyncSem, OSAL_WAIT_FOREVER))
    {
    }

    return true;
}

static void _WDRV_WINC_SPITransferEventHandler(DRV_SPI_TRANSFER_EVENT event,
        DRV_SPI_TRANSFER_HANDLE handle, uintptr_t context)
{
    // The context handle was set to an application specific
    // object. It is now retrievable easily in the event handler.
   // MY_APP_OBJ myAppObj = (MY_APP_OBJ *) context;

    switch(event)
    {
        case DRV_SPI_TRANSFER_EVENT_COMPLETE:
            // This means the data was transferred.
            if (transferTxHandle == handle)
            {
                OSAL_SEM_PostISR(&txSyncSem);
            }
            else if (transferRxHandle == handle)
            {
                OSAL_SEM_PostISR(&rxSyncSem);
            }

            break;

        case DRV_SPI_TRANSFER_EVENT_ERROR:
            // Error handling here.
            break;

        default:
            break;
    }
}

/****************************************************************************
 * Function:        WDRV_WINC_SPISend
 * Summary: Sends data out to the module through the SPI bus.
 *****************************************************************************/
bool WDRV_WINC_SPISend(unsigned char *const buf, uint32_t size)
{
    bool ret = true;
    unsigned char *pData;

    pData = buf;

#ifdef DRV_SPI_DMA_MODE
    while ((true == ret) && (size > SPI_DMA_MAX_TX_SIZE))
    {
        ret = _SPI_Tx(pData, SPI_DMA_MAX_TX_SIZE);
        size -= SPI_DMA_MAX_TX_SIZE;
        pData += SPI_DMA_MAX_TX_SIZE;
    }
#endif

    if ((true == ret) && (size > 0))
    {
        ret = _SPI_Tx(pData, size);
    }

    return ret;
}

/****************************************************************************
 * Function:        WDRV_WINC_SPIReceive
 * Summary: Receives data from the module through the SPI bus.
 *****************************************************************************/
bool WDRV_WINC_SPIReceive(unsigned char *const buf, uint32_t size)
{
    bool ret = true;
    unsigned char *pData;

    pData = buf;

#ifdef DRV_SPI_DMA_MODE
    while ((true == ret) && (size > SPI_DMA_MAX_RX_SIZE))
    {
        ret = _SPI_Rx(pData, SPI_DMA_MAX_RX_SIZE);
        size -= SPI_DMA_MAX_RX_SIZE;
        pData += SPI_DMA_MAX_RX_SIZE;
    }
#endif

    if ((true == ret) && (size > 0))
    {
        ret = _SPI_Rx(pData, size);
    }

    return ret;
}

/****************************************************************************
 * Function:        WDRV_WINC_SPIInitialize
 * Summary: Initializes the SPI object for the WiFi driver.
 *****************************************************************************/
void WDRV_WINC_SPIInitialize(void)
{
    if (OSAL_RESULT_TRUE != OSAL_SEM_Create(&txSyncSem, OSAL_SEM_TYPE_COUNTING, 10, 0))
    {
        return;
    }

    if (OSAL_RESULT_TRUE != OSAL_SEM_Create(&rxSyncSem, OSAL_SEM_TYPE_COUNTING, 10, 0))
    {
        return;
    }

    if (DRV_HANDLE_INVALID == spiHandle)
    {
        spiHandle = DRV_SPI_Open(WDRV_WINC_SPI_INDEX, DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_BLOCKING);

        if (DRV_HANDLE_INVALID == spiHandle)
        {
            WDRV_DBG_ERROR_PRINT("SPI init failed\r\n");
        }
    }

    DRV_SPI_TransferEventHandlerSet( spiHandle, _WDRV_WINC_SPITransferEventHandler, 0);
}

/****************************************************************************
 * Function:        WDRV_WINC_SPIDenitialize
 * Summary: Deinitializes the SPI object for the WiFi driver.
 *****************************************************************************/
void WDRV_WINC_SPIDeinitialize(void)
{
    OSAL_SEM_Post(&txSyncSem);
    OSAL_SEM_Delete(&txSyncSem);

    OSAL_SEM_Post(&rxSyncSem);
    OSAL_SEM_Delete(&rxSyncSem);

    DRV_SPI_Close(spiHandle);
}

//DOM-IGNORE-END
