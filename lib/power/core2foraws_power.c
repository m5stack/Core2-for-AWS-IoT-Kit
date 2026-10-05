// Based on m5core2_axp by @ropg — Rop Gonggrijp
// https://github.com/ropg/m5core2_axp192

// Which was based on functions modified from / inpspired by @usedbytes - Brian Starkey's
// https://github.com/usedbytes/axp192

/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_power.c
 * @brief Core2 for AWS IoT Kit power management hardware driver APIs
 */

#include <stddef.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_timer.h>

#include "axp192.h"
#include "core2foraws_common.h"
#include "core2foraws_power.h"

#define LCD_RESET_LOW_MS  100
#define LCD_RESET_BOOT_MS 300

static const char *_TAG = "CORE2FORAWS_POWER";

static i2c_master_dev_handle_t _axp192_dev = NULL;
/* esp_timer time at which the LCD/touch controllers may be used; 0 = none */
static int64_t _lcd_ready_us = 0;

static int32_t _axp192_i2c_read( void *handle, uint8_t address, uint8_t reg,
                                  uint8_t *buffer, uint16_t size )
{
    (void)handle;
    (void)address;
    return ( int32_t )core2foraws_i2c_read( COMMON_I2C_INTERNAL, _axp192_dev,
                                            ( uint32_t )reg, buffer, size );
}

static int32_t _axp192_i2c_write( void *handle, uint8_t address, uint8_t reg,
                                   const uint8_t *buffer, uint16_t size )
{
    (void)handle;
    (void)address;
        if( buffer == NULL || size == 0 || size > 256U - reg )
        {
                return ESP_ERR_INVALID_ARG;
        }
        for( uint16_t offset = 0; offset < size; ++offset )
        {
                uint16_t target_reg = reg + offset;
                uint8_t value = buffer[ offset ];
                if( ( target_reg == AXP192_DCDC13_LDO23_CONTROL &&
                            ( value & 0x01U ) == 0 ) ||
                        ( target_reg == AXP192_DCDC1_VOLTAGE &&
                            ( value & 0x7fU ) != ( POWER_MCU_MILLIVOLTS - 700U ) / 25U ) )
                {
                        return ESP_ERR_NOT_SUPPORTED;
                }
        }
    return ( int32_t )core2foraws_i2c_write( COMMON_I2C_INTERNAL, _axp192_dev,
                                             ( uint32_t )reg, buffer, size );
}

static const axp192_t _axp = {
    .read = _axp192_i2c_read,
    .write = _axp192_i2c_write,
    .handle = NULL,
};

typedef struct 
{
    uint16_t min_millivolts;
    uint16_t max_millivolts;
    uint16_t step_millivolts;
    uint8_t voltage_reg;
    uint8_t voltage_lsb;
    uint8_t voltage_mask;
} axp192_rail_cfg_t;

static const axp192_rail_cfg_t _axp192_rail_configs[ POWER_RAIL_COUNT ] =
{
    [ POWER_RAIL_DCDC1 ] =
    {
        .min_millivolts = 700,
        .max_millivolts = 3500,
        .step_millivolts = 25,
        .voltage_reg = AXP192_DCDC1_VOLTAGE,
        .voltage_lsb = 0,
        .voltage_mask = (1 << 7) - 1,
    },
    [ POWER_RAIL_DCDC2 ] =
    {
        .min_millivolts = 700,
        .max_millivolts = 2275,
        .step_millivolts = 25,
        .voltage_reg = AXP192_DCDC2_VOLTAGE,
        .voltage_lsb = 0,
        .voltage_mask = (1 << 6) - 1,
    },
    [ POWER_RAIL_DCDC3 ] =
    {
        .min_millivolts = 700,
        .max_millivolts = 3500,
        .step_millivolts = 25,
        .voltage_reg = AXP192_DCDC3_VOLTAGE,
        .voltage_lsb = 0,
        .voltage_mask = (1 << 7) - 1,
    },
    [ POWER_RAIL_LDO2 ] = 
    {
        .min_millivolts = 1800,
        .max_millivolts = 3300,
        .step_millivolts = 100,
        .voltage_reg = AXP192_LDO23_VOLTAGE,
        .voltage_lsb = 4,
        .voltage_mask = 0xf0,
    },
    [ POWER_RAIL_LDO3 ] = 
    {
        .min_millivolts = 1800,
        .max_millivolts = 3300,
        .step_millivolts = 100,
        .voltage_reg = AXP192_LDO23_VOLTAGE,
        .voltage_lsb = 0,
        .voltage_mask = 0x0f,
    },
};

