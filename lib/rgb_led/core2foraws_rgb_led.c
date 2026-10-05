
/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_rgb_led.c
 * @brief Standalone SK6812 RGB LED driver for Core2 for AWS IoT Kit.
 *        Uses RMT peripheral directly — no external library dependencies.
 */

#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sys/cdefs.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/rmt_tx.h>
#include <driver/rmt_encoder.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include "core2foraws_common.h"
#include "core2foraws_rgb_led.h"

static const char *_TAG = "CORE2FORAWS_RGB_LED";

/* ── Hardware constants ─────────────────────────────────────────── */
#define SK6812_GPIO             GPIO_NUM_25
#define SK6812_BYTES_PER_LED    3          /* GRB, no white channel    */
#define SK6812_BUF_SIZE         ( RGB_LED_NUMS * SK6812_BYTES_PER_LED )
#define SK6812_FLUSH_TIMEOUT_MS 1000
#define SK6812_RESET_US         80         /* latch / reset pause (SK6812
                                              requires >= 80 us low) */
/* One RMT symbol per bit plus the latch symbol, rounded up to the 64-symbol
 * block size. Holding the whole frame avoids mid-frame refill interrupts,
 * which Wi-Fi load can delay enough to corrupt the bit timing. This uses 4
 * of the ESP32's 8 RMT memory blocks. */
#define SK6812_RMT_MEM_SYMBOLS  ( ( ( SK6812_BUF_SIZE * 8 + 1 + 63 ) / 64 ) * 64 )

/* RMT resolution: 10 MHz, 1 tick = 100 ns */
#define SK6812_RMT_RESOLUTION_HZ 10000000

/* SK6812 bit timings in RMT ticks (1 tick = 100 ns) */
#define SK6812_T0H_TICKS  3   /* 300 ns */
#define SK6812_T0L_TICKS  9   /* 900 ns */
#define SK6812_T1H_TICKS  6   /* 600 ns */
#define SK6812_T1L_TICKS  6   /* 600 ns */
/* Latch low period, split across both halves of one RMT symbol */
#define SK6812_RESET_TICKS ( SK6812_RESET_US * ( SK6812_RMT_RESOLUTION_HZ / 1000000 ) / 2 )

/* Encodes the pixel bytes followed by the latch low period, so the next
 * frame never needs a CPU busy-wait to satisfy the reset time. */
typedef struct
{
    rmt_encoder_t base;
    rmt_encoder_handle_t bytes_encoder;
    rmt_encoder_handle_t copy_encoder;
    int state;
    rmt_symbol_word_t reset_code;
} sk6812_encoder_t;

static size_t _sk6812_encode( rmt_encoder_t *encoder,
                              rmt_channel_handle_t channel,
                              const void *primary_data, size_t data_size,
                              rmt_encode_state_t *ret_state )
{
    sk6812_encoder_t *sk = __containerof( encoder, sk6812_encoder_t, base );
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    int state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;

    if( sk->state == 0 )
    {
        encoded_symbols += sk->bytes_encoder->encode( sk->bytes_encoder,
            channel, primary_data, data_size, &session_state );
        if( session_state & RMT_ENCODING_COMPLETE )
        {
            sk->state = 1;
        }
        if( session_state & RMT_ENCODING_MEM_FULL )
        {
            *ret_state = ( rmt_encode_state_t )( state | RMT_ENCODING_MEM_FULL );
            return encoded_symbols;
        }
    }

    if( sk->state == 1 )
    {
        encoded_symbols += sk->copy_encoder->encode( sk->copy_encoder,
            channel, &sk->reset_code, sizeof( sk->reset_code ),
            &session_state );
        if( session_state & RMT_ENCODING_COMPLETE )
        {
            sk->state = 0;
            state |= RMT_ENCODING_COMPLETE;
        }
        if( session_state & RMT_ENCODING_MEM_FULL )
        {
            state |= RMT_ENCODING_MEM_FULL;
        }
    }

    *ret_state = ( rmt_encode_state_t )state;
    return encoded_symbols;
}

