// Based on m5core2_axp by @ropg — Rop Gonggrijp
// https://github.com/ropg/m5core2_axp192

// Which was based on functions modified from / inpspired by @usedbytes - Brian Starkey's
// https://github.com/usedbytes/axp192

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
 * @file core2foraws_power.h
 * @brief Core2 for AWS IoT EduKit power management hardware driver APIs
 */

#ifndef _CORE2FORAWS_POWER_H_
#define _CORE2FORAWS_POWER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>

/**
 * @brief The enumerated list of power rail options.
 */
/* @[declare_core2foraws_power_power_rail_t] */
typedef enum 
{
    POWER_RAIL_DCDC1 = 0,
    POWER_RAIL_DCDC2,
    POWER_RAIL_DCDC3,
    POWER_RAIL_LDO1,
    POWER_RAIL_LDO2,
    POWER_RAIL_LDO3,
    POWER_RAIL_EXTEN,
    POWER_RAIL_COUNT,
} power_rail_t;
/* @[declare_core2foraws_power_power_rail_t] */

/**
 * @brief The power rail supplying power to the ESP32.
 */
/* @[declare_core2foraws_power_rail_esp32] */
#define POWER_RAIL_ESP32                       POWER_RAIL_DCDC1
/* @[declare_core2foraws_power_rail_esp32] */

/**
 * @brief The power rail supplying power to the display controller 
 * and SD card.
 */
/* @[declare_core2foraws_power_rail_logic_and_sd] */
#define POWER_RAIL_LOGIC_AND_SD                    POWER_RAIL_LDO2
/* @[declare_core2foraws_power_rail_logic_and_sd] */

/**
 * @brief The power rail supplying power to the vibration motor.
 */
/* @[declare_core2foraws_power_rail_vibrator] */
#define POWER_RAIL_VIBRATOR                        POWER_RAIL_LDO3
/* @[declare_core2foraws_power_rail_vibrator] */

/**
 * @brief The power rail supplying power to the display backlight.
 */
/* @[declare_core2foraws_power_rail_display_backlight] */
#define POWER_RAIL_DISPLAY_BACKLIGHT               POWER_RAIL_DCDC3
/* @[declare_core2foraws_power_rail_display_backlight] */

#ifndef DISPLAY_BACKLIGHT_START
/**
 * @brief The starting percentage for the display backlight 
 * brightness.
 */
/* @[declare_core2foraws_power_display_backlight_start] */
#define DISPLAY_BACKLIGHT_START         80U
/* @[declare_core2foraws_power_display_backlight_start] */
#endif

/**
 * @brief The maximum voltage to be supplied to the display
 * backlight.
 */
/* @[declare_core2foraws_power_display_backlight_max_volts] */
#define DISPLAY_BACKLIGHT_MAX_VOLTS     3300U
/* @[declare_core2foraws_power_display_backlight_max_volts] */

/**
 * @brief The minimum voltage to be supplied to the display
 * backlight.
 */
/* @[declare_core2foraws_power_display_backlight_min_volts] */
#define DISPLAY_BACKLIGHT_MIN_VOLTS     2200U
/* @[declare_core2foraws_power_display_backlight_min_volts] */

/**
 * @brief Initializes the power management chip driver over I2C.
 * 
 * @note The core2foraws_init() calls this function when the 
 * hardware feature is enabled.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_init] */
esp_err_t core2foraws_power_init( void );
/* @[declare_core2foraws_power_init] */

/**
 * @brief Sets the brightness of the display backlight.
 *
 * This function sets the brightness of the display using the power 
 * management unit (PMU). 
 *
 * **Example:**
 *
 * Set the display brightness to maximum (100%).
 * @code{c}
 *  #include "core2foraws.h"
 * 
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      core2foraws_power_backlight_set( 100 );
 *  }
 * @endcode
 *
 * @param[in] brightness The desired brightness of the display. 
 * Accepts a value from 0 to 100.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_backlight_set] */
esp_err_t core2foraws_power_backlight_set( uint8_t brightness );
/* @[declare_core2foraws_power_backlight_set] */

/**
 * @brief Sets the green LED on the bottom of the device on or off.
 *
 * **Example:**
 *
 * Turn off the green LED that's in between the SD card slot and 
 * reset button.
 * @code{c}
 *  #include "core2foraws.h"
 * 
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      core2foraws_power_led_enable( FALSE );
 *  }
 * @endcode
 *
 * @param[in] state The desired boolean state of the LED. 0 or FALSE 
 * is off, 1 or TRUE is on.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_led_enable] */
