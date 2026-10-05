/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_audio.c
 * @brief Core2 for AWS IoT Kit audio hardware driver APIs
 */

#include <stdint.h>
#include <stdatomic.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <esp_rom_sys.h>
#include <driver/i2s_std.h>
#include <driver/i2s_pdm.h>
#include <driver/gpio.h>

#include "core2foraws_common.h"
#include "core2foraws_power.h"
#include "core2foraws_audio.h"

#define I2S_BCK_PIN 12
#define I2S_LRCK_PIN 0
#define I2S_DATA_PIN 2
#define I2S_DATA_IN_PIN 34
#define NS4168_SHUTDOWN_HOLD_US 110
#ifdef CONFIG_CORE2FORAWS_AUDIO_DMA_DESCRIPTORS
#define AUDIO_DMA_DESCRIPTORS CONFIG_CORE2FORAWS_AUDIO_DMA_DESCRIPTORS
#else
#define AUDIO_DMA_DESCRIPTORS 4
#endif
#ifdef CONFIG_CORE2FORAWS_AUDIO_DMA_FRAMES
#define AUDIO_DMA_FRAMES CONFIG_CORE2FORAWS_AUDIO_DMA_FRAMES
#else
#define AUDIO_DMA_FRAMES 128
#endif
/* SPM1423 wake-up time after the PDM clock starts (datasheet section 11.2).
   The LMD4737T261 datasheet gives no start-up time, so the same wait is used. */
#define SPM1423_WAKE_MS 10

/* The PDM clock must stay inside both microphones' active-mode ranges:
   SPM1423 1.0 MHz - 3.25 MHz (lib/audio/datasheet/SPM1423.md, sections 11.1
   and 20) and LMD4737T261 (Core2 for AWS v1.3) standard mode 1.3 MHz -
   4.8 MHz (LMD4737T261.md, operation ratings). The ESP32 PDM receiver
   derives that clock from the audio sample rate using a fixed PDM-to-PCM
   decimation ratio of 64, so the PDM clock equals AUDIO_SAMPLING_FREQ * 64. */
#define AUDIO_PDM_DECIMATION_RATIO 64
#define AUDIO_PDM_CLOCK_MIN_HZ 1300000
#define AUDIO_PDM_CLOCK_MAX_HZ 3250000
#if ( AUDIO_SAMPLING_FREQ * AUDIO_PDM_DECIMATION_RATIO ) < AUDIO_PDM_CLOCK_MIN_HZ || \
    ( AUDIO_SAMPLING_FREQ * AUDIO_PDM_DECIMATION_RATIO ) > AUDIO_PDM_CLOCK_MAX_HZ
#error "AUDIO_SAMPLING_FREQ must keep the PDM clock (AUDIO_SAMPLING_FREQ * 64) within 1.3 MHz - 3.25 MHz: use 20313 Hz to 50781 Hz."
#endif

static bool _speaker_initialized = false;
static bool _microphone_initialized = false;

static i2s_chan_handle_t _tx_handle = NULL;
static i2s_chan_handle_t _rx_handle = NULL;
static bool _tx_running = false;
static bool _rx_running = false;

/* Serializes the speaker/microphone enable/disable paths. The speaker and
   microphone share GPIO0 and the single I2S_NUM_0 controller, so concurrent
   enable calls from different tasks must not race on the init flags or the
   shared peripheral. The mutex is statically allocated so its handle is
   valid from first use without a separate init step. */
static StaticSemaphore_t _audio_mutex_buf;
static SemaphoreHandle_t _audio_mutex = NULL;
static atomic_uchar _audio_mutex_state;

enum
{
    AUDIO_MUTEX_UNINITIALIZED = 0,
    AUDIO_MUTEX_INITIALIZING,
    AUDIO_MUTEX_READY,
};

static SemaphoreHandle_t _core2foraws_audio_mutex_get( void )
{
    if( atomic_load( &_audio_mutex_state ) == AUDIO_MUTEX_READY )
    {
        return _audio_mutex;
    }

    unsigned char expected = AUDIO_MUTEX_UNINITIALIZED;
    if( atomic_compare_exchange_strong( &_audio_mutex_state, &expected,
                                        AUDIO_MUTEX_INITIALIZING ) )
    {
        _audio_mutex = xSemaphoreCreateMutexStatic( &_audio_mutex_buf );
        atomic_store( &_audio_mutex_state,
                      _audio_mutex != NULL ? AUDIO_MUTEX_READY
                                           : AUDIO_MUTEX_UNINITIALIZED );
        return _audio_mutex;
    }

    while( atomic_load( &_audio_mutex_state ) == AUDIO_MUTEX_INITIALIZING )
    {
        vTaskDelay( 1 );
    }
    return atomic_load( &_audio_mutex_state ) == AUDIO_MUTEX_READY
               ? _audio_mutex
               : NULL;
}

