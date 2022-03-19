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
 * @file core2foraws_expports.c
 * @brief Core2 for AWS IoT EduKit expansion ports hardware driver APIs
 */

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/adc.h>
#include <driver/dac.h>
#include <esp_adc_cal.h>
#include <soc/dac_channel.h>

#include "i2c_manager.h"
#include "core2foraws_common.h"
#include "core2foraws_expports.h"

#define DEFAULT_VREF            1100
#define ADC_CHANNEL             ADC1_CHANNEL_0
#define ADC_WIDTH               ADC_WIDTH_BIT_12
#define ADC_ATTENUATION         ADC_ATTEN_DB_11
#define DAC_CHANNEL             DAC_GPIO26_CHANNEL

/**
 * @brief Modes supported by the BSP for the GPIO pins.
 *
 * These are the modes supported for the GPIO pins by the BSP.
 * Read more about [UART communications with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html).
 */
/* @[declare_pin_mode_t] */
typedef enum 
{
    NONE,   /**< @brief Reset GPIO to default state. */
    OUTPUT, /**< @brief Set GPIO to output mode. */
    INPUT,  /**< @brief Set GPIO to input mode. */
    I2C,    /**< @brief Enable I2C mode. Only available on Port A—GPIO 
                        32(SDA) and Port A—GPIO 33 (SCL). */
    ADC,    /**< @brief Enable ADC mode. Only available on Port B—GPIO 36 */
    DAC,    /**< @brief Enable DAC mode. Only available on Port B—GPIO 26 */
    UART    /**< @brief Enable UART RX/TX mode. UART TX only available on Port 
                        C—GPIO 14 and UART RX is only available on Port C—GPIO 
                        13. Only supports full-duplex UART so setting one pin 
                        to UART mode will also set the other pin to UART mode.*/
} pin_mode_t;
/* @[declare_pin_mode_t] */

struct 
{
    gpio_num_t pin;
    pin_mode_t mode;
} static _s_port_pins[] = 
{
    { PORT_A_SDA_PIN, NONE },
    { PORT_A_SCL_PIN, NONE },
    { PORT_B_ADC_PIN, NONE },
    { PORT_B_DAC_PIN, NONE },
    { PORT_C_UART_TX_PIN, NONE },
    { PORT_C_UART_RX_PIN, NONE }
};

static esp_adc_cal_characteristics_t *_s_adc_characterization;

static const char *_s_TAG = "CORE2FORAWS_EXPPORT";

static uint8_t _s_core2foraws_expports_get_index( gpio_num_t pin );
static esp_err_t _s_core2foraws_expports_pin_init( gpio_num_t pin, pin_mode_t mode );
static esp_err_t _s_core2foraws_expports_pin_handler( gpio_num_t pin, pin_mode_t mode );

static uint8_t _s_core2foraws_expports_get_index( gpio_num_t pin )
{
    if ( pin  == PORT_A_SDA_PIN )
        return 0;
    else if ( pin == PORT_A_SCL_PIN )
        return 1;
    else if ( pin == PORT_B_ADC_PIN )
        return 2;
    else if ( pin == PORT_B_DAC_PIN )
        return 3;
    else if( PORT_C_UART_TX_PIN )
        return 4;
    else if ( PORT_C_UART_RX_PIN )
        return 5;

    return 0xFF;
}

