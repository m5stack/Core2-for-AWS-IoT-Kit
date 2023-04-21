// Based on m5core2_axp by @ropg — Rop Gonggrijp
// https://github.com/ropg/m5core2_axp192

// Which was based on functions modified from / inpspired by @usedbytes - Brian Starkey's
// https://github.com/usedbytes/axp192

/*
 * Core2 for AWS IoT Kit BSP v2.0.0
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
 */

/**
 * @file core2foraws_power.c
 * @brief Core2 for AWS IoT Kit power management hardware driver APIs
 */

#include <stddef.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "axp192.h"
#include "i2c_manager.h"
#include "core2foraws_common.h"
#include "core2foraws_power.h"

#define AXP_I2C	i2c_hal( COMMON_I2C_INTERNAL )

static const char *_TAG = "CORE2FORAWS_POWER";

typedef struct 
{
    uint16_t min_millivolts;
    uint16_t max_millivolts;
    uint16_t step_millivolts;
    uint8_t vol_TAGe_reg;
    uint8_t vol_TAGe_lsb;
    uint8_t vol_TAGe_mask;
} axp192_rail_cfg_t;

static const axp192_rail_cfg_t _axp192_rail_configs[] = 
{
    [ POWER_RAIL_DCDC1 ] =
    {
        .min_millivolts = 700,
        .max_millivolts = 3500,
        .step_millivolts = 25,
        .vol_TAGe_reg = AXP192_DCDC1_VOLTAGE,
        .vol_TAGe_lsb = 0,
        .vol_TAGe_mask = (1 << 7) - 1,
    },
    [ POWER_RAIL_DCDC2 ] =
    {
        .min_millivolts = 700,
        .max_millivolts = 2275,
        .step_millivolts = 25,
        .vol_TAGe_reg = AXP192_DCDC2_VOLTAGE,
        .vol_TAGe_lsb = 0,
        .vol_TAGe_mask = (1 << 6) - 1,
    },
    [ POWER_RAIL_DCDC3 ] =
    {
        .min_millivolts = 700,
        .max_millivolts = 3500,
        .step_millivolts = 25,
        .vol_TAGe_reg = AXP192_DCDC3_VOLTAGE,
        .vol_TAGe_lsb = 0,
        .vol_TAGe_mask = (1 << 7) - 1,
    },
    [ POWER_RAIL_LDO2 ] = 
    {
        .min_millivolts = 1800,
        .max_millivolts = 3300,
        .step_millivolts = 100,
        .vol_TAGe_reg = AXP192_LDO23_VOLTAGE,
        .vol_TAGe_lsb = 4,
        .vol_TAGe_mask = 0xf0,
    },
    [ POWER_RAIL_LDO3 ] = 
    {
        .min_millivolts = 1800,
        .max_millivolts = 3300,
        .step_millivolts = 100,
        .vol_TAGe_reg = AXP192_LDO23_VOLTAGE,
        .vol_TAGe_lsb = 0,
        .vol_TAGe_mask = 0x0f,
    },
};

static esp_err_t _core2foraws_power_int_5v_enable( bool state );