static const char *_TAG = "CORE2FORAWS_AUDIO";

static esp_err_t _core2foraws_audio_speaker_install( void );
static esp_err_t _core2foraws_audio_speaker_remove( void );
static esp_err_t _core2foraws_audio_mic_install( void );
static esp_err_t _core2foraws_audio_mic_remove( void );
static void _core2foraws_audio_reset_speaker_pins( void );
static void _core2foraws_audio_reset_mic_pins( void );

esp_err_t core2foraws_audio_speaker_enable( bool state )
{
    SemaphoreHandle_t mutex = _core2foraws_audio_mutex_get();
    if ( mutex == NULL )
    {
        return ESP_ERR_NO_MEM;
    }

    if( xSemaphoreTake( mutex, pdMS_TO_TICKS( AUDIO_IO_TIMEOUT_MS ) ) != pdTRUE )
    {
        return ESP_ERR_TIMEOUT;
    }
    esp_err_t err = state ? _core2foraws_audio_speaker_install() : _core2foraws_audio_speaker_remove();
    xSemaphoreGive( mutex );

    return err;
}

esp_err_t core2foraws_audio_mic_enable( bool state )
{
    SemaphoreHandle_t mutex = _core2foraws_audio_mutex_get();
    if ( mutex == NULL )
    {
        return ESP_ERR_NO_MEM;
    }

    if( xSemaphoreTake( mutex, pdMS_TO_TICKS( AUDIO_IO_TIMEOUT_MS ) ) != pdTRUE )
    {
        return ESP_ERR_TIMEOUT;
    }
    esp_err_t err = state ? _core2foraws_audio_mic_install() : _core2foraws_audio_mic_remove();
    xSemaphoreGive( mutex );

    return err;
}

esp_err_t core2foraws_audio_speaker_write( const uint8_t *sound_buffer, size_t to_write_length )
{
    if ( ( sound_buffer == NULL ) || ( to_write_length == 0 ) )
    {
        ESP_LOGE( _TAG, "Speaker write requires a valid buffer and length." );
        return ESP_ERR_INVALID_ARG;
    }

    SemaphoreHandle_t mutex = _core2foraws_audio_mutex_get();
    if( mutex == NULL )
    {
        return ESP_ERR_NO_MEM;
    }
    if( xSemaphoreTake( mutex, pdMS_TO_TICKS( AUDIO_IO_TIMEOUT_MS ) ) != pdTRUE )
    {
        return ESP_ERR_TIMEOUT;
    }

    esp_err_t err = ESP_FAIL;

    if ( ( _microphone_initialized == false ) && ( _speaker_initialized == true ) )
    {
        size_t bytes_written = 0;
        err = i2s_channel_write( _tx_handle, sound_buffer, to_write_length,
                     &bytes_written,
                     AUDIO_IO_TIMEOUT_MS );
        if ( ( err == ESP_OK ) && ( bytes_written != to_write_length ) )
        {
            ESP_LOGW( _TAG, "Speaker wrote %u of %u bytes.", ( unsigned int ) bytes_written, ( unsigned int ) to_write_length );
            err = ESP_FAIL;
        }
        /* Per-buffer trace only (never per sample) to keep the UART quiet
           even when verbose logging is enabled during audio streaming. */
        ESP_LOGV( _TAG, "Speaker wrote %u of %u bytes (0x%x).", ( unsigned int ) bytes_written, ( unsigned int ) to_write_length, err );
    }
    else
    {
        ESP_LOGE( _TAG, "Cannot write to speaker. Enable the speaker first, and keep the microphone disabled because both share GPIO0." );
        err = ESP_ERR_INVALID_STATE;
    }

    xSemaphoreGive( mutex );
    return err;
}

