/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_expports.c
 * @brief Core2 for AWS IoT Kit expansion ports hardware driver APIs
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/dac_oneshot.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <soc/dac_channel.h>

#include "core2foraws_common.h"
#include "core2foraws_expports.h"

#define DEFAULT_VREF            1100
#define ADC_CHANNEL             ADC_CHANNEL_0
#define ADC_ATTENUATION         ADC_ATTEN_DB_12
#define DAC_CHANNEL             DAC_GPIO26_CHANNEL

/**
 * @brief Modes supported by the BSP for the GPIO pins.
 *
 * These are the modes supported for the GPIO pins by the BSP.
 * Read more about [UART communications with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/uart.html).
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
} static _port_pins[] = 
{
    { PORT_A_SDA_PIN, NONE },
    { PORT_A_SCL_PIN, NONE },
    { PORT_B_ADC_PIN, NONE },
    { PORT_B_DAC_PIN, NONE },
    { PORT_C_UART_TX_PIN, NONE },
    { PORT_C_UART_RX_PIN, NONE }
};

static adc_oneshot_unit_handle_t _adc_handle = NULL;
static adc_cali_handle_t _adc_cali_handle = NULL;
static dac_oneshot_handle_t _dac_handle = NULL;
static StaticSemaphore_t _expports_mutex_storage;
static SemaphoreHandle_t _expports_mutex = NULL;
static atomic_uchar _expports_mutex_state;

static const char *_TAG = "CORE2FORAWS_EXPPORT";

static uint8_t _core2foraws_expports_get_index( gpio_num_t pin );
static esp_err_t _core2foraws_expports_pin_init( gpio_num_t pin, pin_mode_t mode );
static esp_err_t _core2foraws_expports_pin_handler( gpio_num_t pin, pin_mode_t mode );

static esp_err_t _expports_lock( void )
{
    if( atomic_load( &_expports_mutex_state ) != 2 )
    {
        unsigned char expected = 0;
        if( atomic_compare_exchange_strong( &_expports_mutex_state, &expected,
                                            1 ) )
        {
            _expports_mutex = xSemaphoreCreateRecursiveMutexStatic(
                &_expports_mutex_storage );
            atomic_store( &_expports_mutex_state,
                          _expports_mutex != NULL ? 2 : 0 );
        }
        else
        {
            while( atomic_load( &_expports_mutex_state ) == 1 )
            {
                vTaskDelay( 1 );
            }
        }
    }

    if( _expports_mutex == NULL )
    {
        return ESP_ERR_NO_MEM;
    }
    return xSemaphoreTakeRecursive( _expports_mutex, pdMS_TO_TICKS( 1000 ) ) ==
                   pdTRUE
               ? ESP_OK
               : ESP_ERR_TIMEOUT;
}

static void _expports_unlock( void )
{
    xSemaphoreGiveRecursive( _expports_mutex );
}

static uint8_t _core2foraws_expports_get_index( gpio_num_t pin )
{
    if ( pin  == PORT_A_SDA_PIN )
        return 0;
    else if ( pin == PORT_A_SCL_PIN )
        return 1;
    else if ( pin == PORT_B_ADC_PIN )
        return 2;
    else if ( pin == PORT_B_DAC_PIN )
        return 3;
    else if ( pin == PORT_C_UART_TX_PIN )
        return 4;
    else if ( pin == PORT_C_UART_RX_PIN )
        return 5;

    return 0xFF;
}

static esp_err_t _expports_pair_reset( pin_mode_t mode )
{
    gpio_num_t first_pin = mode == I2C ? PORT_A_SDA_PIN : PORT_C_UART_RX_PIN;
    gpio_num_t second_pin = mode == I2C ? PORT_A_SCL_PIN : PORT_C_UART_TX_PIN;
    esp_err_t err = mode == I2C ? core2foraws_i2c_deinit( COMMON_I2C_EXTERNAL )
                                : uart_driver_delete( PORT_C_UART_NUM );
    if( err != ESP_OK ) return err;
    _port_pins[ _core2foraws_expports_get_index( first_pin ) ].mode = NONE;
    _port_pins[ _core2foraws_expports_get_index( second_pin ) ].mode = NONE;
    err = gpio_reset_pin( first_pin );
    esp_err_t second_err = gpio_reset_pin( second_pin );
    return err != ESP_OK ? err : second_err;
}

static esp_err_t _core2foraws_expports_pin_init( gpio_num_t pin, pin_mode_t mode )
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
                ESP_LOGE( _TAG, "\tError configuring GPIO %d as ouput. Error code: 0x%x.", pin, err );
            }
        } 
        else
        {
            io_conf.mode = GPIO_MODE_INPUT;
            /* GPIOs 34-39 are input-only and have no internal pull resistors */
            if ( pin >= GPIO_NUM_34 )
            {
                io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            }
            else
            {
                io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            }
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            err = gpio_config( &io_conf );
            if ( err != ESP_OK )
            {
                ESP_LOGE( _TAG, "\tError configuring GPIO %d as input. Error code: 0x%x.", pin, err );
            }
        }  
    }
    else if (mode == ADC)
    {
        adc_oneshot_unit_init_cfg_t init_cfg = {
            .unit_id = ADC_UNIT_1,
        };
        err = adc_oneshot_new_unit( &init_cfg, &_adc_handle );
        if ( err != ESP_OK )
        {
            ESP_LOGE( _TAG, "\tError initializing ADC unit on pin %d. Error code: 0x%x.", pin, err );
            return err;
        }

        adc_oneshot_chan_cfg_t chan_cfg = {
            .atten = ADC_ATTENUATION,
            .bitwidth = ADC_BITWIDTH_12,
        };
        err = adc_oneshot_config_channel( _adc_handle, ADC_CHANNEL, &chan_cfg );
        if ( err != ESP_OK )
        {
            ESP_LOGE( _TAG, "\tError configuring ADC channel on pin %d. Error code: 0x%x.", pin, err );
            adc_oneshot_del_unit( _adc_handle );
            _adc_handle = NULL;
            return err;
        }

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        adc_cali_curve_fitting_config_t cali_cfg = {
            .unit_id = ADC_UNIT_1,
            .atten = ADC_ATTENUATION,
            .bitwidth = ADC_BITWIDTH_12,
        };
        err = adc_cali_create_scheme_curve_fitting( &cali_cfg, &_adc_cali_handle );
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        adc_cali_line_fitting_config_t cali_cfg = {
            .unit_id = ADC_UNIT_1,
            .atten = ADC_ATTENUATION,
            .bitwidth = ADC_BITWIDTH_12,
            .default_vref = DEFAULT_VREF,
        };
        err = adc_cali_create_scheme_line_fitting( &cali_cfg, &_adc_cali_handle );
#endif
        if ( err != ESP_OK )
        {
            ESP_LOGW( _TAG, "\tADC calibration scheme not available. Raw values only." );
            _adc_cali_handle = NULL;
            err = ESP_OK;
        }
    }
    else if ( mode == DAC )
    {
        dac_oneshot_config_t dac_cfg = {
            .chan_id = DAC_CHANNEL,
        };
        err = dac_oneshot_new_channel( &dac_cfg, &_dac_handle );
    }
    else if ( mode == UART )
    {
        /* UART uses two pins but the driver only needs to be installed once */
        uint8_t rx_idx = _core2foraws_expports_get_index( PORT_C_UART_RX_PIN );
        uint8_t tx_idx = _core2foraws_expports_get_index( PORT_C_UART_TX_PIN );

        if ( _port_pins[ rx_idx ].mode != UART && _port_pins[ tx_idx ].mode != UART )
        {
            err = uart_driver_install(PORT_C_UART_NUM, UART_RX_BUF_SIZE, 0, 0, NULL, 0);
            if ( err != ESP_OK )
            {
                ESP_LOGE( _TAG, "\tUART driver installation failed for UART num %d. Error code: 0x%x.", PORT_C_UART_NUM, err );
                return err;
            }

            _port_pins[ rx_idx ].mode = UART;
            _port_pins[ tx_idx ].mode = UART;
        }
        else
        {
            err = ESP_OK;
        }
    }
    else if ( mode == I2C )
    {
        err = core2foraws_i2c_init( COMMON_I2C_EXTERNAL );
    }
    else if ( mode == NONE )
    {
        uint8_t index = _core2foraws_expports_get_index( pin );
        pin_mode_t current_mode = _port_pins[ index ].mode;

        if( current_mode == I2C || current_mode == UART )
        {
            return _expports_pair_reset( current_mode );
        }
        if ( current_mode == DAC )
        {
            err = dac_oneshot_del_channel( _dac_handle );
            if( err != ESP_OK ) return err;
            _dac_handle = NULL;
        }
        else if ( current_mode == ADC )
        {
            /* The calibration handle is only created when a calibration
             * scheme is available, so it may be NULL even though the ADC
             * unit was allocated. Tear both down independently. */
            if ( _adc_cali_handle != NULL )
            {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
                adc_cali_delete_scheme_curve_fitting( _adc_cali_handle );
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
                adc_cali_delete_scheme_line_fitting( _adc_cali_handle );
#endif
                _adc_cali_handle = NULL;
            }
            if ( _adc_handle != NULL )
            {
                adc_oneshot_del_unit( _adc_handle );
                _adc_handle = NULL;
            }
            err = ESP_OK;
        }
        else
        {
            err = ESP_OK;
        }

        err |= gpio_reset_pin( pin );
    }

    return err;
}