esp_err_t core2foraws_power_led_enable( bool state );
/* @[declare_core2foraws_power_led_enable] */

/**
 * @brief Sets the vibration motor on at the preset voltage or off.
 *
 * **Example:**
 *
 * Turn on the vibration motor for 1 second and then turn it off.
 * @code{c}
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include "core2foraws.h"
 * 
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 * 
 *      core2foraws_power_vibration_enable( TRUE );
 *      vTaskDelay( pdMS_TO_TICKS( 1000 ) );
 *      core2foraws_power_vibration_enable( FALSE );
 *  }
 * @endcode
 *
 * @param[in] state The desired boolean state of the vibration motor. 
 * 0 or FALSE is off, 1 or TRUE is on.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_vibration_enable] */
esp_err_t core2foraws_power_vibration_enable( bool state );
/* @[declare_core2foraws_power_vibration_enable] */

/**
 * @brief Supply power to the internal speaker amplifier.
 *
 * @note This function is called automatically when @ref 
 * core2foraws_audio_speaker_enable is used.
 * 
 * **Example:**
 *
 * Turn on power to the speaker amplifier.
 * @code{c}
 *  #include "core2foraws.h"
 * 
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      core2foraws_power_speaker_enable( TRUE );
 *  }
 * @endcode
 *
 * @param[in] state The desired boolean state of the speaker 
 * amplifier. 0 or FALSE is off, 1 or TRUE is on.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_speaker_enable] */
esp_err_t core2foraws_power_speaker_enable( bool state );
/* @[declare_core2foraws_power_speaker_enable] */

/**
 * @brief Get the battery voltage value from the power management 
 * unit (PMU).
 *
 * @param[out] volts Pointer to the buffer that will store the
 * battery voltage
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_batt_volts_get] */
esp_err_t core2foraws_power_batt_volts_get( float *volts );
/* @[declare_core2foraws_power_batt_volts_get] */

/**
 * @brief Get the battery charge current from the power management 
 * unit (PMU).
 *
 * @param[out] m_amps Pointer to the buffer that will store the
 * battery voltage. A value greater than 0 means the battery is 
 * being charged.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_batt_current_get] */
esp_err_t core2foraws_power_batt_current_get( float *m_amps );
/* @[declare_core2foraws_power_batt_current_get] */

/**
 * @brief Get the current charging status of the built-in battery 
 * from the power management unit (PMU).
 * 
 * @note A fully charged battery (~4.2v) will not receive further 
 * charge and this value will return ~0mA. This is not the same as
 * the device being plugged in.
 * 
 * **Example:**
 *
 * Print out to serial output if the device is charging.
 * @code{c}
 *  #include <esp_log.h>
 *  #include "core2foraws.h"
 * 
 *  static const char *TAG = "MAIN_POWER_DEMO";
 * 
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      
 *      bool charge_status;
 *      core2foraws_power_charging_get( &charge_status );
 *      
 *      if( charge_status)
 *      {
 *          ESP_LOGI( TAG, "Battery is being charged" );
 *      }
 *  }
 * @endcode
 *
 * @param[out] status Pointer to the boolean value of the 
 * charging status. 1 (true) if it is currently charging, 0 (false)
 * if it is not charging or fully charged.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_charging_get] */
esp_err_t core2foraws_power_charging_get( bool *status );
/* @[declare_core2foraws_power_charging_get] */

/**
 * @brief Get status of the device being plugged in to external
 * power source from the power management unit (PMU).
 * 
 * The power source can be either the USB-C port or through the
 * V-In pin.
 * 
 *
 * @param[out] status Pointer to the boolean value of the plugged 
 * in to an external power source status. 1 (true) if it is 
 * currently plugged in, 0 (false) if it is using battery power.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_plugged_get] */
esp_err_t core2foraws_power_plugged_get( bool *status );
/* @[declare_core2foraws_power_plugged_get] */

/**
 * @brief Get the single register value from the power management 
 * unit (PMU).
 *
 * This function is used to directly query the AXP192 PMU register
 * value. 
 * 
 * @warning Manipulating the PMU values is only recommended for 
 * _advanced_ users as it can lead to a bricked device.
 *
 * @param[in] reg The register address to read from.
 * @param[out] buffer Pointer to the buffer data that was read from 
 * the PMU.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_axp_reg_get] */
esp_err_t core2foraws_power_axp_reg_get( uint8_t reg, uint8_t *buffer );
/* @[declare_core2foraws_power_axp_reg_get] */

