/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_display.c
 * @brief Core2 for AWS IoT Kit display driver using esp_lcd + esp_lcd_touch + esp_lvgl_port
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#include <sdkconfig.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <driver/spi_master.h>
#include <esp_check.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_io_interface.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_ili9341.h>
#include <esp_lcd_touch_ft5x06.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <esp_lvgl_port.h>
#include <freertos/FreeRTOS.h>

#include "core2foraws_common.h"
#include "core2foraws_board_priv.h"
#include "core2foraws_power.h"
#include "core2foraws_display.h"

/* ── Hardware constants (from schema.yml) ── */

/* SPI bus shared with SD card (HSPI / SPI2) */
#define LCD_SPI_HOST        SPI2_HOST

/* ILI9342C or ILI9342E display (ILI9341-compatible level 1 commands) */
#define LCD_H_RES           320
#define LCD_V_RES           240
#define LCD_SPI_MOSI        GPIO_NUM_23
#define LCD_SPI_MISO        GPIO_NUM_38
#define LCD_SPI_SCLK        GPIO_NUM_18
#define LCD_SPI_CS          GPIO_NUM_5
#define LCD_DC              GPIO_NUM_15
#define LCD_PIXEL_CLK_HZ    ( 40 * 1000 * 1000 )
#define LCD_CMD_BITS        8
#define LCD_PARAM_BITS      8
/* Register reads are far slower than writes on both controllers. */
#define LCD_PROBE_CLK_HZ    ( 4 * 1000 * 1000 )
#define LCD_PROBE_ATTEMPTS  5

/* ILI9342E level 2 commands (ILI9342E datasheet section 5.3) */
#define ILI9342E_CMD_GET_EXT_REG   0xD9
#define ILI9342E_CMD_SET_EXTC      0xDD
#define ILI9342E_CMD_POWER_CTRL_6  0xCB
#define ILI9342E_EXT_READ_EN       0x10
/* ILI9342C level 2 commands (ILI9342C datasheet sections 8.3.24 and 8.3.28) */
#define ILI9342C_CMD_SET_EXTC      0xC8
#define ILI9342C_CMD_READ_ID4      0xD3

#ifdef CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES
#define LCD_DRAW_BUF_LINES  CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES
#else
#define LCD_DRAW_BUF_LINES  20
#endif

/* 2 bytes per pixel (RGB565). */
#define LCD_DRAW_BUF_BYTES  ( LCD_H_RES * LCD_DRAW_BUF_LINES * 2 )

/* Both draw buffers are flushed through the shared SPI2 bus, so a single
 * buffer must fit inside the bus's configured max_transfer_sz. */
_Static_assert( LCD_DRAW_BUF_BYTES <= CORE2FORAWS_SPI_MAX_TRANSFER_BYTES,
                "LCD draw buffer exceeds the shared SPI max transfer size" );

/* FT6336U touch controller on internal I2C bus */
#define TOUCH_INT_GPIO      GPIO_NUM_39
#define TOUCH_I2C_XFER_TIMEOUT_MS 100
/* The LVGL task skips a touch sample rather than stall behind another bus
 * user; the next input period retries. Other callers use the normal lock. */
#define TOUCH_I2C_LOCK_TIMEOUT_MS 10
/* Faster than LVGL's 33 ms default so drags track the finger closely. */
#define TOUCH_READ_PERIOD_MS 16
/* LVGL (16 ms) and the button task (20 ms) both poll touch. A sample this
 * fresh is shared instead of re-read, reducing FT6336 transfers. */
#define TOUCH_SAMPLE_REUSE_US ( 15 * 1000 )
/* FT6336U reports at most two simultaneous points (datasheet section 14.1) */
#define TOUCH_MAX_POINTS    2

static const char *_TAG = "CORE2FORAWS_DISPLAY";

/* Handles exposed to consumers */
lv_display_t *core2foraws_display_ptr = NULL;

/* Private handles */
static esp_lcd_panel_io_handle_t _io_handle = NULL;
static esp_lcd_panel_io_handle_t _touch_io_handle = NULL;
static esp_lcd_panel_handle_t    _panel_handle = NULL;
static esp_lcd_touch_handle_t    _touch_handle = NULL;
static lv_indev_t               *_touch_indev = NULL;
static bool                      _lvgl_initialized = false;
static atomic_bool               _flush_holds_spi_lock;
static atomic_bool               _touch_interrupt_pending;
static atomic_bool               _touch_contact_active;

static portMUX_TYPE                _touch_sample_lock = portMUX_INITIALIZER_UNLOCKED;
static esp_lcd_touch_point_data_t  _touch_sample[ TOUCH_MAX_POINTS ];
static uint8_t                     _touch_sample_count;
static int64_t                     _touch_sample_us; /* 0: no sample yet */

