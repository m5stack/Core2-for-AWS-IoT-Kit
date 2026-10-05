/*
 * Core2 for AWS IoT Kit BSP
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>

#include <esp_log.h>
#include <esp_rom_sys.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>

#include "core2foraws_common.h"
#include "core2foraws_i2c.h"

static const char *_TAG = "CORE2FORAWS_I2C";

/* Bus pin configurations */
#define INTERNAL_I2C_SDA    GPIO_NUM_21
#define INTERNAL_I2C_SCL    GPIO_NUM_22
#define INTERNAL_I2C_FREQ   100000

#define EXTERNAL_I2C_SDA    GPIO_NUM_32
#define EXTERNAL_I2C_SCL    GPIO_NUM_33
#define EXTERNAL_I2C_FREQ   100000

#define I2C_LOCK_TIMEOUT_MS 1000

/* Per-transfer timeout (milliseconds) passed to the i2c_master driver. A
 * bounded value ensures a stuck or clock-stretching device cannot block the
 * caller forever while holding the per-bus mutex. */
#define I2C_XFER_TIMEOUT_MS 1000

/* Stack buffer size for register writes (register address + payload). Writes
 * that fit are served without a heap allocation; larger writes fall back to
 * malloc. Sized to cover the common 1-2 byte register writes plus a small
 * multi-byte payload. */
#define I2C_WRITE_STACK_BUF_SIZE 32

/* Bus clear (I2C-bus spec UM10204 3.1.16): up to nine SCL pulses at 100 kHz. */
#define I2C_BUS_CLEAR_PULSES       9
#define I2C_BUS_CLEAR_HALF_US      5

/* The ATECC608 wakes on a deliberately NACKed write to the general-call
 * address and NACKs its own address while busy, so neither is reported. */
#define I2C_GENERAL_CALL_ADDRESS 0x00
#define I2C_ATECC608_ADDRESS     0x35

typedef struct core2foraws_i2c_device_node
{
    i2c_master_dev_handle_t handle;
    uint16_t address;
    uint32_t speed_hz;
    size_t references;
    struct core2foraws_i2c_device_node *next;
} core2foraws_i2c_device_node_t;

typedef struct
{
    i2c_master_bus_handle_t handle;
    StaticSemaphore_t mutex_storage;
    SemaphoreHandle_t mutex;
    atomic_uchar mutex_state;
    core2foraws_i2c_device_node_t *devices;
    int64_t last_general_call_us;
} core2foraws_i2c_bus_state_t;

enum
{
    I2C_MUTEX_UNINITIALIZED = 0,
    I2C_MUTEX_INITIALIZING,
    I2C_MUTEX_READY,
};

/* The internal bus lives for the BSP lifetime. The external bus can be
 * opened, populated with multiple application devices, closed, and reopened. */
static core2foraws_i2c_bus_state_t _bus_state[ CORE2FORAWS_I2C_PORT_MAX ];

static const gpio_num_t _sda_pin[ CORE2FORAWS_I2C_PORT_MAX ] = { INTERNAL_I2C_SDA, EXTERNAL_I2C_SDA };
static const gpio_num_t _scl_pin[ CORE2FORAWS_I2C_PORT_MAX ] = { INTERNAL_I2C_SCL, EXTERNAL_I2C_SCL };
static const i2c_port_num_t _i2c_port[ CORE2FORAWS_I2C_PORT_MAX ] = { I2C_NUM_0, I2C_NUM_1 };