static esp_err_t _core2foraws_power_int_5v_enable( bool state );
static bool _power_initialized = false;

esp_err_t core2foraws_power_init( void ) 
{
    ESP_LOGI( _TAG, "\tInitializing" );

    if( _power_initialized )
    {
        return ESP_OK;
    }

    /* Register AXP192 device on the internal I2C bus. The AXP192 supports
       400 kHz fast mode, which shortens how long each transfer holds the
       shared bus. */
    if( _axp192_dev == NULL )
    {
        esp_err_t err = core2foraws_i2c_device_add( COMMON_I2C_INTERNAL,
                                                     AXP192_ADDRESS, 400000,
                                                     &_axp192_dev );
        if( err != ESP_OK )
        {
            ESP_LOGE( _TAG, "Failed to add AXP192 I2C device: 0x%x", err );
            return err;
        }
    }

    /* Accumulate the first failure across the bring-up steps so a failed PMU
       configuration is reported to the caller instead of being silently
       swallowed. */
    esp_err_t ret = ESP_OK;
    esp_err_t err;

    /* REG30H: disable VHOLD pass-through limiting and enable the VBUS
       current limit. Reserved bit 2 is preserved; the current-limit value
       bit (bit 0) is left cleared, which selects the 500mA limit. */
    err = core2foraws_power_axp_twiddle( AXP192_VBUS_IPSOUT_CHANNEL,
            (uint8_t)~AXP192_STATUS_BAT_DIRECTION, /* preserve reserved bit 2 */
            AXP192_VBUS_CTL_CUR_LIMIT_EN );
    if ( err == ESP_OK )
        ESP_LOGI(_TAG, "\tVBUS current limit configured");
    else if ( ret == ESP_OK )
        ret = err;

    /* Set GPIO2 to NMOS open-drain output mode (bits[2:0]=0),
       then pull speaker enable low to disable the amplifier. */
    err = core2foraws_power_axp_twiddle( AXP192_GPIO2_CONTROL, AXP192_GPIO_MODE_MASK, 0x00 );
    if ( err == ESP_OK )
        err = core2foraws_power_speaker_enable( false );
    if ( err == ESP_OK )
        ESP_LOGI(_TAG, "\tSpeaker amplifier off");
    else if ( ret == ESP_OK )
        ret = err;

    /* Enable RTC backup battery charging at 3.0V, 200uA.
       REG35H: bit7=enable, bits[6:5]=voltage, bits[1:0]=current */
    err = core2foraws_power_axp_twiddle( AXP192_BATTERY_CHARGE_CONTROL,
            AXP192_BACKUP_CHG_ENABLE | AXP192_BACKUP_VOLT_MASK | AXP192_BACKUP_CUR_MASK,
            AXP192_BACKUP_CHG_ENABLE
            | (AXP192_BACKUP_VOLT_3V0 << AXP192_BACKUP_VOLT_SHIFT)
            | (AXP192_BACKUP_CUR_200UA << AXP192_BACKUP_CUR_SHIFT) );
    if ( err == ESP_OK )
        ESP_LOGI( _TAG, "\tRTC battery charging enabled (3.0V, 200uA)" );
    else if ( ret == ESP_OK )
        ret = err;

    err = core2foraws_power_rail_mv_set( POWER_RAIL_ESP32, POWER_MCU_MILLIVOLTS );
    if ( err == ESP_OK )
        err = core2foraws_power_rail_state_set( POWER_RAIL_ESP32, true );
    if ( err == ESP_OK )
        ESP_LOGI( _TAG, "\tESP32 power voltage set to 3.35V" );
    else if ( ret == ESP_OK )
        ret = err;

    err = core2foraws_power_backlight_set( DISPLAY_BACKLIGHT_START );
    if ( err == ESP_OK )
        ESP_LOGI( _TAG, "\tDisplay backlight level set to %d%%", DISPLAY_BACKLIGHT_START );
    else if ( ret == ESP_OK )
        ret = err;

    err = core2foraws_power_rail_mv_set( POWER_RAIL_LOGIC_AND_SD, 3300 );
    if ( err == ESP_OK )
        err = core2foraws_power_rail_state_set( POWER_RAIL_LOGIC_AND_SD, true );
    if ( err == ESP_OK )
        ESP_LOGI( _TAG, "\tDisplay logic and SD card voltage set to 3.3V" );
    else if ( ret == ESP_OK )
        ret = err;

    err = core2foraws_power_rail_mv_set( POWER_RAIL_VIBRATOR, 2000 );
    if ( err == ESP_OK )
        ESP_LOGI( _TAG, "\tVibrator voltage preset to 2.0V" );
    else if ( ret == ESP_OK )
        ret = err;

    /* Set GPIO1 to NMOS open-drain output mode for green LED control. */
    err = core2foraws_power_axp_twiddle( AXP192_GPIO1_CONTROL, AXP192_GPIO_MODE_MASK, 0x00 );
    if ( err == ESP_OK )
        err = core2foraws_power_led_enable( true );
    if ( err == ESP_OK )
        ESP_LOGI( _TAG, "\tGreen LED on" );
    else if ( ret == ESP_OK )
        ret = err;

    /* Set charge current to 100mA (bits[3:0] = 0 = lowest setting). */
    err = core2foraws_power_axp_twiddle( AXP192_CHARGE_CONTROL_1, AXP192_CHG1_CURRENT_MASK, 0x00 );
    if ( err == ESP_OK )
        ESP_LOGI( _TAG, "\tCharge current set to 100 mA" );
    else if ( ret == ESP_OK )
        ret = err;

    float volts;
    if (core2foraws_power_axp_read( AXP192_BATTERY_VOLTAGE, &volts ) == ESP_OK)
    {
        ESP_LOGI( _TAG, "\tBattery voltage now: %.2f volts", volts );
    }

    /* PEK (power key) config: 0x4c = 128ms startup, 4s long-press shutdown,
       1s shutdown delay, power-key auto-shutdown enabled. */
    err = core2foraws_power_axp_twiddle( AXP192_PEK, 0xff, 0x4c );
    if ( err == ESP_OK )
        ESP_LOGI( _TAG, "\tPower key set, 4 seconds for hard shutdown" );
    else if ( ret == ESP_OK )
        ret = err;

    /* Enable all ADC channels: battery, ACIN, VBUS, APS, TS voltages & currents. */
    err = core2foraws_power_axp_twiddle( AXP192_ADC_ENABLE_1, 0xff, 0xff );
    if ( err == ESP_OK )
        ESP_LOGI( _TAG, "\tEnabled all ADC channels" );
    else if ( ret == ESP_OK )
        ret = err;

    err = _core2foraws_power_int_5v_enable( true );
    if ( err == ESP_OK )
        ESP_LOGI( _TAG, "\tUSB / battery powered, 5V bus on" );
    else if ( ret == ESP_OK )
        ret = err;

    /* Configure GPIO4 as NMOS open-drain for LCD/touch reset control.
       REG95H: bit7 = GPIO3/4 function enable, bits[3:2] = GPIO4 mode.
       0x84 = enable GPIO3/4 functions, GPIO4 = NMOS output. */
    err = core2foraws_power_axp_twiddle( AXP192_GPIO43_FUNCTION_CONTROL, (uint8_t)~0x72, 0x84 );
    if ( err != ESP_OK && ret == ESP_OK )
        ret = err;

    /* Pull GPIO4 low to reset display and touch controller. */
    err = core2foraws_power_axp_twiddle( AXP192_GPIO43_SIGNAL_STATUS, 0x02, 0x00 );
    if ( err != ESP_OK && ret == ESP_OK )
        ret = err;
    vTaskDelay( CORE2FORAWS_DELAY_MS_TO_TICKS( LCD_RESET_LOW_MS ) );

    /* Release reset by pulling GPIO4 high. The controllers boot while the
       remaining peripherals initialize; the display waits out the rest. */
    err = core2foraws_power_axp_twiddle( AXP192_GPIO43_SIGNAL_STATUS, 0x02, 0x02 );
    if ( err == ESP_OK )
    {
    	ESP_LOGI( _TAG, "\tDisplay and touch reset" );
        _lcd_ready_us = esp_timer_get_time() + LCD_RESET_BOOT_MS * 1000;
    }
    else if ( ret == ESP_OK )
        ret = err;
    
    if( ret == ESP_OK )
    {
        _power_initialized = true;
    }

    return ret;
}

