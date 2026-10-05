/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_common.c
 * @brief Core2 for AWS IoT Kit helper library used across BSP drivers
 */

#include <stdint.h>
#include <inttypes.h>
#include <stdatomic.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/spi_master.h>

#include "core2foraws_common.h"

SemaphoreHandle_t core2foraws_common_spi_semaphore = NULL;

static StaticSemaphore_t _spi_semaphore_storage;
static atomic_uchar _spi_semaphore_state;
static atomic_uchar _spi_bus_state;

enum
{
    COMMON_RESOURCE_UNINITIALIZED = 0,
    COMMON_RESOURCE_INITIALIZING,
    COMMON_RESOURCE_READY,
};

#define SHARED_SPI_HOST SPI2_HOST
#define SHARED_SPI_MOSI GPIO_NUM_23
#define SHARED_SPI_MISO GPIO_NUM_38
#define SHARED_SPI_SCLK GPIO_NUM_18
#define SHARED_SPI_MAX_TRANSFER_BYTES CORE2FORAWS_SPI_MAX_TRANSFER_BYTES

static const char *_TAG = "CORE2FORAWS_COMMON";

esp_err_t core2foraws_common_spi_semaphore_init( void )
{
    if( atomic_load( &_spi_semaphore_state ) == COMMON_RESOURCE_READY )
    {
        return ESP_OK;
    }

    unsigned char expected = COMMON_RESOURCE_UNINITIALIZED;
    if( atomic_compare_exchange_strong( &_spi_semaphore_state, &expected,
                                        COMMON_RESOURCE_INITIALIZING ) )
    {
        core2foraws_common_spi_semaphore =
            xSemaphoreCreateBinaryStatic( &_spi_semaphore_storage );
        if( core2foraws_common_spi_semaphore == NULL )
        {
            atomic_store( &_spi_semaphore_state,
                          COMMON_RESOURCE_UNINITIALIZED );
            return ESP_ERR_NO_MEM;
        }

        xSemaphoreGive( core2foraws_common_spi_semaphore );
        atomic_store( &_spi_semaphore_state, COMMON_RESOURCE_READY );
        return ESP_OK;
    }

    while( atomic_load( &_spi_semaphore_state ) ==
           COMMON_RESOURCE_INITIALIZING )
    {
        vTaskDelay( 1 );
    }

    return atomic_load( &_spi_semaphore_state ) == COMMON_RESOURCE_READY
               ? ESP_OK
               : ESP_ERR_NO_MEM;
}

esp_err_t core2foraws_common_spi_bus_init( void )
{
    esp_err_t err = core2foraws_common_spi_semaphore_init();
    if( err != ESP_OK )
    {
        return err;
    }

    if( atomic_load( &_spi_bus_state ) == COMMON_RESOURCE_READY )
    {
        return ESP_OK;
    }

    unsigned char expected = COMMON_RESOURCE_UNINITIALIZED;
    if( atomic_compare_exchange_strong( &_spi_bus_state, &expected,
                                        COMMON_RESOURCE_INITIALIZING ) )
    {
        const spi_bus_config_t bus_cfg = {
            .mosi_io_num = SHARED_SPI_MOSI,
            .miso_io_num = SHARED_SPI_MISO,
            .sclk_io_num = SHARED_SPI_SCLK,
            .quadwp_io_num = GPIO_NUM_NC,
            .quadhd_io_num = GPIO_NUM_NC,
            .max_transfer_sz = SHARED_SPI_MAX_TRANSFER_BYTES,
        };

        err = spi_bus_initialize( SHARED_SPI_HOST, &bus_cfg,
                                  SPI_DMA_CH_AUTO );
        atomic_store( &_spi_bus_state,
                      err == ESP_OK ? COMMON_RESOURCE_READY
                                    : COMMON_RESOURCE_UNINITIALIZED );
        return err;
    }

    while( atomic_load( &_spi_bus_state ) == COMMON_RESOURCE_INITIALIZING )
    {
        vTaskDelay( 1 );
    }

    return atomic_load( &_spi_bus_state ) == COMMON_RESOURCE_READY
               ? ESP_OK
               : ESP_FAIL;
}

esp_err_t core2foraws_common_error( int32_t error_code )
{
    ESP_LOGV( _TAG, "Original error code: %" PRId32, error_code );

    return ( error_code == 0 ) ? ESP_OK : ESP_FAIL;
}

esp_err_t core2foraws_common_task_stack_watermark( const char *tag,
                                                   TaskHandle_t task,
                                                   size_t *watermark_bytes )
{
    if( watermark_bytes == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    /* uxTaskGetStackHighWaterMark() reports the minimum free stack in words
       (StackType_t units); scale to bytes for a human-readable figure. */
    *watermark_bytes = ( size_t ) uxTaskGetStackHighWaterMark( task ) *
                       sizeof( StackType_t );
    const char *name = pcTaskGetName( task );

    ESP_LOGI( tag != NULL ? tag : _TAG,
              "Task '%s' minimum free stack: %u bytes",
              name != NULL ? name : "self",
              ( unsigned int ) *watermark_bytes );

    return ESP_OK;
}

esp_err_t core2foraws_common_heap_report(
    const char *tag, core2foraws_common_heap_stats_t *stats )
{
    core2foraws_common_heap_stats_t local;

    local.internal_free = heap_caps_get_free_size( MALLOC_CAP_INTERNAL );
    local.internal_minimum_free =
        heap_caps_get_minimum_free_size( MALLOC_CAP_INTERNAL );
    local.internal_largest_block =
        heap_caps_get_largest_free_block( MALLOC_CAP_INTERNAL );
    local.dma_free = heap_caps_get_free_size( MALLOC_CAP_DMA );
    local.dma_minimum_free =
        heap_caps_get_minimum_free_size( MALLOC_CAP_DMA );
    local.dma_largest_block =
        heap_caps_get_largest_free_block( MALLOC_CAP_DMA );
    local.spiram_free = heap_caps_get_free_size( MALLOC_CAP_SPIRAM );
    local.spiram_minimum_free =
        heap_caps_get_minimum_free_size( MALLOC_CAP_SPIRAM );

    const char *log_tag = tag != NULL ? tag : _TAG;
    ESP_LOGI( log_tag,
              "Internal DRAM free: %u bytes (minimum %u, largest block %u)",
              ( unsigned int ) local.internal_free,
              ( unsigned int ) local.internal_minimum_free,
              ( unsigned int ) local.internal_largest_block );
    ESP_LOGI( log_tag,
              "DMA-capable free: %u bytes (minimum %u, largest block %u)",
              ( unsigned int ) local.dma_free,
              ( unsigned int ) local.dma_minimum_free,
              ( unsigned int ) local.dma_largest_block );
    ESP_LOGI( log_tag, "PSRAM free: %u bytes (minimum %u)",
              ( unsigned int ) local.spiram_free,
              ( unsigned int ) local.spiram_minimum_free );

    if( stats != NULL )
    {
        *stats = local;
    }

    return ESP_OK;
}