static esp_err_t _sk6812_encoder_reset( rmt_encoder_t *encoder )
{
    sk6812_encoder_t *sk = __containerof( encoder, sk6812_encoder_t, base );
    esp_err_t err = rmt_encoder_reset( sk->bytes_encoder );
    if( err == ESP_OK )
    {
        err = rmt_encoder_reset( sk->copy_encoder );
    }
    sk->state = 0;
    return err;
}

static esp_err_t _sk6812_encoder_del( rmt_encoder_t *encoder )
{
    sk6812_encoder_t *sk = __containerof( encoder, sk6812_encoder_t, base );
    if( sk->bytes_encoder != NULL )
    {
        esp_err_t err = rmt_del_encoder( sk->bytes_encoder );
        if( err != ESP_OK ) return err;
        sk->bytes_encoder = NULL;
    }
    if( sk->copy_encoder != NULL )
    {
        esp_err_t err = rmt_del_encoder( sk->copy_encoder );
        if( err != ESP_OK ) return err;
        sk->copy_encoder = NULL;
    }
    free( sk );
    return ESP_OK;
}

static esp_err_t _sk6812_encoder_new( rmt_encoder_handle_t *ret_encoder )
{
    sk6812_encoder_t *sk = calloc( 1, sizeof( *sk ) );
    if( sk == NULL )
    {
        return ESP_ERR_NO_MEM;
    }
    sk->base.encode = _sk6812_encode;
    sk->base.reset = _sk6812_encoder_reset;
    sk->base.del = _sk6812_encoder_del;

    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .duration0 = SK6812_T0H_TICKS,
            .level0    = 1,
            .duration1 = SK6812_T0L_TICKS,
            .level1    = 0,
        },
        .bit1 = {
            .duration0 = SK6812_T1H_TICKS,
            .level0    = 1,
            .duration1 = SK6812_T1L_TICKS,
            .level1    = 0,
        },
        .flags.msb_first = 1,
    };
    esp_err_t err = rmt_new_bytes_encoder( &bytes_encoder_config,
                                           &sk->bytes_encoder );
    if( err == ESP_OK )
    {
        rmt_copy_encoder_config_t copy_encoder_config = {};
        err = rmt_new_copy_encoder( &copy_encoder_config, &sk->copy_encoder );
    }
    if( err != ESP_OK )
    {
        ( void )_sk6812_encoder_del( &sk->base );
        return err;
    }

    sk->reset_code = ( rmt_symbol_word_t ){
        .duration0 = SK6812_RESET_TICKS,
        .level0    = 0,
        .duration1 = SK6812_RESET_TICKS,
        .level1    = 0,
    };
    *ret_encoder = &sk->base;
    return ESP_OK;
}

/* ── Static state ───────────────────────────────────────────────── */
static uint8_t  _led_buf[SK6812_BUF_SIZE]; /* GRB pixel buffer      */
static uint8_t  _tx_buf[SK6812_BUF_SIZE];  /* brightness-scaled TX  */
static uint8_t  _brightness = 255;         /* 0-255 driver scale    */
static bool     _initialised = false;

static rmt_channel_handle_t _rmt_channel = NULL;
static rmt_encoder_handle_t _rmt_encoder = NULL;
static bool _rmt_enabled = false;
static StaticSemaphore_t _rgb_mutex_storage;
static SemaphoreHandle_t _rgb_mutex = NULL;
static atomic_uchar _rgb_mutex_state;
static esp_err_t _rgb_remove_locked( void );