void core2foraws_power_lcd_ready_wait( void )
{
    int64_t remaining_us = _lcd_ready_us - esp_timer_get_time();
    if( _lcd_ready_us != 0 && remaining_us > 0 )
    {
        ESP_LOGD( _TAG, "\tWaiting %lld us for LCD/touch start-up",
                  ( long long ) remaining_us );
        vTaskDelay( CORE2FORAWS_DELAY_MS_TO_TICKS( ( remaining_us + 999 ) / 1000 ) );
    }
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
		ret = core2foraws_power_axp_twiddle( AXP192_GPIO0_LDOIO0_VOLTAGE, 0xf0, 0xf0 );
		if ( ret != ESP_OK )
        {
			return ret;
        }

		ret = core2foraws_power_axp_twiddle( AXP192_GPIO0_CONTROL, 0x07, 0x02 );
		if ( ret != ESP_OK )
        {
			return ret;
        }

		ret = core2foraws_power_rail_state_set( POWER_RAIL_EXTEN, true );
	} 
    else
    {
		ret = core2foraws_power_rail_state_set( POWER_RAIL_EXTEN, false );
		if ( ret != ESP_OK )
        {
			return ret;
        }

		ret = core2foraws_power_axp_twiddle( AXP192_GPIO0_CONTROL, 0x07, 0x01 );
	}
	return ret;
}