esp_err_t core2foraws_power_init( void ) 
{
    ESP_LOGI( _TAG, "\tInitializing" );

    // turn off everything except bit 2 and then turn bit 1 on
    if ( core2foraws_power_axp_twiddle( AXP192_VBUS_IPSOUT_CHANNEL, 0b11111011, 0x02 ) == ESP_OK ) 
    {
        ESP_LOGI(_TAG, "\tVbus limit off");
    }

    if ( core2foraws_power_axp_twiddle( AXP192_GPIO2_CONTROL, 0b00000111, 0x00 ) == ESP_OK &&
        core2foraws_power_speaker_enable( false ) == ESP_OK ) 
    {
        ESP_LOGI(_TAG, "\tSpeaker amplifier off");
    }

    if ( core2foraws_power_axp_twiddle( AXP192_BATTERY_CHARGE_CONTROL, 0b11100011, 0b10100010 ) == ESP_OK ) 
    {
        ESP_LOGI( _TAG, "\tRTC battery charging enabled (3v, 200uA)" );
    }

    if ( core2foraws_power_rail_mv_set( POWER_RAIL_ESP32, 3350 ) == ESP_OK &&
        core2foraws_power_rail_state_set( POWER_RAIL_ESP32, true ) == ESP_OK ) 
    {
        ESP_LOGI( _TAG, "\tESP32 power vol_TAGe set to 3.35v" );
    }

    if ( core2foraws_power_backlight_set( DISPLAY_BACKLIGHT_START )  == ESP_OK ) 
    {
        ESP_LOGI( _TAG, "\tDisplay backlight level set to %d%%", DISPLAY_BACKLIGHT_START );
    }

    if ( core2foraws_power_rail_mv_set( POWER_RAIL_LOGIC_AND_SD, 3300 ) == ESP_OK &&
        core2foraws_power_rail_state_set( POWER_RAIL_LOGIC_AND_SD, true ) == ESP_OK ) {
        ESP_LOGI( _TAG, "\tDisplay logic and sdcard vol_TAGe set to 3.3v" );
    }

    if ( core2foraws_power_rail_mv_set( POWER_RAIL_VIBRATOR, 2000) == ESP_OK ) {
        ESP_LOGI( _TAG, "\tVibrator vol_TAGe preset to 2v" );
    }

    if (core2foraws_power_axp_twiddle( AXP192_GPIO1_CONTROL, 0x07, 0x00 ) == ESP_OK &&
        core2foraws_power_led_enable( true ) == ESP_OK ) 
    {
        ESP_LOGI( _TAG, "\tGreen LED on" );
    }

    if (core2foraws_power_axp_twiddle( AXP192_CHARGE_CONTROL_1, 0x0f, 0x00 ) == ESP_OK ) 
    {
        ESP_LOGI( _TAG, "\tCharge current set to 100 mA" );
    }

	float volts;
	if (core2foraws_power_axp_read( AXP192_BATTERY_VOLTAGE, &volts ) == ESP_OK) 
    {
		ESP_LOGI( _TAG, "\tBattery vol_TAGe now: %.2f volts", volts );
    }

    if ( core2foraws_power_axp_twiddle( AXP192_PEK, 0xff, 0x4c ) == ESP_OK ) 
    {
    	ESP_LOGI( _TAG, "\tPower key set, 4 seconds for hard shutdown" );
    }

    if ( core2foraws_power_axp_twiddle( AXP192_ADC_ENABLE_1, 0x00, 0xff ) == ESP_OK ) 
    {
    	ESP_LOGI( _TAG, "\tEnabled all ADC channels" );
    }

    if ( _core2foraws_power_int_5v_enable( true ) == ESP_OK ) 
    {
    	ESP_LOGI( _TAG, "\tUSB / battery powered, 5V bus on" );
    }
	
	// GPIO4 is reset for LCD and touch
	core2foraws_power_axp_twiddle( AXP192_GPIO43_FUNCTION_CONTROL, ~0x72, 0x84 );
    core2foraws_power_axp_twiddle( AXP192_GPIO43_SIGNAL_STATUS, 0x02, 0x00 );
    vTaskDelay( pdMS_TO_TICKS ( 100 ) );
    if ( core2foraws_power_axp_twiddle( AXP192_GPIO43_SIGNAL_STATUS, 0x02, 0x02 ) == ESP_OK )
    {
    	ESP_LOGI( _TAG, "\tDisplay and touch reset" );
    }
    vTaskDelay( pdMS_TO_TICKS( 300 ) );
    
    return ESP_OK;
}

static esp_err_t _core2foraws_power_int_5v_enable( bool state ) 
{

	// To enable the on-board 5V supply, first N_VBUSEN needs to be pulled
	// high using GPIO0, then we can enable the EXTEN output, to enable
	// the SMPS.
	// To disable it (so either no 5V, or externally supplied 5V), we
	// do the opposite: First disable EXTEN, then leave GPIO0 floating.
	// N_VBUSEN will be pulled down by the on-board resistor.
	// Side note: The pull down is 10k according to the schematic, so that's
	// a 0.5 mA drain from the GPIO0 LDO as long as the bus supply is active.
	
	esp_err_t ret = ESP_OK;

	if ( state )
    {
		ret |= core2foraws_power_axp_twiddle( AXP192_GPIO0_LDOIO0_VOLTAGE, 0xf0, 0xf0 );
		ret |= core2foraws_power_axp_twiddle( AXP192_GPIO0_CONTROL, 0x07, 0x02 );
		ret |= core2foraws_power_rail_state_set( POWER_RAIL_EXTEN, true );
	} 
    else
    {
		ret |= core2foraws_power_rail_state_set( POWER_RAIL_EXTEN, false );
		ret |= core2foraws_power_axp_twiddle( AXP192_GPIO0_CONTROL, 0x07, 0x01 );
	}
	return ret;
}

