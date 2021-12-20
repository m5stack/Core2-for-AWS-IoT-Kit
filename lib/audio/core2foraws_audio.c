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
 * @file core2foraws_audio.c
 * @brief Core2 for AWS IoT EduKit audio hardware driver APIs
 */

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>

#include "core2foraws_common.h"
#include "core2foraws_power.h"
#include "core2foraws_audio.h"

#define I2S_BCK_PIN 12
#define I2S_LRCK_PIN 0
#define I2S_DATA_PIN 2
#define I2S_DATA_IN_PIN 34

static bool _s_speaker_initialized = false;
static bool _s_microphone_initialized = false;

static const char *_s_TAG = "CORE2FORAWS_AUDIO";

static esp_err_t _s_core2foraws_audio_speaker_install( void );
static esp_err_t _s_core2foraws_audio_speaker_remove( void );
static esp_err_t _s_core2foraws_audio_mic_install( void );
static esp_err_t _s_core2foraws_audio_mic_remove( void );

esp_err_t core2foraws_audio_speaker_enable( bool state )
{
    esp_err_t err = state ? _s_core2foraws_audio_speaker_install() : _s_core2foraws_audio_speaker_remove();

    return err;
}

esp_err_t core2foraws_audio_mic_enable( bool state )
{
    esp_err_t err = state ? _s_core2foraws_audio_mic_install() : _s_core2foraws_audio_mic_remove();

    return err;
}

esp_err_t core2foraws_audio_speaker_write( const uint8_t *sound_buffer, size_t to_write_length )
{
    esp_err_t err = ESP_FAIL;

    if ( ( _s_microphone_initialized == false ) && ( _s_speaker_initialized == true ) )
    {
        size_t bytes_written = 0;
        err = i2s_write( AUDIO_I2S_PORT_NUM, sound_buffer, to_write_length, &bytes_written, portMAX_DELAY );

    }

    return err;
}

esp_err_t core2foraws_audio_mic_read( int8_t *sound_buffer, size_t to_read_length , size_t *was_read_length )
{
    esp_err_t err = ESP_FAIL;

    if ( ( _s_speaker_initialized == false) && ( _s_microphone_initialized == true ) )
    {
        err = i2s_read( AUDIO_I2S_PORT_NUM, sound_buffer, to_read_length, was_read_length, portMAX_DELAY );
    }
    else
    {
        ESP_LOGE( _s_TAG, "Error writing to microphone. Either speaker is initialized or microphone has not been initialized yet." );
    }

    return err;
}

static esp_err_t _s_core2foraws_audio_speaker_install( void )
{
    ESP_LOGI(_s_TAG, "\tInitializing speaker");

    esp_err_t err = ESP_FAIL;
    
    if ( _s_microphone_initialized == true )
    {
        ESP_LOGD( _s_TAG, "Microphone is initialized. Cannot use speaker at the same time." );
        return err;
    }

    err = core2foraws_power_speaker_enable( true );
    if (err != ESP_OK )
    {
        ESP_LOGD( _s_TAG, "\tFailed to power on speaker amplifier. core2foraws_power_speaker returned 0x%x.", err );
        return err;
    }

    i2s_config_t i2s_config = {
        .mode = ( i2s_mode_t )( I2S_MODE_MASTER ),
        .sample_rate = AUDIO_SAMPLING_FREQ,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL( 4, 1, 0 )
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
#else
        .communication_format = I2S_COMM_FORMAT_I2S,
#endif
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = 128,
    };

    i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
    i2s_config.use_apll = false;
    i2s_config.tx_desc_auto_clear = true;

    err |= i2s_driver_install( AUDIO_I2S_PORT_NUM, &i2s_config, 0, NULL );
    if ( err != ESP_OK )
    {
        ESP_LOGD( _s_TAG, "\tFailed to install speaker i2s driver. i2s_driver_install returned 0x%x.", err );
    }

    i2s_pin_config_t tx_pin_config;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL( 4, 4, 0 )
    tx_pin_config.mck_io_num = 0,
#endif
    tx_pin_config.bck_io_num = I2S_BCK_PIN;
    tx_pin_config.ws_io_num = I2S_LRCK_PIN;
    tx_pin_config.data_out_num = I2S_DATA_PIN;
    tx_pin_config.data_in_num = I2S_DATA_IN_PIN;
    err |= i2s_set_pin( AUDIO_I2S_PORT_NUM, &tx_pin_config );
    err |= i2s_set_clk( AUDIO_I2S_PORT_NUM, AUDIO_SAMPLING_FREQ, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO );

    _s_speaker_initialized = true;

    return core2foraws_common_error( err );  
}

static esp_err_t _s_core2foraws_audio_speaker_remove( void )
{
    esp_err_t err = ESP_FAIL;

    err = core2foraws_power_speaker_enable( false );

    if (err != ESP_OK )
    {
        ESP_LOGD( _s_TAG, "\tFailed to power off speaker amplifier. core2foraws_power_speaker returned 0x%x.", err );
        
    }
    err |= i2s_driver_uninstall( AUDIO_I2S_PORT_NUM );
    err |= gpio_reset_pin( I2S_LRCK_PIN );
    err |= gpio_reset_pin( I2S_DATA_PIN );
    err |= gpio_reset_pin( I2S_BCK_PIN );
    err |= gpio_reset_pin( I2S_DATA_IN_PIN );
    
    _s_speaker_initialized = false;

    return core2foraws_common_error( err );
}

static esp_err_t _s_core2foraws_audio_mic_install( void )
{
    ESP_LOGI( _s_TAG, "\tInitializing microphone" );

    esp_err_t err = ESP_FAIL;

    if ( _s_speaker_initialized == true )
    {
        ESP_LOGD( _s_TAG, "Speaker is initialized. Cannot use microphone at the same time." );
        return err;
    }

    i2s_config_t i2s_config = 
    {
        .mode = ( i2s_mode_t )( I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM ),
        .sample_rate = AUDIO_SAMPLING_FREQ,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL( 4, 1, 0 )
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
#else
        .communication_format = I2S_COMM_FORMAT_I2S,
#endif
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = 128,
    };

    i2s_pin_config_t pin_config;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL( 4, 4, 0 )
    pin_config.mck_io_num = 0,
#endif
    pin_config.bck_io_num = I2S_PIN_NO_CHANGE;
    pin_config.ws_io_num = I2S_LRCK_PIN;
    pin_config.data_out_num = I2S_PIN_NO_CHANGE;
    pin_config.data_in_num = I2S_DATA_IN_PIN;
    
    err = i2s_driver_install( AUDIO_I2S_PORT_NUM, &i2s_config, 0, NULL );
    err |= i2s_set_pin( AUDIO_I2S_PORT_NUM, &pin_config );
    err |= i2s_set_clk( AUDIO_I2S_PORT_NUM, AUDIO_SAMPLING_FREQ, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO );
    
    _s_microphone_initialized = true;

    return core2foraws_common_error( err );
}

static esp_err_t _s_core2foraws_audio_mic_remove( void )
{
    esp_err_t err = ESP_FAIL;

    err = i2s_driver_uninstall( AUDIO_I2S_PORT_NUM );
    err |= gpio_reset_pin( I2S_LRCK_PIN );
    err |= gpio_reset_pin( I2S_DATA_IN_PIN );

    _s_microphone_initialized = false;

    return core2foraws_common_error( err );
}