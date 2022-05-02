/*
 * Core2 for AWS IoT EduKit BSP v2.0.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://aws.amazon.com/iot/edukit
 *
 */

/**
 * @file core2foraws_sd.c
 * @brief Core2 for AWS IoT EduKit SD card hardware driver APIs
 */

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>

#include "core2foraws_sd.h"
#include "core2foraws_common.h"

static sdmmc_card_t* _sd_card;
static const char *_mount_path = "/sd_card";
static size_t _mount_path_len;

static const char *_TAG = "CORE2FORAWS_SD";

/**
 * @brief The SPI2 peripheral that controls the SPI bus.
 *
 * This is peripheral type used for initializing the SPI bus.
 * There are two peripherals connected to the SPI bus, the
 * ILI9342C display controller and the TF/SD card slot.
 *
 */
/* @[declare_spi_host_use] */
#define SPI_HOST_USE HSPI_HOST
/* @[declare_spi_host_use] */

/**
 * @brief DMA channel for SPI bus
 *
 * This is the channel used by the SPI bus for the ILI9342C
 * display controller and TF/SD card slot.
 */
/* @[declare_spi_dma_chan] */
#define SPI_DMA_CHAN 2
/* @[declare_spi_dma_chan] */

esp_err_t core2foraws_sd_mount( void )
{
    esp_err_t err = ESP_OK;

    _mount_path_len = strlen( _mount_path );

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 512
    };
    sdmmc_card_t *card;

    host.slot = SPI_HOST_USE;

#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 1, 0)
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.host_id = host.slot;
#else
    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
#endif
    slot_config.gpio_cs = 4;
    xSemaphoreTake( core2foraws_common_spi_semaphore, portMAX_DELAY );
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL( 4, 1, 0 )
    err = esp_vfs_fat_sdspi_mount( _mount_path, &host, &slot_config, &mount_config, &card );
#else
    err = esp_vfs_fat_sdmmc_mount( _mount_path, &host, &slot_config, &mount_config, &card );
#endif
    xSemaphoreGive( core2foraws_common_spi_semaphore );
    if ( err == ESP_OK )
    {
        ESP_LOGD( _TAG, "Mounted SD card %s with mount point %s", card->cid.name, _mount_path );
        _sd_card = card;
    }
    
    vTaskDelay( pdMS_TO_TICKS( SD_ACCESS_DELAY_MS ) );
    return err;
}

esp_err_t core2foraws_sd_read( const char *file_name, char *message, size_t to_read_length )
{
    esp_err_t err = ESP_OK;
    
    
    const size_t file_name_len = strlen( file_name );
    char *path = heap_caps_malloc( _mount_path_len + file_name_len + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT );
    memcpy( path, _mount_path, _mount_path_len );
    memcpy( path + _mount_path_len, file_name, file_name_len + 1 );
    
    xSemaphoreTake( core2foraws_common_spi_semaphore, portMAX_DELAY );
    
    FILE* f = fopen( path, "r" );
    if ( f == NULL )
    {
        err = ESP_FAIL;
        ESP_LOGI( _TAG, "Failed to open SD card path %s", path );
        return err;
    }

    if ( fgets(message, to_read_length, f) == NULL )
    {
        err = ESP_FAIL;
        ESP_LOGI( _TAG, "Failed to read from SD card" );
    }

    fclose(f);
    xSemaphoreGive( core2foraws_common_spi_semaphore );
    free( path );
    vTaskDelay( pdMS_TO_TICKS( SD_ACCESS_DELAY_MS ) );

    return err;
}

esp_err_t core2foraws_sd_write( const char *file_name, const char* message, size_t *wrote_length )
{
    esp_err_t err = ESP_OK;
    
    const size_t file_name_len = strlen( file_name );
    char *path = heap_caps_malloc( _mount_path_len + file_name_len + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT );
    memcpy( path, _mount_path, _mount_path_len );
    memcpy( path + _mount_path_len, file_name, file_name_len + 1 );

    xSemaphoreTake( core2foraws_common_spi_semaphore, portMAX_DELAY );

    FILE* f = fopen(path, "w+");
    if (f == NULL) {
        err = ESP_FAIL;
        ESP_LOGD( _TAG, "Failed to open SD card" );
        return err;
    }

    int32_t wrote = fprintf(f, "%s", message);
    if ( wrote < 0 )
    {
        err = ESP_FAIL;
        ESP_LOGD( _TAG, "Failed to write to SD card" );
        *wrote_length = 0;
    }
    else
    {
        *wrote_length = wrote;
    }

    fclose(f);
    xSemaphoreGive( core2foraws_common_spi_semaphore );
    free( path );
    vTaskDelay( pdMS_TO_TICKS( SD_ACCESS_DELAY_MS ) );

    return err;
}

esp_err_t core2foraws_sd_unmount( void )
{
    esp_err_t err = ESP_FAIL;

    xSemaphoreTake( core2foraws_common_spi_semaphore, portMAX_DELAY );
    err = esp_vfs_fat_sdcard_unmount( _mount_path, _sd_card );
    xSemaphoreGive( core2foraws_common_spi_semaphore );
    
    if ( err == ESP_OK )
    {
        _sd_card = NULL;
    }

    return err;
}