static esp_err_t _i2c_mutex_ensure( core2foraws_i2c_port_t port )
{
    core2foraws_i2c_bus_state_t *state = &_bus_state[ port ];
    unsigned char current = atomic_load( &state->mutex_state );

    if( current == I2C_MUTEX_READY )
    {
        return ESP_OK;
    }

    unsigned char expected = I2C_MUTEX_UNINITIALIZED;
    if( atomic_compare_exchange_strong( &state->mutex_state, &expected,
                                        I2C_MUTEX_INITIALIZING ) )
    {
        state->mutex =
            xSemaphoreCreateRecursiveMutexStatic( &state->mutex_storage );
        if( state->mutex == NULL )
        {
            atomic_store( &state->mutex_state, I2C_MUTEX_UNINITIALIZED );
            return ESP_ERR_NO_MEM;
        }

        atomic_store( &state->mutex_state, I2C_MUTEX_READY );
        return ESP_OK;
    }

    while( atomic_load( &state->mutex_state ) == I2C_MUTEX_INITIALIZING )
    {
        vTaskDelay( 1 );
    }

    return atomic_load( &state->mutex_state ) == I2C_MUTEX_READY
               ? ESP_OK
               : ESP_ERR_NO_MEM;
}

static core2foraws_i2c_device_node_t *_i2c_device_find_by_handle(
    core2foraws_i2c_bus_state_t *state, i2c_master_dev_handle_t handle,
    core2foraws_i2c_device_node_t **previous )
{
    core2foraws_i2c_device_node_t *prev = NULL;
    core2foraws_i2c_device_node_t *node = state->devices;

    while( node != NULL )
    {
        if( node->handle == handle )
        {
            if( previous != NULL )
            {
                *previous = prev;
            }
            return node;
        }
        prev = node;
        node = node->next;
    }

    return NULL;
}

/* Called with the bus lock held after every managed transfer. Failures on
 * the internal bus are logged with the time since the last ATECC608 wake
 * token so a NACK can be correlated with (or ruled out from) that token. */
static void _i2c_transfer_note( core2foraws_i2c_port_t port,
                                const core2foraws_i2c_device_node_t *node,
                                esp_err_t err )
{
    core2foraws_i2c_bus_state_t *state = &_bus_state[ port ];
    int64_t now_us = esp_timer_get_time();

    if( node->address == I2C_GENERAL_CALL_ADDRESS )
    {
        state->last_general_call_us = now_us;
        return;
    }

    if( err == ESP_OK || port != CORE2FORAWS_I2C_INTERNAL ||
        node->address == I2C_ATECC608_ADDRESS )
    {
        return;
    }

    if( state->last_general_call_us != 0 )
    {
        ESP_LOGW( _TAG, "Transfer to 0x%02x failed: %s (t=%lld us, %lld us after last ATECC608 wake token)",
                  node->address, esp_err_to_name( err ),
                  ( long long ) now_us,
                  ( long long )( now_us - state->last_general_call_us ) );
    }
    else
    {
        ESP_LOGW( _TAG, "Transfer to 0x%02x failed: %s (t=%lld us, no ATECC608 wake token yet)",
                  node->address, esp_err_to_name( err ),
                  ( long long ) now_us );
    }
}

/* Clock a target that holds SDA low out of its byte, then send a STOP. */
static void _i2c_bus_clock_out( gpio_num_t sda, gpio_num_t scl )
{
    int pulses = 0;
    while( gpio_get_level( sda ) == 0 && pulses < I2C_BUS_CLEAR_PULSES )
    {
        gpio_set_level( scl, 0 );
        esp_rom_delay_us( I2C_BUS_CLEAR_HALF_US );
        gpio_set_level( scl, 1 );
        esp_rom_delay_us( I2C_BUS_CLEAR_HALF_US );
        pulses++;
    }

    gpio_set_level( scl, 0 );
    esp_rom_delay_us( I2C_BUS_CLEAR_HALF_US );
    gpio_set_level( sda, 0 );
    esp_rom_delay_us( I2C_BUS_CLEAR_HALF_US );
    gpio_set_level( scl, 1 );
    esp_rom_delay_us( I2C_BUS_CLEAR_HALF_US );
    gpio_set_level( sda, 1 );
    esp_rom_delay_us( I2C_BUS_CLEAR_HALF_US );

    if( gpio_get_level( sda ) == 0 )
    {
        ESP_LOGW( _TAG, "SDA (GPIO %d) still held low after %d clocks", sda, pulses );
    }
    else
    {
        ESP_LOGW( _TAG, "Released stuck SDA (GPIO %d) after %d clocks", sda, pulses );
    }
}