static esp_err_t _core2foraws_expports_pin_handler( gpio_num_t pin, pin_mode_t mode )
{
    if ( pin != PORT_A_SDA_PIN && pin != PORT_A_SCL_PIN && pin != PORT_B_ADC_PIN && pin != PORT_B_DAC_PIN && pin != PORT_C_UART_RX_PIN && pin != PORT_C_UART_TX_PIN )
    {
        ESP_LOGE( _TAG, "\tOnly Port A (GPIO 32 and 33), Port B (GPIO 26 and 36), and Port C (GPIO 13 and 14) are supported. Pin selected: %d", pin );
        return ESP_ERR_NOT_SUPPORTED;
    }

    if ( mode == OUTPUT && pin == GPIO_NUM_36 )
    {
        ESP_LOGE( _TAG, "\tGPIO 36 does not support digital output" );
        return ESP_ERR_NOT_SUPPORTED;
    }

    uint8_t index = _core2foraws_expports_get_index( pin );
    pin_mode_t current_mode = _port_pins[ index ].mode;

    ESP_LOGD( _TAG, "\n\n\tPin %d mode updating — new mode %d, current mode %d,\n\n", pin, mode, current_mode );

    /* Already in the requested mode — nothing to do */
    if ( current_mode == mode )
    {
        return ESP_OK;
    }

    /* If pin is in a different active mode, reset it first */
    if ( current_mode != NONE )
    {
        ESP_LOGD( _TAG, "\tPin %d is currently set in a different mode. Resetting", pin );

        esp_err_t reset_err = _core2foraws_expports_pin_init( pin, NONE );
        if ( reset_err != ESP_OK )
        {
            return reset_err;
        }
        _port_pins[ index ].mode = NONE;
    }

    /* If the target mode is NONE, we are done (already reset above) */
    if ( mode == NONE )
    {
        return ESP_OK;
    }

    /* Initialize pin in the new mode */
    esp_err_t err = _core2foraws_expports_pin_init( pin, mode );
    if ( err == ESP_OK )
    {
        _port_pins[ index ].mode = mode;
    }

    return err;
}