esp_err_t core2foraws_audio_speaker_drain(void)
{
    static const uint8_t silence[(AUDIO_DMA_DESCRIPTORS + 1) * AUDIO_DMA_FRAMES * 4] = {0};
    return core2foraws_audio_speaker_write(silence, sizeof(silence));
}

esp_err_t core2foraws_audio_mic_read( int8_t *sound_buffer, size_t to_read_length , size_t *was_read_length )
{
    if ( was_read_length != NULL )
    {
        *was_read_length = 0;
    }

    if ( ( sound_buffer == NULL ) || ( was_read_length == NULL ) || ( to_read_length == 0 ) )
    {
        ESP_LOGE( _TAG, "Microphone read requires a valid buffer, output length pointer, and length." );
        return ESP_ERR_INVALID_ARG;
    }

    SemaphoreHandle_t mutex = _core2foraws_audio_mutex_get();
    if( mutex == NULL )
    {
        return ESP_ERR_NO_MEM;
    }
    if( xSemaphoreTake( mutex, pdMS_TO_TICKS( AUDIO_IO_TIMEOUT_MS ) ) != pdTRUE )
    {
        return ESP_ERR_TIMEOUT;
    }

    esp_err_t err = ESP_FAIL;

    if ( ( _speaker_initialized == false) && ( _microphone_initialized == true ) )
    {
        err = i2s_channel_read( _rx_handle, sound_buffer, to_read_length,
                    was_read_length,
                    AUDIO_IO_TIMEOUT_MS );
        /* Per-buffer trace only (never per sample) to keep the UART quiet
           even when verbose logging is enabled during audio capture. */
        ESP_LOGV( _TAG, "Microphone read %u of %u bytes (0x%x).", ( unsigned int ) *was_read_length, ( unsigned int ) to_read_length, err );
    }
    else
    {
        ESP_LOGE( _TAG, "Cannot read from microphone. Enable the microphone first, and keep the speaker disabled because both share GPIO0." );
        err = ESP_ERR_INVALID_STATE;
    }

    xSemaphoreGive( mutex );
    return err;
}

static void _core2foraws_audio_reset_speaker_pins( void )
{
    gpio_reset_pin( I2S_LRCK_PIN );
    gpio_reset_pin( I2S_DATA_PIN );
    gpio_reset_pin( I2S_BCK_PIN );
}

static void _core2foraws_audio_reset_mic_pins( void )
{
    gpio_reset_pin( I2S_LRCK_PIN );
    gpio_reset_pin( I2S_DATA_IN_PIN );
}

static esp_err_t _core2foraws_audio_speaker_install( void )
{
    ESP_LOGI(_TAG, "\tInitializing speaker");

    esp_err_t err = ESP_OK;

    if ( _speaker_initialized == true )
    {
        ESP_LOGD( _TAG, "Speaker is already initialized." );
        return ESP_OK;
    }
    
    if ( _rx_handle != NULL || _tx_handle != NULL )
    {
        ESP_LOGE( _TAG, "Audio channel still owned; disable its current mode before enabling speaker." );
        return ESP_ERR_INVALID_STATE;
    }

    /* Set up the I2S bus BEFORE powering on the NS4168 amplifier.
       This way the amp receives valid silence (zeroed DMA buffers)
       the moment it exits shutdown, which prevents an audible pop. */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG( I2S_NUM_0, I2S_ROLE_MASTER );
    chan_cfg.dma_desc_num  = AUDIO_DMA_DESCRIPTORS;
    chan_cfg.dma_frame_num = AUDIO_DMA_FRAMES;
    chan_cfg.auto_clear    = true;

    err = i2s_new_channel( &chan_cfg, &_tx_handle, NULL );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to create I2S TX channel: 0x%x.", err );
        return err;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG( AUDIO_SAMPLING_FREQ ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG( I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO ),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_PIN,
            .ws   = I2S_LRCK_PIN,
            .dout = I2S_DATA_PIN,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT;

    err = i2s_channel_init_std_mode( _tx_handle, &std_cfg );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to init I2S std mode: 0x%x.", err );
        (void)_core2foraws_audio_speaker_remove();
        return err;
    }

    err = i2s_channel_enable( _tx_handle );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to enable I2S TX channel: 0x%x.", err );
        (void)_core2foraws_audio_speaker_remove();
        return err;
    }
    _tx_running = true;

    /* Now power on the NS4168. The I2S bus is already clocking out
       silence, so the amp wakes into a clean, quiet state. */
    err = core2foraws_power_speaker_enable( true );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to power on speaker amplifier. core2foraws_power_speaker returned 0x%x.", err );
        (void)_core2foraws_audio_speaker_remove();
        return err;
    }

    _speaker_initialized = true;

    return ESP_OK;
}