typedef struct
{
    esp_lcd_panel_io_t base;
    i2c_master_dev_handle_t device;
} core2foraws_touch_io_t;

static esp_err_t _touch_io_rx( esp_lcd_panel_io_t *io, int command,
                               void *parameters, size_t parameter_size )
{
    if( io == NULL || command < 0 || command > UINT8_MAX ||
        parameters == NULL || parameter_size == 0 )
    {
        return ESP_ERR_INVALID_ARG;
    }

    core2foraws_touch_io_t *touch_io = ( core2foraws_touch_io_t * )io;
    uint8_t register_address = ( uint8_t )command;
    esp_err_t err = core2foraws_i2c_lock( COMMON_I2C_INTERNAL );
    if( err != ESP_OK ) return err;

    err = i2c_master_transmit_receive( touch_io->device,
        &register_address, sizeof( register_address ), parameters,
        parameter_size, TOUCH_I2C_XFER_TIMEOUT_MS );
    esp_err_t unlock_err = core2foraws_i2c_unlock( COMMON_I2C_INTERNAL );
    return err != ESP_OK ? err : unlock_err;
}

static esp_err_t _touch_io_tx( esp_lcd_panel_io_t *io, int command,
                               const void *parameters, size_t parameter_size )
{
    if( io == NULL || command < 0 || command > UINT8_MAX ||
        parameter_size > UINT16_MAX ||
        ( parameter_size > 0 && parameters == NULL ) )
    {
        return ESP_ERR_INVALID_ARG;
    }

    core2foraws_touch_io_t *touch_io = ( core2foraws_touch_io_t * )io;
    return core2foraws_i2c_write( COMMON_I2C_INTERNAL, touch_io->device,
                                  ( uint8_t )command, parameters,
                                  ( uint16_t )parameter_size );
}