static esp_err_t _rgb_lock( void )
{
    if( atomic_load( &_rgb_mutex_state ) != 2 )
    {
        unsigned char expected = 0;
        if( atomic_compare_exchange_strong( &_rgb_mutex_state, &expected, 1 ) )
        {
            _rgb_mutex = xSemaphoreCreateMutexStatic( &_rgb_mutex_storage );
            atomic_store( &_rgb_mutex_state, _rgb_mutex != NULL ? 2 : 0 );
        }
        else
        {
            while( atomic_load( &_rgb_mutex_state ) == 1 )
            {
                vTaskDelay( 1 );
            }
        }
    }

    if( _rgb_mutex == NULL )
    {
        return ESP_ERR_NO_MEM;
    }
    return xSemaphoreTake( _rgb_mutex,
                           pdMS_TO_TICKS( SK6812_FLUSH_TIMEOUT_MS ) ) == pdTRUE
               ? ESP_OK
               : ESP_ERR_TIMEOUT;
}

static void _rgb_unlock( void )
{
    xSemaphoreGive( _rgb_mutex );
}

/* ── Brightness helper (video-safe scale, never dims to 0) ─────── */
static inline uint8_t _scale8_video( uint8_t val, uint8_t scale )
{
    return (uint8_t)( ( (int)val * (int)scale ) >> 8 )
           + ( ( val && scale ) ? 1 : 0 );
}

/* ── Public API ─────────────────────────────────────────────────── */

esp_err_t core2foraws_rgb_led_init( void )
{
    ESP_LOGI( _TAG, "\tInitializing" );

    esp_err_t err = _rgb_lock();
    if( err != ESP_OK )
    {
        return err;
    }

    if ( _initialised )
    {
        _rgb_unlock();
        return ESP_OK;
    }
    if( _rmt_channel != NULL || _rmt_encoder != NULL )
    {
        _rgb_unlock();
        return ESP_ERR_INVALID_STATE;
    }

    /* Zero the pixel buffer */
    memset( _led_buf, 0, SK6812_BUF_SIZE );

    /* Configure RMT TX channel */
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num          = SK6812_GPIO,
        .clk_src           = RMT_CLK_SRC_DEFAULT,
        .resolution_hz     = SK6812_RMT_RESOLUTION_HZ,
        .mem_block_symbols = SK6812_RMT_MEM_SYMBOLS,
        .trans_queue_depth = 4,
    };

    err = rmt_new_tx_channel( &tx_chan_config, &_rmt_channel );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to create RMT TX channel on GPIO%d: 0x%x", SK6812_GPIO, err );
        _rgb_unlock();
        return err;
    }

    err = _sk6812_encoder_new( &_rmt_encoder );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to create SK6812 RMT encoder: 0x%x", err );
        (void)_rgb_remove_locked();
        _rgb_unlock();
        return err;
    }

    err = rmt_enable( _rmt_channel );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to enable RMT channel: 0x%x", err );
        (void)_rgb_remove_locked();
        _rgb_unlock();
        return err;
    }

    _rmt_enabled = true;
    _initialised = true;
    _rgb_unlock();
    return ESP_OK;
}