esp_err_t core2foraws_expports_digital_read( gpio_num_t pin, bool *level )
{
    if( level == NULL ) return ESP_ERR_INVALID_ARG;
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    err = _core2foraws_expports_pin_handler( pin, INPUT );
    if ( err == ESP_OK )
    {
        *level = gpio_get_level( pin );
    }

    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_digital_write( gpio_num_t pin, const bool level )
{
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    err = _core2foraws_expports_pin_handler( pin, OUTPUT );
    if ( err == ESP_OK )
    {
        err = gpio_set_level(pin, level);
    }
    
    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_pin_reset( gpio_num_t pin )
{
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    err = _core2foraws_expports_pin_handler( pin, NONE );
    _expports_unlock();
    return err;
}

static esp_err_t _expports_pair_prepare( gpio_num_t first_pin,
                                         gpio_num_t second_pin, pin_mode_t mode )
{
    if( _port_pins[ _core2foraws_expports_get_index( first_pin ) ].mode == mode &&
        _port_pins[ _core2foraws_expports_get_index( second_pin ) ].mode == mode )
    {
        return ESP_OK;
    }
    esp_err_t err = _core2foraws_expports_pin_handler( first_pin, NONE );
    if( err != ESP_OK ) return err;
    return _core2foraws_expports_pin_handler( second_pin, NONE );
}

esp_err_t core2foraws_expports_i2c_begin( void )
{
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;

    err = _expports_pair_prepare( PORT_A_SDA_PIN, PORT_A_SCL_PIN, I2C );
    if( err != ESP_OK )
    {
        _expports_unlock();
        return err;
    }
    err = _core2foraws_expports_pin_handler( PORT_A_SDA_PIN, I2C );
    if( err == ESP_OK )
    {
        err = _core2foraws_expports_pin_handler( PORT_A_SCL_PIN, I2C );
    }
    if( err != ESP_OK )
    {
        _core2foraws_expports_pin_handler( PORT_A_SDA_PIN, NONE );
    }

    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_i2c_device_add( uint16_t device_address, uint32_t scl_speed_hz, i2c_master_dev_handle_t *dev_handle )
{
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    err = core2foraws_i2c_device_add( COMMON_I2C_EXTERNAL, device_address,
                                      scl_speed_hz, dev_handle );
    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_i2c_device_remove(
    i2c_master_dev_handle_t dev_handle )
{
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    err = core2foraws_i2c_device_remove( dev_handle );
    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_i2c_read( i2c_master_dev_handle_t dev_handle, uint32_t register_address, uint8_t *data, uint16_t length )
{
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    err = core2foraws_i2c_read( COMMON_I2C_EXTERNAL, dev_handle,
                                register_address, data, length );
    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_i2c_write( i2c_master_dev_handle_t dev_handle, uint32_t register_address, const uint8_t *data, uint16_t length )
{
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    err = core2foraws_i2c_write( COMMON_I2C_EXTERNAL, dev_handle,
                                 register_address, data, length );
    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_i2c_close( void )
{
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;

    err = _expports_pair_reset( I2C );

    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_adc_read( int *raw_adc_value )
{
    if( raw_adc_value == NULL ) return ESP_ERR_INVALID_ARG;
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    err = _core2foraws_expports_pin_handler( PORT_B_ADC_PIN, ADC );
    if ( err == ESP_OK )
    {
        err = adc_oneshot_read( _adc_handle, ADC_CHANNEL, raw_adc_value );
    }
    
    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_adc_mv_read( uint32_t *adc_mvolts )
{
    if( adc_mvolts == NULL ) return ESP_ERR_INVALID_ARG;
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    
    err = _core2foraws_expports_pin_handler( PORT_B_ADC_PIN, ADC );
    if ( err == ESP_OK )
    {
        int raw = 0;
        err = adc_oneshot_read( _adc_handle, ADC_CHANNEL, &raw );
        if ( err == ESP_OK && _adc_cali_handle != NULL )
        {
            int voltage = 0;
            err = adc_cali_raw_to_voltage( _adc_cali_handle, raw, &voltage );
            *adc_mvolts = ( uint32_t ) voltage;
        }
        else if( err == ESP_OK )
        {
            err = ESP_ERR_NOT_SUPPORTED;
        }
    }
    
    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_dac_mv_write( const uint16_t dac_mvolts )
{
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;

    err = _core2foraws_expports_pin_handler( PORT_B_DAC_PIN, DAC );
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

        err = dac_oneshot_output_voltage(_dac_handle, duty);
    }
    
    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_uart_begin( uint32_t baud )
{
    if( baud == 0 ) return ESP_ERR_INVALID_ARG;
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;

    err = _expports_pair_prepare( PORT_C_UART_RX_PIN, PORT_C_UART_TX_PIN, UART );
    if( err != ESP_OK )
    {
        _expports_unlock();
        return err;
    }
    err = _core2foraws_expports_pin_handler( PORT_C_UART_RX_PIN, UART );
    if ( err != ESP_OK )
    {
        _expports_unlock();
        return err;
    }

    err = _core2foraws_expports_pin_handler( PORT_C_UART_TX_PIN, UART );
    if ( err != ESP_OK )
    {
        _core2foraws_expports_pin_handler( PORT_C_UART_RX_PIN, NONE );
        _expports_unlock();
        return err;
    }

    const uart_config_t uart_config = 
    {
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    
    err = uart_set_pin( PORT_C_UART_NUM, PORT_C_UART_TX_PIN, PORT_C_UART_RX_PIN,
                        UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE );
    if( err == ESP_OK ) err = uart_param_config( PORT_C_UART_NUM, &uart_config );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "\tFailed to configure UART%d with the provided configuration.", PORT_C_UART_NUM );
        _core2foraws_expports_pin_handler( PORT_C_UART_RX_PIN, NONE );
        _core2foraws_expports_pin_handler( PORT_C_UART_TX_PIN, NONE );
    }    

    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_uart_read( uint8_t *message_buffer,
                                          size_t buffer_capacity,
                                          size_t *was_read_length )
{
    if( message_buffer == NULL || buffer_capacity == 0 ||
        was_read_length == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    int rxBytes = 0;
    size_t cached_buffer_length = 0;

    *was_read_length = 0;

    err = uart_get_buffered_data_len( PORT_C_UART_NUM, &cached_buffer_length );
    if ( err != ESP_OK )
    {
        ESP_LOGE( _TAG, "\tFailed to get UART ring buffer length. Check if pins were set to UART and has been configured." );
        _expports_unlock();
        return err;
    }

    if ( cached_buffer_length )
    {
        size_t to_read = cached_buffer_length < buffer_capacity
                     ? cached_buffer_length
                     : buffer_capacity;
        rxBytes = uart_read_bytes( PORT_C_UART_NUM, message_buffer, to_read,
                       pdMS_TO_TICKS( 1000 ) );
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
    else
    {
        err = ESP_OK;
    }
    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_uart_write( const char *message, size_t length, size_t *was_written_length )
{
    if( message == NULL || length == 0 || was_written_length == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    int txBytes = 0;

    txBytes = uart_write_bytes( PORT_C_UART_NUM, message, length );
    if ( txBytes == -1 )
    {
        err = ESP_ERR_INVALID_ARG;
        *was_written_length = 0;
    }
    else
    {
        err = ESP_OK;
        *was_written_length = ( size_t ) txBytes;
    }

    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_uart_send_finished( void )
{
    esp_err_t err = _expports_lock();
    if( err != ESP_OK ) return err;
    err = uart_wait_tx_done( PORT_C_UART_NUM,
                             pdMS_TO_TICKS( UART_TX_SEND_WAIT ) );
    _expports_unlock();
    return err;
}

esp_err_t core2foraws_expports_uart_read_flush( bool *was_flushed )
{
    if( was_flushed == NULL ) return ESP_ERR_INVALID_ARG;
    *was_flushed = false;
    esp_err_t lock_err = _expports_lock();
    if( lock_err != ESP_OK ) return lock_err;
    esp_err_t err = core2foraws_expports_uart_send_finished();
    if ( err == ESP_OK )
    {
        err = uart_flush_input( PORT_C_UART_NUM );
        if ( err == ESP_OK )
        {
            *was_flushed = true;
        }
    }
    _expports_unlock();
    return err;
}