/* A reset in the middle of a read can leave a target driving SDA low, which
 * makes the first transfer after boot time out. Clear it before the
 * controller takes the pins. */
static void _i2c_bus_clear( gpio_num_t sda, gpio_num_t scl )
{
    gpio_config_t io_cfg = {
        .pin_bit_mask = ( 1ULL << sda ) | ( 1ULL << scl ),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    /* Release both lines before enabling the outputs so no START is driven. */
    gpio_set_level( sda, 1 );
    gpio_set_level( scl, 1 );
    if( gpio_config( &io_cfg ) != ESP_OK )
    {
        return;
    }
    esp_rom_delay_us( I2C_BUS_CLEAR_HALF_US );

    if( gpio_get_level( sda ) == 0 )
    {
        _i2c_bus_clock_out( sda, scl );
    }

    /* Drop the output reservation so the I2C driver can claim the pins. */
    gpio_reset_pin( sda );
    gpio_reset_pin( scl );
}

esp_err_t core2foraws_i2c_init( core2foraws_i2c_port_t port )
{
    if( port >= CORE2FORAWS_I2C_PORT_MAX )
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = _i2c_mutex_ensure( port );
    if( err != ESP_OK )
    {
        return err;
    }

    err = core2foraws_i2c_lock( port );
    if( err != ESP_OK )
    {
        return err;
    }

    core2foraws_i2c_bus_state_t *state = &_bus_state[ port ];
    if( state->handle != NULL )
    {
        core2foraws_i2c_unlock( port );
        return ESP_OK;
    }

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = _i2c_port[ port ],
        .sda_io_num = _sda_pin[ port ],
        .scl_io_num = _scl_pin[ port ],
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    _i2c_bus_clear( _sda_pin[ port ], _scl_pin[ port ] );
    err = i2c_new_master_bus( &bus_cfg, &state->handle );
    if( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to create I2C master bus on port %d: 0x%x",
                  port, err );
        state->handle = NULL;
    }
    else
    {
        ESP_LOGI( _TAG, "I2C master bus initialized on port %d (SDA=%d, SCL=%d)",
                  port, _sda_pin[ port ], _scl_pin[ port ] );
    }

    core2foraws_i2c_unlock( port );
    return err;
}

esp_err_t core2foraws_i2c_deinit( core2foraws_i2c_port_t port )
{
    if( port >= CORE2FORAWS_I2C_PORT_MAX )
    {
        return ESP_ERR_INVALID_ARG;
    }

    if( port == CORE2FORAWS_I2C_INTERNAL )
    {
        return ESP_ERR_NOT_SUPPORTED;
    }

    esp_err_t err = _i2c_mutex_ensure( port );
    if( err != ESP_OK )
    {
        return err;
    }

    err = core2foraws_i2c_lock( port );
    if( err != ESP_OK )
    {
        return err;
    }

    core2foraws_i2c_bus_state_t *state = &_bus_state[ port ];
    while( state->devices != NULL )
    {
        core2foraws_i2c_device_node_t *node = state->devices;
        esp_err_t remove_err = i2c_master_bus_rm_device( node->handle );
        if( remove_err != ESP_OK )
        {
            core2foraws_i2c_unlock( port );
            return remove_err;
        }
        state->devices = node->next;
        free( node );
    }

    if( state->handle != NULL )
    {
        esp_err_t delete_err = i2c_del_master_bus( state->handle );
        if( err == ESP_OK )
        {
            err = delete_err;
        }
        if( delete_err == ESP_OK )
        {
            state->handle = NULL;
        }
    }

    core2foraws_i2c_unlock( port );

    return err;
}