static esp_err_t _s_core2foraws_expports_pin_init( gpio_num_t pin, pin_mode_t mode )
{
    esp_err_t err = ESP_FAIL;

    if ( mode == OUTPUT || mode == INPUT )
    {
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.pin_bit_mask = ( 1ULL << pin );

        if ( mode == OUTPUT )
        {
            io_conf.mode = GPIO_MODE_OUTPUT; 
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            err = gpio_config( &io_conf );
            if ( err != ESP_OK )
            {
                ESP_LOGE( _s_TAG, "\tError configuring GPIO %d as ouput. Error code: 0x%x.", pin, err);
            }
        } 
        else
        {
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            err = gpio_config( &io_conf );
            if ( err != ESP_OK )
            {
                ESP_LOGE( _s_TAG, "\tError configuring GPIO %d as input. Error code: 0x%x.", pin, err );
            }
        }  
    }
    else if (mode == ADC)
    {
        err = adc1_config_width( ADC_WIDTH );
        if ( err != ESP_OK )
        {
            ESP_LOGE( _s_TAG, "\tError configuring ADC width on pin %d. Error code: 0x%x.", pin, err );
            return err;
        }
        
        err = adc1_config_channel_atten( ADC_CHANNEL, ADC_ATTENUATION );
        if ( err != ESP_OK )
        {
            ESP_LOGE( _s_TAG, "\tError configuring ADC channel attenuation on pin %d. Error code: 0x%x.", pin, err );
        }
        
        _s_adc_characterization = heap_caps_calloc( 1, sizeof( esp_adc_cal_characteristics_t ), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT );
        esp_adc_cal_characterize( ADC_UNIT_1, ADC_ATTENUATION, ADC_WIDTH, DEFAULT_VREF, _s_adc_characterization );
    }
    else if ( mode == DAC )
    {
        dac_output_enable( DAC_CHANNEL );
    }
    else if ( mode == UART )
    {
        err = uart_driver_install(PORT_C_UART_NUM, UART_RX_BUF_SIZE, 0, 0, NULL, 0);
        if ( err != ESP_OK )
        {
            ESP_LOGE( _s_TAG, "\tUART driver installation failed for UART num %d. Error code: 0x%x.", PORT_C_UART_NUM, err);
            return err;
        }

        err = uart_set_pin(PORT_C_UART_NUM, PORT_C_UART_TX_PIN, PORT_C_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        if ( err != ESP_OK )
        {
            ESP_LOGE( _s_TAG, "\tFailed to set pins %d, %d, to  UART%d. Error code: 0x%x.", PORT_C_UART_RX_PIN, PORT_C_UART_TX_PIN, PORT_C_UART_NUM, err);
        }
    }
    else if ( mode == NONE )
    {
        err = dac_output_disable(DAC_CHANNEL);
        err |= gpio_reset_pin(pin);
        err |= uart_driver_delete(UART_NUM_2);
        err |= core2foraws_expports_i2c_close();
    }

    return err;
}

static esp_err_t _s_core2foraws_expports_pin_handler( gpio_num_t pin, pin_mode_t mode )
{
    esp_err_t err = ESP_ERR_NOT_SUPPORTED;

    if ( pin != PORT_A_SDA_PIN && pin != PORT_A_SCL_PIN && pin != PORT_B_ADC_PIN && pin != PORT_B_DAC_PIN && pin != PORT_C_UART_RX_PIN && pin != PORT_C_UART_TX_PIN )
    {
        ESP_LOGE( _s_TAG, "\tOnly Port A (GPIO 32 and 33), Port B (GPIO 26 and 36), and Port C (GPIO 13 and 14) are supported. Pin selected: %d", pin );
        return err;
    }
    else if ( mode == OUTPUT && pin == GPIO_NUM_36 )
    {
        ESP_LOGE( _s_TAG, "\tGPIO 36 does not support digital output" );
        return err;
    }
    else 
    {
        err = ESP_OK;
    }

    uint8_t index = _s_core2foraws_expports_get_index( pin );

    if ( _s_port_pins[ index ].mode != mode || _s_port_pins[ index ].mode != NONE )
    {
        err = ESP_ERR_INVALID_STATE;
        ESP_LOGD( _s_TAG, "\tPin %d is currently set in a different mode. Resetting", pin );
        
        _s_core2foraws_expports_pin_init( pin, NONE );
        _s_port_pins[ index ].mode = NONE;
    }

    if ( _s_port_pins[ index ].mode == NONE && _s_port_pins[ index ].mode != mode )
    {
        _s_core2foraws_expports_pin_init( pin, mode );
        _s_port_pins[ index ].mode = mode;
    }

    return err; 
}

esp_err_t core2foraws_expports_digital_read( gpio_num_t pin, bool *level )
{
    esp_err_t err = _s_core2foraws_expports_pin_handler( pin, INPUT );
    if ( err == ESP_OK )
    {
        *level = gpio_get_level( pin );
    }

    return err;
}

esp_err_t core2foraws_expports_digital_write( gpio_num_t pin, const bool level )
{
    esp_err_t err = _s_core2foraws_expports_pin_handler( pin, OUTPUT );
    if ( err == ESP_OK )
    {
        err = gpio_set_level(pin, level);
    }
    
    return err;
}

esp_err_t core2foraws_expports_pin_reset( gpio_num_t pin )
{
    return _s_core2foraws_expports_pin_handler( pin, NONE );
}

esp_err_t core2foraws_expports_i2c_begin( void )
{
    _s_core2foraws_expports_pin_handler( PORT_A_SDA_PIN, I2C );
    _s_core2foraws_expports_pin_handler( PORT_A_SCL_PIN, I2C );

    return i2c_manager_init( COMMON_I2C_EXTERNAL );
}

esp_err_t core2foraws_expports_i2c_read( uint16_t device_address, uint32_t register_address, uint8_t *data, uint16_t length )
{
    return i2c_manager_read( COMMON_I2C_EXTERNAL, device_address, register_address, data, length );
}

esp_err_t core2foraws_expports_i2c_write( uint16_t device_address, uint32_t register_address, const uint8_t *data, uint16_t length )
{
    return i2c_manager_write( COMMON_I2C_EXTERNAL, device_address, register_address, data, length );
}

esp_err_t core2foraws_expports_i2c_close( void )
{
    _s_core2foraws_expports_pin_handler( PORT_A_SDA_PIN, NONE );
    _s_core2foraws_expports_pin_handler( PORT_A_SCL_PIN, NONE );
    return i2c_manager_close( COMMON_I2C_EXTERNAL );
}

esp_err_t core2foraws_expports_adc_read( int *raw_adc_value )
{
    esp_err_t err = _s_core2foraws_expports_pin_handler( PORT_B_ADC_PIN, ADC );
    if ( err == ESP_OK )
    {
        *raw_adc_value = adc1_get_raw(ADC_CHANNEL);
    }
    
    return err;
}

esp_err_t core2foraws_expports_adc_mv_read( uint32_t *adc_mvolts )
{
    esp_err_t err = ESP_FAIL;
    
    err = _s_core2foraws_expports_pin_handler( PORT_B_ADC_PIN, ADC );
    if ( err == ESP_OK )
    {
        err = esp_adc_cal_get_voltage(ADC_CHANNEL, _s_adc_characterization, adc_mvolts );
    }
    
    return err;
}

esp_err_t core2foraws_expports_dac_mv_write( const uint16_t dac_mvolts )
{
    esp_err_t err = ESP_FAIL;

    err = _s_core2foraws_expports_pin_handler( PORT_B_DAC_PIN, DAC );
    if ( err == ESP_OK )
    {
        uint8_t duty = 0;
    
        if ( dac_mvolts > 3200 )
        {
            duty = 255;
        }
        else if ( dac_mvolts >= 200 && dac_mvolts <= 3200 )
        {
            duty = (dac_mvolts - 200) / 12.15; // An approximate linear formula to calulate mv output.
        }

        err = dac_output_voltage(DAC_CHANNEL, duty);
    }
    
    return err;
}

esp_err_t core2foraws_expports_uart_begin( uint32_t baud )
{
    _s_core2foraws_expports_pin_handler( PORT_C_UART_RX_PIN, UART );
    _s_core2foraws_expports_pin_handler( PORT_C_UART_TX_PIN, UART );

    const uart_config_t uart_config = 
    {
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    
    esp_err_t err = uart_param_config( PORT_C_UART_NUM, &uart_config );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _s_TAG, "\tFailed to configure UART%d with the provided configuration.", PORT_C_UART_NUM );
    }    

    return err;
}