esp_err_t core2foraws_rgb_led_single_color_set( uint8_t led_num, uint32_t color )
{
    if ( led_num >= RGB_LED_NUMS )
    {
        ESP_LOGE( _TAG, "LED number %u is out of range (0-%u)",
                  led_num, RGB_LED_NUMS - 1 );
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = _rgb_lock();
    if( err != ESP_OK ) return err;

    size_t idx = led_num * SK6812_BYTES_PER_LED;
    /* SK6812 expects GRB order */
    _led_buf[idx]     = ( color >> 8 )  & 0xFF;   /* G */
    _led_buf[idx + 1] = ( color >> 16 ) & 0xFF;   /* R */
    _led_buf[idx + 2] =   color         & 0xFF;   /* B */

    _rgb_unlock();
    return ESP_OK;
}

esp_err_t core2foraws_rgb_led_side_color_set( rgb_led_side_type_t side,
                                              uint32_t color )
{
    size_t start, count;

    if ( side == RGB_LED_SIDE_RIGHT )
    {
        start = 0;
        count = RGB_LED_NUMS / 2;
    }
    else if ( side == RGB_LED_SIDE_LEFT )
    {
        start = RGB_LED_NUMS / 2;
        count = RGB_LED_NUMS / 2;
    }
    else
    {
        ESP_LOGE( _TAG, "Invalid side value: %d", side );
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = _rgb_lock();
    if( err != ESP_OK ) return err;

    for ( size_t i = start; i < start + count; i++ )
    {
        size_t idx = i * SK6812_BYTES_PER_LED;
        _led_buf[idx]     = ( color >> 8 )  & 0xFF;
        _led_buf[idx + 1] = ( color >> 16 ) & 0xFF;
        _led_buf[idx + 2] =   color         & 0xFF;
    }

    _rgb_unlock();
    return ESP_OK;
}

esp_err_t core2foraws_rgb_led_brightness_set( uint8_t brightness )
{
    if ( brightness > 100 )
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = _rgb_lock();
    if( err != ESP_OK ) return err;
    _brightness = (uint8_t)( ( (uint16_t)brightness * 255 ) / 100 );

    _rgb_unlock();
    return ESP_OK;
}

esp_err_t core2foraws_rgb_led_write( void )
{
    esp_err_t err = _rgb_lock();
    if( err != ESP_OK ) return err;

    /* The RMT channel and encoder are only valid after initialization.
       Guard against use before init to avoid passing a NULL handle to the
       RMT driver. */
    if ( !_initialised )
    {
        _rgb_unlock();
        return ESP_ERR_INVALID_STATE;
    }

    /* The previous asynchronous transfer owns _tx_buf until completion. */
    err = rmt_tx_wait_all_done( _rmt_channel, SK6812_FLUSH_TIMEOUT_MS );
    if( err != ESP_OK )
    {
        _rgb_unlock();
        return err;
    }

    /* Apply brightness scaling to a separate TX buffer */
    if ( _brightness != 255 )
    {
        for ( size_t i = 0; i < SK6812_BUF_SIZE; i++ )
        {
            _tx_buf[i] = _scale8_video( _led_buf[i], _brightness );
        }
    }
    else
    {
        memcpy( _tx_buf, _led_buf, SK6812_BUF_SIZE );
    }

    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };

    err = rmt_transmit( _rmt_channel, _rmt_encoder, _tx_buf,
                        SK6812_BUF_SIZE, &tx_config );
    _rgb_unlock();
    return err;
}

esp_err_t core2foraws_rgb_led_clear( void )
{
    esp_err_t err = _rgb_lock();
    if( err != ESP_OK ) return err;
    memset( _led_buf, 0, SK6812_BUF_SIZE );
    _rgb_unlock();
    return ESP_OK;
}

static esp_err_t _rgb_remove_locked( void )
{
    _initialised = false;
    if( _rmt_enabled )
    {
        esp_err_t err = rmt_tx_wait_all_done( _rmt_channel, SK6812_FLUSH_TIMEOUT_MS );
        if( err != ESP_OK ) return err;
        err = rmt_disable( _rmt_channel );
        if( err != ESP_OK ) return err;
        _rmt_enabled = false;
    }
    if( _rmt_encoder != NULL )
    {
        esp_err_t err = rmt_del_encoder( _rmt_encoder );
        if( err != ESP_OK ) return err;
        _rmt_encoder = NULL;
    }
    if( _rmt_channel != NULL )
    {
        esp_err_t err = rmt_del_channel( _rmt_channel );
        if( err != ESP_OK ) return err;
        _rmt_channel = NULL;
    }
    memset( _led_buf, 0, SK6812_BUF_SIZE );
    return ESP_OK;
}

esp_err_t core2foraws_rgb_led_deinit( void )
{
    esp_err_t err = _rgb_lock();
    if( err != ESP_OK ) return err;
    err = _rgb_remove_locked();
    _rgb_unlock();
    return err;
}