esp_err_t core2foraws_i2c_get_bus_handle( core2foraws_i2c_port_t port,
                                          i2c_master_bus_handle_t *handle )
{
    if( port >= CORE2FORAWS_I2C_PORT_MAX || handle == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    if( atomic_load( &_bus_state[ port ].mutex_state ) != I2C_MUTEX_READY )
    {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = core2foraws_i2c_lock( port );
    if( err != ESP_OK )
    {
        return err;
    }

    *handle = _bus_state[ port ].handle;
    err = *handle != NULL ? ESP_OK : ESP_ERR_INVALID_STATE;
    core2foraws_i2c_unlock( port );
    return err;
}

esp_err_t core2foraws_i2c_device_add( core2foraws_i2c_port_t port,
                                      uint16_t dev_addr,
                                      uint32_t scl_speed_hz,
                                      i2c_master_dev_handle_t *dev_handle )
{
    if( port >= CORE2FORAWS_I2C_PORT_MAX || dev_addr > 0x7f ||
        scl_speed_hz == 0 || dev_handle == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    if( atomic_load( &_bus_state[ port ].mutex_state ) != I2C_MUTEX_READY )
    {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = core2foraws_i2c_lock( port );
    if( err != ESP_OK )
    {
        return err;
    }

    core2foraws_i2c_bus_state_t *state = &_bus_state[ port ];
    if( state->handle == NULL )
    {
        core2foraws_i2c_unlock( port );
        return ESP_ERR_INVALID_STATE;
    }

    for( core2foraws_i2c_device_node_t *node = state->devices;
         node != NULL; node = node->next )
    {
        if( node->address == dev_addr && node->speed_hz == scl_speed_hz )
        {
            node->references++;
            *dev_handle = node->handle;
            core2foraws_i2c_unlock( port );
            return ESP_OK;
        }
    }

    core2foraws_i2c_device_node_t *node = calloc( 1, sizeof( *node ) );
    if( node == NULL )
    {
        core2foraws_i2c_unlock( port );
        return ESP_ERR_NO_MEM;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev_addr,
        .scl_speed_hz = scl_speed_hz,
    };

    err = i2c_master_bus_add_device( state->handle, &dev_cfg, dev_handle );
    if( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to add device 0x%02x on port %d: 0x%x",
                  dev_addr, port, err );
    }
    else
    {
        node->handle = *dev_handle;
        node->address = dev_addr;
        node->speed_hz = scl_speed_hz;
        node->references = 1;
        node->next = state->devices;
        state->devices = node;
        /* One line per device (registration is rare), so this is safe to
           leave on without flooding the console. */
        ESP_LOGV( _TAG, "Added device 0x%02x on port %d at %lu Hz",
                  dev_addr, port, ( unsigned long ) scl_speed_hz );
    }

    if( err != ESP_OK )
    {
        free( node );
    }

    core2foraws_i2c_unlock( port );

    return err;
}

esp_err_t core2foraws_i2c_device_remove( i2c_master_dev_handle_t dev_handle )
{
    if( dev_handle == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    for( core2foraws_i2c_port_t port = CORE2FORAWS_I2C_INTERNAL;
         port < CORE2FORAWS_I2C_PORT_MAX; port++ )
    {
        if( atomic_load( &_bus_state[ port ].mutex_state ) != I2C_MUTEX_READY )
        {
            continue;
        }

        esp_err_t err = core2foraws_i2c_lock( port );
        if( err != ESP_OK )
        {
            return err;
        }

        core2foraws_i2c_bus_state_t *state = &_bus_state[ port ];
        core2foraws_i2c_device_node_t *previous = NULL;
        core2foraws_i2c_device_node_t *node =
            _i2c_device_find_by_handle( state, dev_handle, &previous );
        if( node != NULL )
        {
            if( node->references > 1 )
            {
                node->references--;
                core2foraws_i2c_unlock( port );
                return ESP_OK;
            }

            err = i2c_master_bus_rm_device( node->handle );
            if( err == ESP_OK )
            {
                if( previous == NULL )
                {
                    state->devices = node->next;
                }
                else
                {
                    previous->next = node->next;
                }
                free( node );
            }
            core2foraws_i2c_unlock( port );
            return err;
        }

        core2foraws_i2c_unlock( port );
    }

    return ESP_ERR_NOT_FOUND;
}

esp_err_t core2foraws_i2c_read( core2foraws_i2c_port_t port,
                                i2c_master_dev_handle_t dev_handle,
                                uint32_t reg,
                                uint8_t *buffer,
                                uint16_t size )
{
    if( port >= CORE2FORAWS_I2C_PORT_MAX || dev_handle == NULL ||
        buffer == NULL || size == 0 )
    {
        return ESP_ERR_INVALID_ARG;
    }

    if( atomic_load( &_bus_state[ port ].mutex_state ) != I2C_MUTEX_READY )
    {
        return ESP_ERR_INVALID_STATE;
    }

    if( core2foraws_i2c_lock( port ) != ESP_OK )
    {
        ESP_LOGE( _TAG, "I2C read: mutex timeout on port %d", port );
        return ESP_ERR_TIMEOUT;
    }

    const core2foraws_i2c_device_node_t *node = _bus_state[ port ].handle == NULL
        ? NULL
        : _i2c_device_find_by_handle( &_bus_state[ port ], dev_handle, NULL );
    if( node == NULL )
    {
        core2foraws_i2c_unlock( port );
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err;

    if( reg & CORE2FORAWS_I2C_NO_REG )
    {
        /* No register address — direct read */
        err = i2c_master_receive( dev_handle, buffer, size,
                                  I2C_XFER_TIMEOUT_MS );
    }
    else
    {
        /* Write register address, then read (atomic with repeated start) */
        uint8_t reg_buf[ 2 ];
        uint8_t reg_len;

        if( reg & CORE2FORAWS_I2C_REG_16 )
        {
            reg_buf[ 0 ] = ( reg >> 8 ) & 0xFF;
            reg_buf[ 1 ] = reg & 0xFF;
            reg_len = 2;
        }
        else
        {
            reg_buf[ 0 ] = reg & 0xFF;
            reg_len = 1;
        }

        err = i2c_master_transmit_receive( dev_handle, reg_buf, reg_len,
                                           buffer, size,
                                           I2C_XFER_TIMEOUT_MS );
    }

    _i2c_transfer_note( port, node, err );
    core2foraws_i2c_unlock( port );
    return err;
}

esp_err_t core2foraws_i2c_write( core2foraws_i2c_port_t port,
                                 i2c_master_dev_handle_t dev_handle,
                                 uint32_t reg,
                                 const uint8_t *buffer,
                                 uint16_t size )
{
    if( port >= CORE2FORAWS_I2C_PORT_MAX || dev_handle == NULL ||
        ( size > 0 && buffer == NULL ) ||
        ( ( reg & CORE2FORAWS_I2C_NO_REG ) && size == 0 ) )
    {
        return ESP_ERR_INVALID_ARG;
    }

    if( atomic_load( &_bus_state[ port ].mutex_state ) != I2C_MUTEX_READY )
    {
        return ESP_ERR_INVALID_STATE;
    }

    if( core2foraws_i2c_lock( port ) != ESP_OK )
    {
        ESP_LOGE( _TAG, "I2C write: mutex timeout on port %d", port );
        return ESP_ERR_TIMEOUT;
    }

    const core2foraws_i2c_device_node_t *node = _bus_state[ port ].handle == NULL
        ? NULL
        : _i2c_device_find_by_handle( &_bus_state[ port ], dev_handle, NULL );
    if( node == NULL )
    {
        core2foraws_i2c_unlock( port );
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err;

    if( reg & CORE2FORAWS_I2C_NO_REG )
    {
        /* No register address — direct write */
        err = i2c_master_transmit( dev_handle, buffer, size,
                                   I2C_XFER_TIMEOUT_MS );
    }
    else
    {
        /* Prepend register address to data */
        uint8_t reg_len;
        if( reg & CORE2FORAWS_I2C_REG_16 )
        {
            reg_len = 2;
        }
        else
        {
            reg_len = 1;
        }

        /* Most register writes are a few bytes, so serve them from a stack
           buffer and avoid heap churn on this hot path. Fall back to a
           heap allocation only for unusually large payloads. */
        uint8_t stack_buf[ I2C_WRITE_STACK_BUF_SIZE ];
        uint8_t *tx_buf;
        bool tx_buf_heap = false;

        if( (size_t)reg_len + size <= sizeof( stack_buf ) )
        {
            tx_buf = stack_buf;
        }
        else
        {
            tx_buf = malloc( reg_len + size );
            if( tx_buf == NULL )
            {
                core2foraws_i2c_unlock( port );
                return ESP_ERR_NO_MEM;
            }
            tx_buf_heap = true;
        }

        if( reg_len == 2 )
        {
            tx_buf[ 0 ] = ( reg >> 8 ) & 0xFF;
            tx_buf[ 1 ] = reg & 0xFF;
        }
        else
        {
            tx_buf[ 0 ] = reg & 0xFF;
        }

        if( size > 0 )
        {
            memcpy( tx_buf + reg_len, buffer, size );
        }

        err = i2c_master_transmit( dev_handle, tx_buf, reg_len + size,
                                   I2C_XFER_TIMEOUT_MS );
        if( tx_buf_heap )
        {
            free( tx_buf );
        }
    }

    _i2c_transfer_note( port, node, err );
    core2foraws_i2c_unlock( port );
    return err;
}

esp_err_t core2foraws_i2c_lock_timeout( core2foraws_i2c_port_t port,
                                       uint32_t timeout_ms )
{
    if( port >= CORE2FORAWS_I2C_PORT_MAX ||
        atomic_load( &_bus_state[ port ].mutex_state ) != I2C_MUTEX_READY )
    {
        return ESP_ERR_INVALID_ARG;
    }

    /* Round up so a short wait is not truncated to a tick that expires
     * almost immediately; 0 stays non-blocking. */
    TickType_t ticks = timeout_ms == 0 ? 0
                                       : CORE2FORAWS_DELAY_MS_TO_TICKS( timeout_ms );
    return xSemaphoreTakeRecursive( _bus_state[ port ].mutex, ticks ) == pdTRUE
               ? ESP_OK
               : ESP_ERR_TIMEOUT;
}

esp_err_t core2foraws_i2c_lock( core2foraws_i2c_port_t port )
{
    if( port >= CORE2FORAWS_I2C_PORT_MAX ||
        atomic_load( &_bus_state[ port ].mutex_state ) != I2C_MUTEX_READY )
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = xSemaphoreTakeRecursive( _bus_state[ port ].mutex,
                                             pdMS_TO_TICKS( I2C_LOCK_TIMEOUT_MS ) ) == pdTRUE
                        ? ESP_OK
                        : ESP_ERR_TIMEOUT;
    if( err == ESP_ERR_TIMEOUT )
    {
        TaskHandle_t holder = xSemaphoreGetMutexHolder( _bus_state[ port ].mutex );
        ESP_LOGE( _TAG,
              "I2C lock timeout on port %d (requester=%s, holder=%s, state=%d, priority=%u)",
                  port, pcTaskGetName( NULL ),
                  holder != NULL ? pcTaskGetName( holder ) : "none",
                  holder != NULL ? ( int )eTaskGetState( holder ) : -1,
                  holder != NULL ? ( unsigned )uxTaskPriorityGet( holder ) : 0 );
    }

    return err;
}

esp_err_t core2foraws_i2c_unlock( core2foraws_i2c_port_t port )
{
    if( port >= CORE2FORAWS_I2C_PORT_MAX ||
        atomic_load( &_bus_state[ port ].mutex_state ) != I2C_MUTEX_READY )
    {
        return ESP_ERR_INVALID_ARG;
    }

    return xSemaphoreGiveRecursive( _bus_state[ port ].mutex ) == pdTRUE
               ? ESP_OK
               : ESP_ERR_INVALID_STATE;
}