static esp_err_t _audio_channel_remove( i2s_chan_handle_t *handle, bool *running )
{
    if( *running )
    {
        esp_err_t err = i2s_channel_disable( *handle );
        if( err != ESP_OK ) return err;
        *running = false;
    }
    esp_err_t err = i2s_del_channel( *handle );
    if( err == ESP_OK ) *handle = NULL;
    return err;
}

static esp_err_t _core2foraws_audio_speaker_remove( void )
{
    if ( _tx_handle == NULL )
    {
        return ESP_OK;
    }

    _speaker_initialized = false;
    esp_err_t err = core2foraws_power_speaker_enable( false );

    if (err != ESP_OK )
    {
        ESP_LOGW( _TAG, "Failed to power off speaker amplifier. core2foraws_power_speaker returned 0x%x.", err );
        return err;
    }
    else
    {
        esp_rom_delay_us( NS4168_SHUTDOWN_HOLD_US );
    }

    err = _audio_channel_remove( &_tx_handle, &_tx_running );
    if( err != ESP_OK ) return err;

    _core2foraws_audio_reset_speaker_pins();

    return ESP_OK;
}

static esp_err_t _core2foraws_audio_mic_install( void )
{
    ESP_LOGI( _TAG, "\tInitializing microphone" );

    esp_err_t err = ESP_OK;

    if ( _microphone_initialized == true )
    {
        ESP_LOGD( _TAG, "Microphone is already initialized." );
        return ESP_OK;
    }

    if ( _tx_handle != NULL || _rx_handle != NULL )
    {
        ESP_LOGE( _TAG, "Audio channel still owned; disable its current mode before enabling microphone." );
        return ESP_ERR_INVALID_STATE;
    }

    /* PDM mode: the ESP32 generates a clock on the WS pin and reads
       the microphone's single-bit PDM stream. The I2S hardware
       decimates the PDM bitstream into 16-bit PCM samples for us. */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG( I2S_NUM_0, I2S_ROLE_MASTER );
    chan_cfg.dma_desc_num  = AUDIO_DMA_DESCRIPTORS;
    chan_cfg.dma_frame_num = AUDIO_DMA_FRAMES;

    err = i2s_new_channel( &chan_cfg, NULL, &_rx_handle );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to create I2S RX channel: 0x%x.", err );
        return err;
    }

    i2s_pdm_rx_config_t pdm_rx_cfg = {
        .clk_cfg  = I2S_PDM_RX_CLK_DEFAULT_CONFIG( AUDIO_SAMPLING_FREQ ),
        .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG( I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO ),
        .gpio_cfg = {
            .clk = I2S_LRCK_PIN,
            .din = I2S_DATA_IN_PIN,
            .invert_flags = {
                .clk_inv = false,
            },
        },
    };
    pdm_rx_cfg.slot_cfg.slot_mask = I2S_PDM_SLOT_RIGHT;

    err = i2s_channel_init_pdm_rx_mode( _rx_handle, &pdm_rx_cfg );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to init I2S PDM RX mode: 0x%x.", err );
        (void)_core2foraws_audio_mic_remove();
        return err;
    }

    err = i2s_channel_enable( _rx_handle );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "Failed to enable I2S RX channel: 0x%x.", err );
        (void)_core2foraws_audio_mic_remove();
        return err;
    }
    _rx_running = true;

    /* The SPM1423 microphone needs up to 10 ms to wake up after the
       PDM clock starts. Wait here so the first read returns real audio
       instead of startup noise. */
    vTaskDelay( CORE2FORAWS_DELAY_MS_TO_TICKS( SPM1423_WAKE_MS ) );
    
    _microphone_initialized = true;

    return ESP_OK;
}

static esp_err_t _core2foraws_audio_mic_remove( void )
{
    if ( _rx_handle == NULL )
    {
        return ESP_OK;
    }

    _microphone_initialized = false;
    esp_err_t err = _audio_channel_remove( &_rx_handle, &_rx_running );
    if( err != ESP_OK ) return err;
    _core2foraws_audio_reset_mic_pins();

    return ESP_OK;
}