esp_err_t core2foraws_power_backlight_set( uint8_t brightness )
{
    esp_err_t err = ESP_OK;

    if ( brightness > 100 )
    {
        brightness = 100;
    }

    uint16_t volts = ( uint32_t )brightness * ( DISPLAY_BACKLIGHT_MAX_VOLTS - DISPLAY_BACKLIGHT_MIN_VOLTS ) / 100 + DISPLAY_BACKLIGHT_MIN_VOLTS;
    ESP_LOGD( _TAG, "\tDISPLAY VOL_TAGE %d", volts );

    err |= core2foraws_power_rail_mv_set( POWER_RAIL_DISPLAY_BACKLIGHT, volts );
    err |= core2foraws_power_rail_state_set( POWER_RAIL_DISPLAY_BACKLIGHT, true );

    return err;
}

esp_err_t core2foraws_power_led_enable( bool state )
{
    return core2foraws_power_axp_twiddle( AXP192_GPIO20_SIGNAL_STATUS, 0x02, state ? 0x00 : 0x02 );
}

esp_err_t core2foraws_power_vibration_enable( bool state )
{
	return core2foraws_power_rail_state_set( POWER_RAIL_LDO3, state );
}

esp_err_t core2foraws_power_speaker_enable( bool state )
{
    return core2foraws_power_axp_twiddle( AXP192_GPIO20_SIGNAL_STATUS, 0x04, state ? 0x04 : 0x00 );
}

esp_err_t core2foraws_power_batt_volts_get( float *volts )
{
    axp192_t *ptr = ( axp192_t * )AXP_I2C;
    return axp192_read( ptr, AXP192_BATTERY_VOLTAGE, ( void * ) volts );
}

esp_err_t core2foraws_power_batt_current_get( float *m_amps )
{
    esp_err_t ret = ESP_OK;
    axp192_t *ptr = ( axp192_t * )AXP_I2C;
    float current_in, current_out = 0.00;

    ret |= axp192_read( ptr, AXP192_CHARGE_CURRENT, ( void * ) &current_in );
    ret |= axp192_read( ptr, AXP192_DISCHARGE_CURRENT, ( void * ) &current_out );

    *m_amps = ( current_in - current_out );

    return core2foraws_common_error( ret );
}

esp_err_t core2foraws_power_charging_get( bool *status )
{
    esp_err_t ret = ESP_OK;
    uint8_t reg_val = 0x00;
    axp192_t *ptr = ( axp192_t * )AXP_I2C;

    ret |= axp192_read( ptr, AXP192_CHARGE_STATUS, ( void * ) &reg_val );

    *status = ( ( reg_val >> 6 ) & 1U );

    return ret;
}

esp_err_t core2foraws_power_plugged_get( bool *status )
{
    esp_err_t ret = ESP_OK;
    uint8_t reg_val = 0x00;
    axp192_t *ptr = ( axp192_t * )AXP_I2C;

    ret |= axp192_read( ptr, AXP192_POWER_STATUS, ( void * ) &reg_val );

    *status = ( ( reg_val >> 7 ) & 1U );

    return ret;
}

esp_err_t core2foraws_power_axp_reg_get( uint8_t reg, uint8_t *buffer )
{
	axp192_t *ptr = ( axp192_t * )AXP_I2C;
	return ptr->read(ptr->handle, AXP192_ADDRESS, reg, ( void * )buffer, 1);
}

esp_err_t core2foraws_power_axp_reg_set( uint8_t reg, uint8_t value )
{
	axp192_t *ptr = ( axp192_t * )AXP_I2C;
	uint8_t buffer = value;
	return ptr->write( ptr->handle, AXP192_ADDRESS, reg, &buffer, 1 );
}

esp_err_t core2foraws_power_axp_read( uint8_t reg, void *buffer )
{
	axp192_t *ptr = ( axp192_t * )AXP_I2C;
	return axp192_read( ptr, reg, buffer );
}

esp_err_t core2foraws_power_axp_write( uint8_t reg, const uint8_t *buffer )
{
	axp192_t *ptr = ( axp192_t * )AXP_I2C;
	return axp192_write( ptr, reg, buffer );
}

esp_err_t core2foraws_power_axp_twiddle( uint8_t reg, uint8_t affect, uint8_t value )
{
	esp_err_t ret;
	uint8_t buffer;
	ret = core2foraws_power_axp_reg_get( reg, &buffer );
	if ( ret == ESP_OK )
    {
		buffer &= ~affect;
		buffer |= (value & affect);
		ret = core2foraws_power_axp_reg_set( reg, buffer );
	}
	return ret;
}