/**
 * @brief Set a value to the specified power management unit (PMU)
 * register.
 *
 * This function is used to directly write the AXP192 PMU register
 * value. 
 * 
 * @warning Manipulating the PMU values is only recommended for 
 * _advanced_ users as it can lead to a bricked device.
 *
 * @param[in] reg The register address to read from.
 * @param[in] value The value to set the register to.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_axp_reg_set] */
esp_err_t core2foraws_power_axp_reg_set( uint8_t reg, uint8_t value );
/* @[declare_core2foraws_power_axp_reg_set] */

/**
 * @brief Read the values starting from the specified power 
 * management unit (PMU) register.
 *
 * This function is used to directly read the AXP192 PMU register
 * values. 
 * 
 * @warning Manipulating the PMU values is only recommended for 
 * _advanced_ users as it can lead to a bricked device.
 *
 * @param[in] reg The register address to read from.
 * @param[out] buffer Pointer to the buffer data read from the PMU.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_axp_read] */
esp_err_t core2foraws_power_axp_read( uint8_t reg, void *buffer );
/* @[declare_core2foraws_power_axp_read] */

/**
 * @brief Write values to the specified power management unit (PMU) 
 * register.
 *
 * This function is used to directly write to the AXP192 PMU 
 * registers. 
 * 
 * @warning Manipulating the PMU values is only recommended for 
 * _advanced_ users as it can lead to a bricked device.
 *
 * @param[in] reg The register address to write to.
 * @param[out] buffer Pointer to the buffer data read from the PMU.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_axp_write] */
esp_err_t core2foraws_power_axp_write( uint8_t reg, const uint8_t *buffer );
/* @[declare_core2foraws_power_axp_write] */

/**
 * @brief Update one or more Power Management Unit (PMU) registers 
 * at once.
 *
 * This function is used update one or more AXP192 PMU register 
 * values in a single call.
 * 
 * @warning Manipulating the PMU values is only recommended for 
 * _advanced_ users as it can lead to a bricked device.
 *
 * @param[in] reg The starting register address to write to.
 * @param[in] affect The desired registers to change.
 * @param[in] value The desired value to change register(s) to.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_axp_twiddle] */
esp_err_t core2foraws_power_axp_twiddle( uint8_t reg, uint8_t affect, uint8_t value );
/* @[declare_core2foraws_power_axp_twiddle] */

/**
 * @brief Get the specified power rail state from the power 
 * management unit (PMU).
 * 
 * @warning Manipulating the PMU values is only recommended for 
 * _advanced_ users as it can lead to a bricked device.
 *
 * @param[in] rail The power rail to retrieve the state of.
 * @param[out] enabled Pointer to the current state of the rail.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_rail_state_get] */
esp_err_t core2foraws_power_rail_state_get( power_rail_t rail, bool *enabled );
/* @[declare_core2foraws_power_rail_state_get] */

/**
 * @brief Set the specified power rail state from the power 
 * management unit (PMU).
 * 
 * This function is used to directly write the AXP192 PMU register
 * value. 
 * 
 * @warning Manipulating the PMU values is only recommended for 
 * _advanced_ users as it can lead to a bricked device.
 *
 * @param[in] rail The power rail to set the state of.
 * @param[out] enabled The state to set the rail to.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_rail_state_set] */
esp_err_t core2foraws_power_rail_state_set( power_rail_t rail, bool enabled );
/* @[declare_core2foraws_power_rail_state_set] */

/**
 * @brief Get the specified power rail voltage (in millivolts) from 
 * the power management unit (PMU).
 * 
 * This function is used to directly query the AXP192 PMU register
 * value. 
 * 
 * @warning Manipulating the PMU values is only recommended for 
 * _advanced_ users as it can lead to a bricked device.
 *
 * @param[in] rail The power rail to set the voltage of.
 * @param[out] millivolts Pointer to the value the rail is set to in 
 * millivolts.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_rail_mv_get] */
esp_err_t core2foraws_power_rail_mv_get( power_rail_t rail, uint16_t *millivolts );
/* @[declare_core2foraws_power_rail_mv_get] */

/**
 * @brief Set the specified power rail voltage (in mv) from the 
 * power management unit (PMU).
 * 
 * This function is used to directly write the AXP192 PMU register
 * value. 
 * 
 * @warning Manipulating the PMU values is only recommended for 
 * _advanced_ users as it can lead to a bricked device.
 *
 * @param[in] rail The power rail to set the voltage of.
 * @param[out] millivolts The millivolts to set the rail to.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_power_rail_mv_set] */
esp_err_t core2foraws_power_rail_mv_set( power_rail_t rail, uint16_t millivolts );
/* @[declare_core2foraws_power_rail_mv_set] */

#ifdef __cplusplus
}
#endif
#endif