static esp_err_t _touch_io_tx_color( esp_lcd_panel_io_t *io, int command,
                                     const void *color, size_t color_size )
{
    ( void )io;
    ( void )command;
    ( void )color;
    ( void )color_size;
    return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t _touch_io_register_callbacks(
    esp_lcd_panel_io_handle_t io,
    const esp_lcd_panel_io_callbacks_t *callbacks, void *user_context )
{
    ( void )io;
    ( void )callbacks;
    ( void )user_context;
    return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t _touch_io_del( esp_lcd_panel_io_t *io )
{
    if( io == NULL ) return ESP_ERR_INVALID_ARG;
    core2foraws_touch_io_t *touch_io = ( core2foraws_touch_io_t * )io;
    esp_err_t err = core2foraws_i2c_device_remove( touch_io->device );
    if( err == ESP_OK ) free( touch_io );
    return err;
}

static esp_err_t _touch_io_new(
    const esp_lcd_panel_io_i2c_config_t *config,
    esp_lcd_panel_io_handle_t *io_handle )
{
    if( config == NULL || io_handle == NULL ) return ESP_ERR_INVALID_ARG;

    core2foraws_touch_io_t *touch_io = calloc( 1, sizeof( *touch_io ) );
    if( touch_io == NULL ) return ESP_ERR_NO_MEM;

    esp_err_t err = core2foraws_i2c_device_add( COMMON_I2C_INTERNAL,
        config->dev_addr, config->scl_speed_hz, &touch_io->device );
    if( err != ESP_OK )
    {
        free( touch_io );
        return err;
    }

    touch_io->base.rx_param = _touch_io_rx;
    touch_io->base.tx_param = _touch_io_tx;
    touch_io->base.tx_color = _touch_io_tx_color;
    touch_io->base.del = _touch_io_del;
    touch_io->base.register_event_callbacks = _touch_io_register_callbacks;
    *io_handle = &touch_io->base;
    return ESP_OK;
}

/*
 * esp_lcd releases the SPI bus lock when it *queues* the colour transfer, not
 * when the DMA completes, and leaves CS asserted from the command phase
 * (esp_lcd_panel_io_spi.c: acquire -> RAMWR with SPI_TRANS_CS_KEEP_ACTIVE ->
 * queue_trans -> release). This semaphore covers that window so an SD
 * transaction cannot assert its own CS while the panel is still selected,
 * which is why it is taken here and released in the completion ISR.
 */
static void _display_flush( lv_display_t *display, const lv_area_t *area,
                            uint8_t *color_map )
{
    if( core2foraws_common_spi_semaphore == NULL ||
        xSemaphoreTake( core2foraws_common_spi_semaphore,
                        pdMS_TO_TICKS( CORE2FORAWS_SPI_LOCK_TIMEOUT_MS ) ) !=
        pdTRUE )
    {
        ESP_LOGE( _TAG, "Shared SPI unavailable; skipping display transfer" );
        lvgl_port_flush_ready( display );
        return;
    }

    atomic_store( &_flush_holds_spi_lock, true );
    esp_err_t err = esp_lcd_panel_draw_bitmap( _panel_handle,
        area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map );
    if( err != ESP_OK )
    {
        if( atomic_exchange( &_flush_holds_spi_lock, false ) )
        {
            xSemaphoreGive( core2foraws_common_spi_semaphore );
        }
        ESP_LOGE( _TAG, "Display transfer failed: 0x%x", err );
        lvgl_port_flush_ready( display );
    }
}

/**
 * @brief Verify enough contiguous DMA-capable DRAM exists for both draw
 * buffers before esp_lvgl_port tries to allocate them.
 *
 * esp_lvgl_port only reports a NULL display on failure, which gives the user
 * nothing to act on. Contiguity is what actually fails here, not total free
 * heap, so check the largest free block explicitly.
 */
static esp_err_t _display_draw_buffer_check( void )
{
    const size_t needed = LCD_DRAW_BUF_BYTES;
    size_t largest = heap_caps_get_largest_free_block( MALLOC_CAP_DMA );
    size_t available = heap_caps_get_free_size( MALLOC_CAP_DMA );

    if( largest >= needed && available >= needed * 2 )
    {
        ESP_LOGD( _TAG,
                  "Draw buffers need 2 x %u bytes; %u free, largest block %u",
                  ( unsigned int ) needed,
                  ( unsigned int ) available, ( unsigned int ) largest );
        return ESP_OK;
    }

    ESP_LOGE( _TAG,
              "Not enough DMA-capable DRAM for the LVGL draw buffers. "
              "Need 2 contiguous blocks of %u bytes (%u total) at "
              "CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES=%d, but only %u bytes "
              "are free with a largest block of %u bytes.",
              ( unsigned int ) needed, ( unsigned int ) ( needed * 2 ),
              LCD_DRAW_BUF_LINES, ( unsigned int ) available,
              ( unsigned int ) largest );
    ESP_LOGE( _TAG,
              "Lower CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES, or free internal "
              "DRAM with CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP=y and "
              "CONFIG_ESP32_WIFI_TX_BUFFER=dynamic. PSRAM cannot hold these "
              "buffers; it is not DMA-addressable." );

    return ESP_ERR_NO_MEM;
}

static bool _display_flush_done( esp_lcd_panel_io_handle_t panel_io,
                                 esp_lcd_panel_io_event_data_t *event_data,
                                 void *user_ctx )
{
    (void)panel_io;
    (void)event_data;

    BaseType_t task_woken = pdFALSE;

    /* Release only what this flush actually acquired, so a take that timed out
       cannot hand a free count to an unrelated holder. */
    if( atomic_exchange( &_flush_holds_spi_lock, false ) )
    {
        xSemaphoreGiveFromISR( core2foraws_common_spi_semaphore, &task_woken );
    }
    lvgl_port_flush_ready( ( lv_display_t * )user_ctx );
    return task_woken == pdTRUE;
}

/* Copies the cached sample out. Call with _touch_sample_lock held. */
static uint8_t _touch_sample_copy( esp_lcd_touch_point_data_t *points,
                                   uint8_t max_points )
{
    uint8_t count = _touch_sample_count < max_points ? _touch_sample_count
                                                     : max_points;
    memcpy( points, _touch_sample, count * sizeof( *points ) );
    return count;
}

/* Returns true and fills the outputs when the cached sample is fresh. */
static bool _touch_sample_reuse( esp_lcd_touch_point_data_t *points,
                                 uint8_t *point_count, uint8_t max_points )
{
    bool fresh = false;
    int64_t now_us = esp_timer_get_time();
    taskENTER_CRITICAL( &_touch_sample_lock );
    if( _touch_sample_us != 0 &&
        now_us - _touch_sample_us < TOUCH_SAMPLE_REUSE_US )
    {
        *point_count = _touch_sample_copy( points, max_points );
        fresh = true;
    }
    taskEXIT_CRITICAL( &_touch_sample_lock );
    return fresh;
}

static esp_err_t _display_touch_read( esp_lcd_touch_point_data_t *points,
                                      uint8_t *point_count,
                                      uint8_t max_points, bool bounded_wait )
{
    if( points == NULL || point_count == NULL || max_points == 0 )
    {
        return ESP_ERR_INVALID_ARG;
    }

    if( _touch_sample_reuse( points, point_count, max_points ) )
    {
        return ESP_OK;
    }

    esp_err_t err = bounded_wait
        ? core2foraws_i2c_lock_timeout( COMMON_I2C_INTERNAL,
                                        TOUCH_I2C_LOCK_TIMEOUT_MS )
        : core2foraws_i2c_lock( COMMON_I2C_INTERNAL );
    if( err != ESP_OK )
    {
        return err;
    }

    if( _touch_handle == NULL )
    {
        esp_err_t unlock_err = core2foraws_i2c_unlock( COMMON_I2C_INTERNAL );
        return unlock_err != ESP_OK ? unlock_err : ESP_ERR_INVALID_STATE;
    }

    /* Another poller may have refreshed the sample while this one waited. */
    if( _touch_sample_reuse( points, point_count, max_points ) )
    {
        return core2foraws_i2c_unlock( COMMON_I2C_INTERNAL );
    }

    esp_lcd_touch_point_data_t fresh[ TOUCH_MAX_POINTS ];
    uint8_t fresh_count = 0;
    err = esp_lcd_touch_read_data( _touch_handle );
    if( err == ESP_OK )
    {
        err = esp_lcd_touch_get_data( _touch_handle, fresh, &fresh_count,
                                      TOUCH_MAX_POINTS );
    }
    if( err == ESP_OK )
    {
        if( fresh_count > TOUCH_MAX_POINTS )
        {
            fresh_count = TOUCH_MAX_POINTS;
        }
        taskENTER_CRITICAL( &_touch_sample_lock );
        memcpy( _touch_sample, fresh, fresh_count * sizeof( fresh[ 0 ] ) );
        _touch_sample_count = fresh_count;
        _touch_sample_us = esp_timer_get_time();
        *point_count = _touch_sample_copy( points, max_points );
        taskEXIT_CRITICAL( &_touch_sample_lock );
    }

    esp_err_t unlock_err = core2foraws_i2c_unlock( COMMON_I2C_INTERNAL );
    return err != ESP_OK ? err : unlock_err;
}

static void _touch_interrupt( esp_lcd_touch_handle_t touch_handle )
{
    ( void )touch_handle;
    atomic_store_explicit( &_touch_interrupt_pending, true,
                           memory_order_release );
}

static void _lvgl_touch_read( lv_indev_t *indev, lv_indev_data_t *data )
{
    (void)indev;
    esp_lcd_touch_point_data_t point;
    uint8_t point_count = 0;

    data->state = LV_INDEV_STATE_RELEASED;
    bool interrupt_pending = atomic_exchange_explicit(
        &_touch_interrupt_pending, false, memory_order_acq_rel );
    if( !interrupt_pending &&
        !atomic_load_explicit( &_touch_contact_active,
                               memory_order_acquire ) &&
        gpio_get_level( TOUCH_INT_GPIO ) != 0 )
    {
        return;
    }

    esp_err_t err = _display_touch_read( &point, &point_count, 1, true );
    if( err == ESP_ERR_TIMEOUT )
    {
        /* Bus busy: repeat the last known state so a held touch is not
         * reported as a release, and retry on the next input period. */
        taskENTER_CRITICAL( &_touch_sample_lock );
        point_count = _touch_sample_copy( &point, 1 );
        taskEXIT_CRITICAL( &_touch_sample_lock );
        atomic_store( &_touch_contact_active, true );
        if( point_count > 0 )
        {
            data->point.x = point.x;
            data->point.y = point.y;
            data->state = LV_INDEV_STATE_PRESSED;
        }
        return;
    }
    if( err != ESP_OK )
    {
        atomic_store( &_touch_contact_active, true );
        return;
    }

    bool contact_active = point_count > 0;
    atomic_store( &_touch_contact_active, contact_active );
    if( contact_active )
    {
        data->point.x = point.x;
        data->point.y = point.y;
        data->state = LV_INDEV_STATE_PRESSED;
    }
}

static esp_err_t _display_cleanup( void )
{
    esp_err_t cleanup_err = ESP_OK;

    if( _lvgl_initialized )
    {
        lvgl_port_stop();
    }

    /* Refresh is stopped, so no new flush can start. Taking and immediately
     * releasing the semaphore waits for an in-flight transfer to finish before
     * the display context its callback references is freed. It is not held
     * across teardown: LVGL teardown can re-enter the flush path, and this
     * binary semaphore has no owner tracking to make that re-entry safe. */
    if( core2foraws_common_spi_semaphore != NULL )
    {
        if( xSemaphoreTake( core2foraws_common_spi_semaphore,
                           pdMS_TO_TICKS( CORE2FORAWS_SPI_LOCK_TIMEOUT_MS ) ) !=
            pdTRUE )
        {
            ESP_LOGE( _TAG, "Display teardown timed out waiting for SPI; resources retained" );
            return ESP_ERR_TIMEOUT;
        }
        xSemaphoreGive( core2foraws_common_spi_semaphore );
    }

    if( _touch_indev != NULL )
    {
        lvgl_port_lock( 0 );
        lv_indev_delete( _touch_indev );
        lvgl_port_unlock();
        _touch_indev = NULL;
    }

    if( core2foraws_display_ptr != NULL )
    {
        lvgl_port_remove_disp( core2foraws_display_ptr );
        core2foraws_display_ptr = NULL;
    }

    if( _lvgl_initialized )
    {
        lvgl_port_deinit();
        _lvgl_initialized = false;
    }

    if( _touch_handle != NULL || _touch_io_handle != NULL )
    {
        esp_err_t lock_err = core2foraws_i2c_lock( COMMON_I2C_INTERNAL );
        if( lock_err != ESP_OK )
        {
            ESP_LOGE( _TAG, "Failed to lock internal I2C for touch teardown: 0x%x",
                      lock_err );
            cleanup_err = lock_err;
        }
        else
        {
            if( _touch_handle != NULL )
            {
                esp_err_t err = esp_lcd_touch_del( _touch_handle );
                if( err == ESP_OK )
                {
                    _touch_handle = NULL;
                }
                else if( cleanup_err == ESP_OK )
                {
                    cleanup_err = err;
                }
            }
            if( _touch_handle == NULL && _touch_io_handle != NULL )
            {
                esp_err_t err = esp_lcd_panel_io_del( _touch_io_handle );
                if( err == ESP_OK )
                {
                    _touch_io_handle = NULL;
                }
                else if( cleanup_err == ESP_OK )
                {
                    cleanup_err = err;
                }
            }

            esp_err_t unlock_err = core2foraws_i2c_unlock( COMMON_I2C_INTERNAL );
            if( cleanup_err == ESP_OK )
            {
                cleanup_err = unlock_err;
            }
        }
    }
    if( _panel_handle != NULL )
    {
        esp_lcd_panel_del( _panel_handle );
        _panel_handle = NULL;
    }
    if( _io_handle != NULL )
    {
        esp_lcd_panel_io_del( _io_handle );
        _io_handle = NULL;
    }

    return cleanup_err;
}

esp_err_t core2foraws_display_touch_data_get(
    esp_lcd_touch_point_data_t *points, uint8_t *point_count,
    uint8_t max_points )
{
    if( points == NULL || point_count == NULL || max_points == 0 )
    {
        return ESP_ERR_INVALID_ARG;
    }

    *point_count = 0;
    if( !atomic_load_explicit( &_touch_interrupt_pending,
                               memory_order_acquire ) &&
        !atomic_load_explicit( &_touch_contact_active,
                               memory_order_acquire ) &&
        gpio_get_level( TOUCH_INT_GPIO ) != 0 )
    {
        return ESP_OK;
    }

    return _display_touch_read( points, point_count, max_points, false );
}


/**
 * @brief Read one level 2 command parameter through Get External Register
 * (D9h), which both controllers implement.
 *
 * Returns true when @p expected is read back. Two bytes are clocked in so a
 * one-bit dummy cycle before the data is also accepted.
 */
static bool _lcd_ext_reg_matches( esp_lcd_panel_io_handle_t io, uint8_t cmd,
                                  uint8_t index, uint8_t expected,
                                  uint16_t *raw )
{
    uint8_t rx[ 2 ] = { 0 };
    *raw = 0;
    if( esp_lcd_panel_io_tx_param( io, ILI9342E_CMD_GET_EXT_REG,
            ( uint8_t[] ){ ILI9342E_EXT_READ_EN | index }, 1 ) != ESP_OK ||
        esp_lcd_panel_io_rx_param( io, cmd, rx, sizeof( rx ) ) != ESP_OK )
    {
        return false;
    }
    *raw = ( uint16_t )( ( rx[ 0 ] << 8 ) | rx[ 1 ] );
    return rx[ 0 ] == expected || ( uint8_t )( *raw >> 7 ) == expected;
}

/**
 * @brief Tell the ILI9342E from the ILI9342C.
 *
 * The two share level 1 commands but not level 2 ones, so each needs its own
 * init list. The E stores and reads back Set EXTC (DDh) and Power Control 6
 * (CBh); the C has no DDh and leaves level 2 commands as NOPs until its own
 * EXTC key is sent, after which Read ID4 (D3h) returns 93h 42h. The C key is
 * only sent once the E keys failed: on the E, C8h is the VCOM setting. The
 * panel is software reset after this, so no probe write persists.
 */
static core2foraws_board_lcd_t _lcd_controller_detect( void )
{
    const esp_lcd_panel_io_spi_config_t probe_config = {
        .dc_gpio_num       = LCD_DC,
        .cs_gpio_num       = LCD_SPI_CS,
        .pclk_hz           = LCD_PROBE_CLK_HZ,
        .lcd_cmd_bits      = LCD_CMD_BITS,
        .lcd_param_bits    = LCD_PARAM_BITS,
        .spi_mode          = 0,
        .trans_queue_depth = 1,
        /* The panel answers on its bidirectional SDA line (GPIO23). */
        .flags.sio_mode    = 1,
    };
    esp_lcd_panel_io_handle_t io = NULL;
    if( esp_lcd_new_panel_io_spi( ( esp_lcd_spi_bus_handle_t ) LCD_SPI_HOST,
                                  &probe_config, &io ) != ESP_OK )
    {
        ESP_LOGW( _TAG, "LCD probe unavailable; assuming ILI9342C" );
        return BOARD_LCD_ILI9342C;
    }

    core2foraws_board_lcd_t lcd = BOARD_LCD_UNKNOWN;
    uint16_t raw[ 4 ] = { 0 };
    for( int attempt = 0; attempt < LCD_PROBE_ATTEMPTS &&
                          lcd == BOARD_LCD_UNKNOWN; attempt++ )
    {
        if( attempt > 0 )
        {
            vTaskDelay( CORE2FORAWS_DELAY_MS_TO_TICKS( 2 ) );
        }

        esp_lcd_panel_io_tx_param( io, ILI9342E_CMD_SET_EXTC,
                                   ( uint8_t[] ){ 0x01 }, 1 );
        esp_lcd_panel_io_tx_param( io, ILI9342E_CMD_POWER_CTRL_6,
                                   ( uint8_t[] ){ 0x1C }, 1 );
        bool extc = _lcd_ext_reg_matches( io, ILI9342E_CMD_SET_EXTC, 1, 0x01,
                                          &raw[ 0 ] );
        bool pwr6 = _lcd_ext_reg_matches( io, ILI9342E_CMD_POWER_CTRL_6, 1,
                                          0x1C, &raw[ 1 ] );
        if( extc && pwr6 )
        {
            lcd = BOARD_LCD_ILI9342E;
            break;
        }

        esp_lcd_panel_io_tx_param( io, ILI9342C_CMD_SET_EXTC,
                                   ( uint8_t[] ){ 0xFF, 0x93, 0x42 }, 3 );
        bool id_hi = _lcd_ext_reg_matches( io, ILI9342C_CMD_READ_ID4, 2, 0x93,
                                           &raw[ 2 ] );
        bool id_lo = _lcd_ext_reg_matches( io, ILI9342C_CMD_READ_ID4, 3, 0x42,
                                           &raw[ 3 ] );
        if( id_hi && id_lo )
        {
            lcd = BOARD_LCD_ILI9342C;
        }
    }
    esp_lcd_panel_io_tx_param( io, ILI9342E_CMD_GET_EXT_REG,
                               ( uint8_t[] ){ 0x00 }, 1 );
    esp_lcd_panel_io_del( io );

    ESP_LOGI( _TAG, "\tLCD read-back DDh:%04x CBh:%04x ID4:%04x %04x -> %s",
              raw[ 0 ], raw[ 1 ], raw[ 2 ], raw[ 3 ],
              lcd == BOARD_LCD_ILI9342E ? "ILI9342E" :
              lcd == BOARD_LCD_ILI9342C ? "ILI9342C" : "unknown" );
    if( lcd == BOARD_LCD_UNKNOWN )
    {
        ESP_LOGW( _TAG, "LCD controller did not answer; assuming ILI9342C" );
        lcd = BOARD_LCD_ILI9342C;
    }
    return lcd;
}

/* ILI9342E level 2 settings. Command meanings are from the ILI9342E
 * datasheet section 5.3; the values are the panel tuning M5Stack ships
 * for the Core2 ILI9342E (M5GFX 0.2.27 and later). */
static const ili9341_lcd_init_cmd_t _ili9342e_init_cmds[] = {
    /* Set EXTC: enable level 2 commands */
    { 0xDD, ( uint8_t[] ){ 0x01 }, 1, 0 },
    /* MADCTL_EOR: no OTP inversion of the MADCTL (36h) bits */
    { 0xD5, ( uint8_t[] ){ 0x00 }, 1, 0 },
    /* Display Inversion Control */
    { 0xB1, ( uint8_t[] ){ 0x22 }, 1, 0 },
    /* Power Control 3: VCOM */
    { 0xC8, ( uint8_t[] ){ 0x38 }, 1, 0 },
    /* Power Control 6: VDV */
    { 0xCB, ( uint8_t[] ){ 0x1C }, 1, 0 },
    /* Power Control 4/5: VREG1OUT for dot and column inversion */
    { 0xC9, ( uint8_t[] ){ 0x1A }, 1, 0 },
    { 0xCA, ( uint8_t[] ){ 0x1A }, 1, 0 },
    /* Source Chopper Control */
    { 0xB7, ( uint8_t[] ){ 0x5A, 0x41, 0x11, 0x19 }, 4, 0 },
    /* Positive / negative gamma correction */
    { 0xE4, ( uint8_t[] ){ 0x04, 0x08, 0x11, 0x06, 0x12, 0x07, 0x3A, 0x76,
                           0x47, 0x07, 0x0F, 0x0A, 0x11, 0x19, 0x05 }, 15, 0 },
    { 0xE5, ( uint8_t[] ){ 0x02, 0x03, 0x07, 0x06, 0x12, 0x07, 0x36, 0x5F,
                           0x48, 0x06, 0x10, 0x0C, 0x16, 0x14, 0x09 }, 15, 0 },
};

/**
 * @brief Create the esp_lcd panel IO and panel driver for the ILI9342C/E.
 */
static esp_err_t _init_lcd_panel( void )
{
    core2foraws_board_lcd_t lcd = _lcd_controller_detect();
    core2foraws_board_lcd_set( lcd );

    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num       = LCD_DC,
        .cs_gpio_num       = LCD_SPI_CS,
        .pclk_hz           = LCD_PIXEL_CLK_HZ,
        .lcd_cmd_bits      = LCD_CMD_BITS,
        .lcd_param_bits    = LCD_PARAM_BITS,
        .spi_mode          = 0,
        .trans_queue_depth = 10,
    };

    esp_err_t err = esp_lcd_new_panel_io_spi(
        ( esp_lcd_spi_bus_handle_t ) LCD_SPI_HOST, &io_config, &_io_handle );
    if( err != ESP_OK )
    {
        return err;
    }

    const ili9341_vendor_config_t ili9342e_config = {
        .init_cmds      = _ili9342e_init_cmds,
        .init_cmds_size = sizeof( _ili9342e_init_cmds ) /
                          sizeof( _ili9342e_init_cmds[ 0 ] ),
    };
    /* The ILI9342C keeps the driver's default list, whose level 2 commands
     * it ignores without its EXTC key. */
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num  = GPIO_NUM_NC,
        .rgb_ele_order   = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel  = 16,
        .vendor_config   = lcd == BOARD_LCD_ILI9342E
                               ? ( void * )&ili9342e_config : NULL,
    };

    err = esp_lcd_new_panel_ili9341( _io_handle, &panel_config, &_panel_handle );
    if( err != ESP_OK )
    {
        return err;
    }

    ESP_RETURN_ON_ERROR( esp_lcd_panel_reset( _panel_handle ), _TAG, "panel reset" );
    ESP_RETURN_ON_ERROR( esp_lcd_panel_init( _panel_handle ), _TAG, "panel init" );
    ESP_RETURN_ON_ERROR( esp_lcd_panel_invert_color( _panel_handle, true ), _TAG, "invert color" );
    ESP_RETURN_ON_ERROR( esp_lcd_panel_disp_on_off( _panel_handle, true ), _TAG, "disp on" );

    return ESP_OK;
}