esp_err_t core2foraws_expports_uart_read( uint8_t *message_buffer, size_t *was_read_length )
{
    esp_err_t err = ESP_FAIL;
    int rxBytes = 0;
    int cached_buffer_length = 0;

    err = uart_get_buffered_data_len( PORT_C_UART_NUM, ( size_t* )&cached_buffer_length );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _s_TAG, "\tFailed to get UART ring buffer length. Check if pins were set to UART and has been configured." );
        return err;
    }

    if ( cached_buffer_length )
    {
        rxBytes = uart_read_bytes(PORT_C_UART_NUM, message_buffer, (size_t)&cached_buffer_length, pdMS_TO_TICKS(1000));
        if ( rxBytes == -1 )
        {
            err = ESP_FAIL;
            *was_read_length = 0;
        }
        else
        {
            err = ESP_OK;
            *was_read_length = ( size_t ) rxBytes;
        }
    }
    return err;
}

esp_err_t core2foraws_expports_uart_write( const char *message, size_t length, size_t *was_written_length )
{
    esp_err_t err = ESP_FAIL;
    int txBytes = 0;

    txBytes = uart_write_bytes( PORT_C_UART_NUM, message, length );
    if( txBytes == -1 )
    {
        err = ESP_ERR_INVALID_ARG;
        *was_written_length = 0;
    }
    else{
        err = ESP_OK;
        *was_written_length = ( size_t ) txBytes;
    }

    return err;
}