esp_err_t core2foraws_power_backlight_set( uint8_t brightness )
{
    if ( brightness > 100 )
    {
        brightness = 100;
    }

    if ( brightness == 0 )
    {
        return core2foraws_power_rail_state_set( POWER_RAIL_DISPLAY_BACKLIGHT, false );
    }

    uint16_t volts = ( uint32_t )brightness * ( DISPLAY_BACKLIGHT_MAX_VOLTS - DISPLAY_BACKLIGHT_MIN_VOLTS ) / 100 + DISPLAY_BACKLIGHT_MIN_VOLTS;
    /* AXP192 DCDC3 steps in 25 mV increments from a 700 mV base.
     * Round down to the nearest valid step so the rail driver does not
     * reject the voltage with ESP_ERR_INVALID_ARG. */
    volts -= ( volts - 700U ) % 25U;
    ESP_LOGD( _TAG, "\tDisplay backlight voltage: %d mV", volts );

    esp_err_t err = core2foraws_power_rail_mv_set( POWER_RAIL_DISPLAY_BACKLIGHT, volts );
    if ( err != ESP_OK )
    {
        return err;
    }

    return core2foraws_power_rail_state_set( POWER_RAIL_DISPLAY_BACKLIGHT, true );
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
    if ( volts == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    return core2foraws_common_error( axp192_read( &_axp, AXP192_BATTERY_VOLTAGE, ( void * ) volts ) );
}

esp_err_t core2foraws_power_batt_current_get( float *m_amps )
{
    if ( m_amps == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    float current_in, current_out = 0.00;

    esp_err_t ret = axp192_read( &_axp, AXP192_CHARGE_CURRENT, ( void * ) &current_in );
    if ( ret != ESP_OK )
    {
        return core2foraws_common_error( ret );
    }

    ret = axp192_read( &_axp, AXP192_DISCHARGE_CURRENT, ( void * ) &current_out );
    if ( ret != ESP_OK )
    {
        return core2foraws_common_error( ret );
    }

    /* axp192_read_adc returns current in amps; convert to milliamps. */
    *m_amps = ( current_in - current_out ) * 1000.0f;

    return ESP_OK;
}

esp_err_t core2foraws_power_charging_get( bool *status )
{
    if ( status == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t reg_val = 0x00;

    esp_err_t ret = axp192_read( &_axp, AXP192_CHARGE_STATUS, ( void * ) &reg_val );
    if ( ret != ESP_OK )
    {
        return core2foraws_common_error( ret );
    }

    *status = ( ( reg_val >> 6 ) & 1U );

    return ESP_OK;
}

esp_err_t core2foraws_power_plugged_get( bool *status )
{
    if ( status == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t reg_val = 0x00;

    esp_err_t ret = axp192_read( &_axp, AXP192_POWER_STATUS, ( void * ) &reg_val );
    if ( ret != ESP_OK )
    {
        return core2foraws_common_error( ret );
    }

    *status = ( ( reg_val >> 7 ) & 1U );

    return ESP_OK;
}

esp_err_t core2foraws_power_off( void )
{
    return core2foraws_power_axp_twiddle( AXP192_SHUTDOWN_BATTERY_CHGLED_CONTROL,
                                          AXP192_POWER_OFF_REQUEST,
                                          AXP192_POWER_OFF_REQUEST );
}

esp_err_t core2foraws_power_axp_reg_get( uint8_t reg, uint8_t *buffer )
{
	return core2foraws_i2c_read( COMMON_I2C_INTERNAL, _axp192_dev,
	                            ( uint32_t )reg, buffer, 1 );
}

esp_err_t core2foraws_power_axp_reg_set( uint8_t reg, uint8_t value )
{
    return ( esp_err_t )_axp192_i2c_write( NULL, AXP192_ADDRESS, reg, &value, 1 );
}

esp_err_t core2foraws_power_axp_read( uint8_t reg, void *buffer )
{
	return axp192_read( &_axp, reg, buffer );
}

esp_err_t core2foraws_power_axp_write( uint8_t reg, const uint8_t *buffer )
{
	return axp192_write( &_axp, reg, buffer );
}

esp_err_t core2foraws_power_axp_twiddle( uint8_t reg, uint8_t affect, uint8_t value )
{
    esp_err_t ret = core2foraws_i2c_lock( COMMON_I2C_INTERNAL );
    if( ret != ESP_OK )
    {
        return ret;
    }
	uint8_t buffer;
	ret = core2foraws_power_axp_reg_get( reg, &buffer );
	if ( ret == ESP_OK )
    {
		buffer &= ~affect;
		buffer |= (value & affect);
		ret = core2foraws_power_axp_reg_set( reg, buffer );
	}
    esp_err_t unlock_err = core2foraws_i2c_unlock( COMMON_I2C_INTERNAL );
    return ret != ESP_OK ? ret : unlock_err;
}

esp_err_t core2foraws_power_rail_state_get( power_rail_t rail, bool *enabled )
{
    if ( enabled == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

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
        /* LDO1 is the always-on RTC rail; it has no enable bit. */
        case POWER_RAIL_LDO1:
            *enabled = true;
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
    if( rail == POWER_RAIL_ESP32 && !enabled )
    {
        return ESP_ERR_NOT_SUPPORTED;
    }
    uint8_t mask;

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
        /* LDO1 powers the RTC domain and cannot be switched off. */
        case POWER_RAIL_LDO1:
            return ESP_ERR_NOT_SUPPORTED;
        default:
            return ESP_ERR_INVALID_ARG;
    }

    return core2foraws_power_axp_twiddle( AXP192_DCDC13_LDO23_CONTROL,
                                          mask, enabled ? mask : 0 );
}

esp_err_t core2foraws_power_rail_mv_get( power_rail_t rail, uint16_t *millivolts )
{
    if ( millivolts == NULL )
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;
    uint8_t val;

    if ( ( rail < POWER_RAIL_DCDC1 ) || ( rail >= POWER_RAIL_COUNT ) ) {
        return ESP_ERR_INVALID_ARG;
    }

    const axp192_rail_cfg_t *cfg = &_axp192_rail_configs[ rail ];
    if ( cfg->step_millivolts == 0 ) {
        /* LDO1 is fixed and EXTEN is a switch, so neither is adjustable. */
        return ESP_ERR_NOT_SUPPORTED;
    }

    ret = core2foraws_power_axp_reg_get( cfg->voltage_reg, &val );
    if ( ret != ESP_OK ) {
        return ret;
    }

    val = ( val & cfg->voltage_mask ) >> cfg->voltage_lsb;

    *millivolts = cfg->min_millivolts + cfg->step_millivolts * val;

    return ESP_OK;
}

esp_err_t core2foraws_power_rail_mv_set( power_rail_t rail, uint16_t millivolts )
{
    if( rail == POWER_RAIL_ESP32 && millivolts != POWER_MCU_MILLIVOLTS )
    {
        return ESP_ERR_NOT_SUPPORTED;
    }
    uint8_t steps;

    if ( ( rail < POWER_RAIL_DCDC1 ) || ( rail >= POWER_RAIL_COUNT ) )
    {
        return ESP_ERR_INVALID_ARG;
    }

    const axp192_rail_cfg_t *cfg = &_axp192_rail_configs[ rail ];
    if ( cfg->step_millivolts == 0 )
    {
        /* LDO1 is fixed and EXTEN is a switch, so neither is adjustable. */
        return ESP_ERR_NOT_SUPPORTED;
    }

    if ( ( millivolts < cfg->min_millivolts ) || ( millivolts > cfg->max_millivolts ) )
    {
        return ESP_ERR_INVALID_ARG;
    }

    if ( ( ( millivolts - cfg->min_millivolts ) % cfg->step_millivolts ) != 0 )
    {
        return ESP_ERR_INVALID_ARG;
    }

    steps = ( millivolts - cfg->min_millivolts ) / cfg->step_millivolts;
    return core2foraws_power_axp_twiddle(
        cfg->voltage_reg, cfg->voltage_mask,
        ( uint8_t )( steps << cfg->voltage_lsb ) );
}