/**
 * @brief Create the esp_lcd_touch driver for the FT6336U (FT5x06-compatible).
 */
static esp_err_t _init_touch( void )
{
    esp_lcd_panel_io_i2c_config_t io_config =
        ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
    io_config.scl_speed_hz = 400000;

    esp_err_t err = _touch_io_new( &io_config, &_touch_io_handle );
    if( err != ESP_OK )
    {
        return err;
    }

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_H_RES,
        .y_max = LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = TOUCH_INT_GPIO,
        .levels = {
            .reset     = 0,
            .interrupt = 0,
        },
        .interrupt_callback = _touch_interrupt,
        .flags = {
            .swap_xy  = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    err = esp_lcd_touch_new_i2c_ft5x06( _touch_io_handle, &tp_cfg,
                                        &_touch_handle );
    if( err == ESP_OK )
    {
        atomic_store( &_touch_interrupt_pending, true );
        _touch_contact_active = false;
        taskENTER_CRITICAL( &_touch_sample_lock );
        _touch_sample_count = 0;
        _touch_sample_us = 0;
        taskEXIT_CRITICAL( &_touch_sample_lock );
    }
    return err;
}

esp_err_t core2foraws_display_get_touch_handle(
    esp_lcd_touch_handle_t *touch_handle )
{
    if( touch_handle == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    if( _touch_handle == NULL )
    {
        return ESP_ERR_INVALID_STATE;
    }

    *touch_handle = _touch_handle;
    return ESP_OK;
}

esp_err_t core2foraws_display_init( void )
{
    ESP_LOGI( _TAG, "\tInitializing" );

    if( core2foraws_display_ptr != NULL )
    {
        ESP_LOGW( _TAG, "Display already initialized" );
        return ESP_OK;
    }

    esp_err_t err = _display_draw_buffer_check();
    if( err != ESP_OK )
    {
        return err;
    }

    err = core2foraws_common_spi_bus_init();
    if( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to create shared SPI semaphore: 0x%x", err );
        return err;
    }

    /* ── 1. LCD panel via the shared SPI bus ── */
    core2foraws_power_lcd_ready_wait();
    err = _init_lcd_panel();
    if( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "LCD panel init failed: 0x%x", err );
        (void)_display_cleanup();
        return err;
    }

    /* ── 2. Touch via esp_lcd_touch ── */
    err = _init_touch();
    if( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Touch init failed: 0x%x", err );
        (void)_display_cleanup();
        return err;
    }

    /* ── 3. LVGL port ── */
    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    /* Raise the LVGL task stack from the default 7168 bytes to avoid a
     * stack overflow during canvas/image rendering. */
    lvgl_cfg.task_stack = 10240;
    /* Pin the LVGL render/flush task to core 1, away from core 0 where the
     * Wi-Fi stack and the IDF event loop run by default. This keeps the
     * DMA-driven display flush off the same core as networking, avoiding
     * scheduling contention that shows up as dropped frames. */
    lvgl_cfg.task_affinity = 1;
    err = lvgl_port_init( &lvgl_cfg );
    if( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "LVGL port init failed: 0x%x", err );
        (void)_display_cleanup();
        return err;
    }
    _lvgl_initialized = true;

    /* ── 4. Add display to LVGL port ── */
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle     = _io_handle,
        .panel_handle  = _panel_handle,
        .buffer_size   = LCD_H_RES * LCD_DRAW_BUF_LINES,
        .double_buffer = true,
        .hres          = LCD_H_RES,
        .vres          = LCD_V_RES,
        .monochrome    = false,
        /* The panel takes big-endian RGB565, so LVGL renders it directly without a flush-time swap. */
        .color_format  = LV_COLOR_FORMAT_RGB565_SWAPPED,
        .rotation = {
            .swap_xy  = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            /* Draw buffers must live in internal, DMA-capable RAM. PSRAM is
             * not DMA-addressable; a PSRAM draw buffer stalls the SPI flush
             * and causes UI hangs/crashes. Keep buff_spiram = false. See
             * .claude/rules/memory-placement.md. */
            .buff_dma    = true,
            .buff_spiram = false,
        },
    };

    lvgl_port_lock( 0 );
    core2foraws_display_ptr = lvgl_port_add_disp( &disp_cfg );
    if( core2foraws_display_ptr == NULL )
    {
        lvgl_port_unlock();
        ESP_LOGE( _TAG,
                  "Failed to add display to LVGL port. The 2 x %u byte draw "
                  "buffers could not be allocated; lower "
                  "CONFIG_CORE2FORAWS_LCD_DRAW_BUF_LINES (currently %d).",
                  ( unsigned int ) LCD_DRAW_BUF_BYTES, LCD_DRAW_BUF_LINES );
        (void)_display_cleanup();
        return ESP_ERR_NO_MEM;
    }

    const esp_lcd_panel_io_callbacks_t io_callbacks = {
        .on_color_trans_done = _display_flush_done,
    };
    err = esp_lcd_panel_io_register_event_callbacks(
        _io_handle, &io_callbacks, core2foraws_display_ptr );
    if( err != ESP_OK )
    {
        lvgl_port_unlock();
        ESP_LOGE( _TAG, "Failed to register display flush callback: 0x%x",
                  err );
        (void)_display_cleanup();
        return err;
    }

    lv_display_set_flush_cb( core2foraws_display_ptr, _display_flush );
    lvgl_port_unlock();

    /* ── 5. Add BSP-owned touch input to LVGL ── */
    lvgl_port_lock( 0 );
    _touch_indev = lv_indev_create();
    if( _touch_indev != NULL )
    {
        lv_indev_set_type( _touch_indev, LV_INDEV_TYPE_POINTER );
        lv_indev_set_display( _touch_indev, core2foraws_display_ptr );
        lv_indev_set_read_cb( _touch_indev, _lvgl_touch_read );
        lv_timer_set_period( lv_indev_get_read_timer( _touch_indev ),
                             TOUCH_READ_PERIOD_MS );
    }
    lvgl_port_unlock();
    if( _touch_indev == NULL )
    {
        ESP_LOGE( _TAG, "Failed to add touch input" );
        (void)_display_cleanup();
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI( _TAG, "\tDisplay initialized" );
    return ESP_OK;
}

esp_err_t core2foraws_display_deinit( void )
{
    return _display_cleanup();
}