esp_err_t core2foraws_power_rail_state_get( power_rail_t rail, bool *enabled )
{
    esp_err_t ret;
    uint8_t val;

    ret = core2foraws_power_axp_reg_get( AXP192_DCDC13_LDO23_CONTROL, &val );
    if ( ret != ESP_OK )
    {
        return ret;
    }

    switch ( rail )
    {
        case POWER_RAIL_DCDC1:
            *enabled = !!( val & ( 1 << 0 ) );
            break;
        case POWER_RAIL_DCDC2:
            *enabled = !!( val & ( 1 << 4 ) );
            break;
        case POWER_RAIL_DCDC3:
            *enabled = !!( val & ( 1 << 1 ) );
            break;
        case POWER_RAIL_LDO2:
            *enabled = !!( val & ( 1 << 2 ) );
            break;
        case POWER_RAIL_LDO3:
            *enabled = !!( val & ( 1 << 3 ) );
            break;
        case POWER_RAIL_EXTEN:
            *enabled = !!( val & ( 1 << 6 ) );
            break;
        default:
            return ESP_ERR_INVALID_ARG;
    }

    return ESP_OK;
}

esp_err_t core2foraws_power_rail_state_set( power_rail_t rail, bool enabled )
{
    esp_err_t ret;
    uint8_t val;
    uint8_t mask;

    ret = core2foraws_power_axp_reg_get( AXP192_DCDC13_LDO23_CONTROL, &val );
    if ( ret != ESP_OK )
    {
        return ret;
    }

    switch ( rail )
    {
        case POWER_RAIL_DCDC1:
            mask = ( 1 << 0 );
            break;
        case POWER_RAIL_DCDC2:
            mask = ( 1 << 4 );
            break;
        case POWER_RAIL_DCDC3:
            mask = ( 1 << 1 );
            break;
        case POWER_RAIL_LDO2:
            mask = ( 1 << 2 );
            break;
        case POWER_RAIL_LDO3:
            mask = ( 1 << 3 );
            break;
        case POWER_RAIL_EXTEN:
            mask = ( 1 << 6 );
            break;
        default:
            return ESP_ERR_INVALID_ARG;
    }

    if ( enabled )
    {
        val |= mask;
    }
    else
    {
        val = val & ~mask;
    }

    ret = core2foraws_power_axp_reg_set( AXP192_DCDC13_LDO23_CONTROL, val );
    if ( ret != ESP_OK )
    {
        return ret;
    }

    return ESP_OK;
}

esp_err_t core2foraws_power_rail_mv_get( power_rail_t rail, uint16_t *millivolts )
{
    esp_err_t ret;
    uint8_t val;

    if ( ( rail < POWER_RAIL_DCDC1 ) || ( rail >= POWER_RAIL_COUNT ) ) {
        return ESP_ERR_INVALID_ARG;
    }

    const axp192_rail_cfg_t *cfg = &_axp192_rail_configs[ rail ];
    if ( cfg->step_millivolts == 0 ) {
        return ESP_ERR_INVALID_ARG;
    }

    ret = core2foraws_power_axp_reg_get( cfg->vol_TAGe_reg, &val );
    if ( ret != ESP_OK ) {
        return ret;
    }

    val = ( val & cfg->vol_TAGe_mask ) >> cfg->vol_TAGe_lsb;

    *millivolts = cfg->min_millivolts + cfg->step_millivolts * val;

    return ESP_OK;
}

esp_err_t core2foraws_power_rail_mv_set( power_rail_t rail, uint16_t millivolts )
{

    esp_err_t ret;
    uint8_t val, steps;

    if ( ( rail < POWER_RAIL_DCDC1 ) || ( rail >= POWER_RAIL_COUNT ) )
    {
        return ESP_ERR_INVALID_ARG;
    }

    const axp192_rail_cfg_t *cfg = &_axp192_rail_configs[ rail ];
    if ( cfg->step_millivolts == 0 )
    {
        return ESP_ERR_INVALID_ARG;
    }

    if ( ( millivolts < cfg->min_millivolts ) || ( millivolts > cfg->max_millivolts ) )
    {
        return ESP_ERR_INVALID_ARG;
    }

    ret = core2foraws_power_axp_reg_get(cfg->vol_TAGe_reg, &val);
    if (ret != ESP_OK)
    {
        return ret;
    }

    steps = ( millivolts - cfg->min_millivolts ) / cfg->step_millivolts;
    val = ( val & ~( cfg->vol_TAGe_mask ) ) | ( steps << cfg->vol_TAGe_lsb );

    ret = core2foraws_power_axp_reg_set( cfg->vol_TAGe_reg, val );
    if ( ret != ESP_OK )
    {
        return ret;
    }

    return ESP_OK;
}
