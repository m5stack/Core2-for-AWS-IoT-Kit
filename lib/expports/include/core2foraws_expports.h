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
 * @file core2foraws_expports.h
 * @brief Core2 for AWS IoT EduKit expansion ports hardware driver APIs
 */

#ifndef _core2foraws_expports_H_
#define _core2foraws_expports_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_err.h>

#include "i2c_manager.h"

/**
 * @brief The I2C SDA pin on expansion port A.
 *
 * This maps to GPIO 32 and can be used as the Serial Data Line 
 * (SDA) pin for I2C bus communication. Read more about [I2C 
 * capabilities of the ESP32 microcontroller and available APIs 
 * in the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/peripherals/i2c.html).
 */
/* @[declare_core2foraws_expports_port_a_sda_pin] */
#define PORT_A_SDA_PIN GPIO_NUM_32
/* @[declare_core2foraws_expports_port_a_sda_pin] */

/**
 * @brief The I2C SCL pin on expansion port A.
 *
 * This maps to GPIO 33 and can be used as the Serial CLock Line 
 * (SCL) pin to synchronize I2C bus communication. Read more about 
 * [I2C capabilities of the ESP32 microcontroller and available 
 * APIs in the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/peripherals/i2c.html).
 */
/* @[declare_core2foraws_expports_port_a_scl_pin] */
#define PORT_A_SCL_PIN GPIO_NUM_33
/* @[declare_core2foraws_expports_port_a_scl_pin] */

/**
 * @brief Placeholder for I2C comms without a register address
 */
/* @[declare_core2foraws_expports_i2c_no_register_addr] */
#define I2C_NO_REGISTER_ADDR I2C_NO_REG
/* @[declare_core2foraws_expports_i2c_no_register_addr] */

/**
 * @brief The ADC pin on expansion port B.
 *
 * This maps to GPIO 36 and is capable of converting analog signals 
 * to digital. Read more about using [ADCs with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html).
 */
/* @[declare_core2foraws_expports_port_b_adc_pin] */
#define PORT_B_ADC_PIN GPIO_NUM_36
/* @[declare_core2foraws_expports_port_b_adc_pin] */

/**
 * @brief The DAC pin on expansion port B.
 *
 * This maps to GPIO 26 and is capable of converting digital 
 * signals to analog. Read more about using [DACs with the 
 * ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/dac.html).
 */
/* @[declare_core2foraws_expports_port_b_dac_pin] */
#define PORT_B_DAC_PIN GPIO_NUM_26
/* @[declare_core2foraws_expports_port_b_dac_pin] */

/**
 * @brief The UART transmission pin on expansion port C.
 *
 * This maps to GPIO 14 and can be used as the transmission 
 * channel for UART communication. Read more about [UART 
 * communications with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html).
 */
/* @[declare_core2foraws_expports_port_c_uart_tx_pin] */
#define PORT_C_UART_TX_PIN GPIO_NUM_14
/* @[declare_core2foraws_expports_port_c_uart_tx_pin] */

/**
 * @brief The UART receiver pin on expansion port C.
 *
 * This maps to GPIO 13 and can be used as the receiver channel 
 * for UART communication. Read more about [UART communications 
 * with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html).
 */
/* @[declare_core2foraws_expports_port_c_uart_rx_pin] */
#define PORT_C_UART_RX_PIN GPIO_NUM_13
/* @[declare_core2foraws_expports_port_c_uart_rx_pin] */

/**
 * @brief The default UART controller used for expansion port C.
 *
 * This maps to UART controller UART2.
 * Read more about [UART communications with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html).
 */
/* @[declare_core2foraws_expports_port_c_uart_num] */
#define PORT_C_UART_NUM UART_NUM_2
/* @[declare_core2foraws_expports_port_c_uart_num] */

#ifndef UART_RX_BUF_SIZE
/**
 * @brief The default UART RX ring buffer size.
 *
 * This is the default size of the UART interface's receiver ring
 * buffer.
 * Read more about [UART communications with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html).
 */
/* @[declare_core2foraws_expports_uart_rx_buf_size] */
#define UART_RX_BUF_SIZE 2048
/* @[declare_core2foraws_expports_uart_rx_buf_size] */
#endif

/**
 * @brief Reads the value from a specified digital pin — either 1 (high) or 0 (low).
 *
 * Pin reads high when connected to Vcc. In order for the pin to 
 * read high, the connected peripheral must use the available 
 * ground and Vcc pins and not an external power source.
 * 
 * @note To use the pin for another compatible purpose (e.g. 
 * depending on the pin — read, ADC, DAC, I2C, UART, etc.), you must 
 * first reset the pin with @ref core2foraws_expports_pin_reset.
 * 
 * @note The AXP192 PMU is configured to output 5v on the 
 * expansion port's VCC pin. Some peripherals require 3.3V and 
 * must have the voltage reduced to avoid damage.
 *
 * **Example:**
 * 
 * The example code creates a FreeRTOS task which periodically 
 * (1s) reads from GPIO 26 to see if a compatible peripheral is 
 * connected. The task prints the returned value to the serial 
 * monitor.
 * @code{c}
 *  #include <stdio.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_log.h>
 *
 *  #include "core2foraws.h"
 *
 *  static const char *TAG = "MAIN_DIGITAL_READ_DEMO";
 *
 *  void pin_read_task()
 *  {
 *      bool state = false;
 * 
 *      for( ;; )
 *      {
 *          core2foraws_expports_digital_read( GPIO_NUM_26, &state );
 *          ESP_LOGI( TAG, "\tSensor plugged in ( 0=no / 1=yes ): %d", state);
 *          vTaskDelay( pdMS_TO_TICKS( 1000 ) );
 *      }
 *  }
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 * 
 *      if ( err == ESP_OK )
 *      {
 *          xTaskCreatePinnedToCore( pin_read_task, "read_pin", 1024*3, NULL, 1, NULL, 1 );
 *      }
 *  }
 * @endcode
 *
 * @param[in] pin The GPIO pin to read.
 * @param[out] level Pointer to set as the boolean digital level of 
 * the pin reading.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL	: Failed to read
 */
/* @[declare_core2foraws_expport_digital_read] */
esp_err_t core2foraws_expports_digital_read( gpio_num_t pin, bool *level );
/* @[declare_core2foraws_expport_digital_read] */

/**
 * @brief Writes the value to a specified digital pin — either 1 
 * (high) or 0 (low).
 *
 * A high output is connected to Vcc. In order to complete the 
 * circuit, the connected peripheral must use the available 
 * ground.
 * 
 * @note To use the pin for another compatible purpose (e.g. 
 * depending on the pin — read, ADC, DAC, I2C, UART, etc.), you 
 * must first reset the pin with @ref core2foraws_expports_pin_reset.
 * 
 * @note The AXP192 PMU is configured to output 5v on the 
 * expansion port's VCC pin. Some peripherals require 3.3V and 
 * must have the voltage reduced to avoid damage.
 *
 * **Example:**
 * 
 * The example code creates a FreeRTOS task which periodically 
 * (1s) reads from GPIO 26 to see if a compatible peripheral is 
 * connected. The task prints the returned value to the serial 
 * monitor.
 * @code{c}
 *  #include <stdio.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_log.h>
 *
 *  static const char *TAG = "MAIN_DIGITAL_WRITE_DEMO";
 *
 *  void pin_write_task()
 *  {
 *      bool output = 0;
 *      for( ;; )
 *      {
 *          esp_err_t err = ESP_FAIL;
 *          
 *          err = core2foraws_expports_digital_write( GPIO_NUM_26, output );
 *          if ( err == ESP_OK )
 *              ESP_LOGI( TAG, "\tOutput on GPIO %d: %s", GPIO_NUM_26, output ? "HIGH" : "LOW" );
 *
 *          output = !output;
 *
 *          vTaskDelay( pdMS_TO_TICKS( 1000 ) );
 *      }
 *  }
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      xTaskCreatePinnedToCore( pin_write_task, "write_pin", 1024*3, NULL, 1, NULL, 1 );
 *  }
 * @endcode
 *
 * @param[in] pin The GPIO pin to write.
 * @param[out] level The digital level of the pin reading.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL	: Failed to write
 */
/* @[declare_core2foraws_expport_digital_write] */
esp_err_t core2foraws_expports_digital_write( gpio_num_t pin, const bool level );
/* @[declare_core2foraws_expport_digital_write] */

/**
 * @brief Resets the specified pin.
 *
 * This function is used to reset the pin configuration to either 
 * save memory or to use the pin for another compatible purpose 
 * (e.g. depending on the pin — read, ADC, DAC, I2C, UART, etc.), 
 * you must first reset the pin with @ref core2foraws_expports_pin_reset.
 * 
 * **Example:**
 * 
 * The example code reads from GPIO 26 and then resets the pin.
 * @code{c}
 *  #include <stdio.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <esp_log.h>
 *
 *  static const char *TAG = "MAIN_PIN_RESET_DEMO";
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      
 *      bool state = false;
 *      core2foraws_expports_digital_read( GPIO_NUM_26, &state );
 *          ESP_LOGI( TAG, "\tSensor plugged in ( 0=no / 1=yes ): %d", state);
 *      core2foraws_expports_pin_reset( GPIO_NUM_26 );
 *  }
 * @endcode
 *
 * @param[in] pin The GPIO pin to reset. 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL	: Failed to reset
 */
/* @[declare_core2foraws_expport_pin_reset] */
esp_err_t core2foraws_expports_pin_reset( gpio_num_t pin );
/* @[declare_core2foraws_expport_pin_reset] */

/**
 * @brief Configures expansion port A for use with an I2C peripheral.
 *  
 * Configures the pin to use I2C.
 * 
 * @note The baud rate of the I2C device can be set in the Kconfig 
 * menu for the symbol `CONFIG_I2C_MANAGER_1_FREQ_HZ`. The default 
 * baud rate is 400KHz.
 *
 * @note The AXP192 PMU is configured to output 5v on the 
 * expansion port's VCC pin. Some peripherals require 3.3V and 
 * must have the voltage reduced to avoid damage.
 *
 * **Example:**
 * 
 * The example creates a tasks which initializes the port for I2C, 
 * and loops every 10 seconds to read from a heart-rate sensor with 
 * the 8-bit device address 0xA0 (does not need to be right-shifted 
 * by 1-bit to be a 7-bit address). Since the device doesn't take a 
 * register address, use the placeholder macro @ref I2C_NO_REGISTER_ADDR. 
 * At the end of the loop, the sensor reading is printed to the 
 * serial output.
 * @code{c}
 *  #include <stdio.h>
 *  #include <stdlib.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 *  
 *  #define DEVICE_ADDRESS 0xA0
 *  
 *  static const char *TAG = "MAIN_I2C_EXAMPLE";
 *  
 *  void i2c_test_task( void *pvParameters )
 *  {
 *      esp_err_t err = ESP_FAIL;
 *      
 *      err = core2foraws_expports_i2c_begin();
 *      
 *      if ( err == ESP_OK)
 *      {
 *          uint8_t heart_rate = 255;
 * 
 *          for( ;; )
 *          {
 *              err = core2foraws_expports_i2c_read( DEVICE_ADDRESS, I2C_NO_REG, &heart_rate, 1 );
 *              if ( !err )
 *              {
 *                  ESP_LOGI(TAG, "Heart Rate — %ubpm", heart_rate);    
 *              }
 * 
 *              vTaskDelay( pdMS_TO_TICKS( 10000 ) );
 *          }
 *      }
 *      
 *      core2foraws_expports_i2c_close();
 *      vTaskDelete( NULL );
 *  }
 *  
 *  void app_main()
 *  {
 *      core2foraws_init();
 *      xTaskCreatePinnedToCore(&i2c_test_task, "i2c_test_task", 1024*4, NULL, 2, NULL, 1);
 *  }
 * @endcode
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_expport_i2c_begin] */
esp_err_t core2foraws_expports_i2c_begin( void );
/* @[declare_core2foraws_expport_i2c_begin] */

/**
 * @brief Read from the I2C peripheral attatched to expansion port A.
 *
 * A thread-safe method to read from the I2C peripheral.
 * 
 * Read more about [I2C capabilities of the ESP32 microcontroller 
 * and available APIs in the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/peripherals/i2c.html).
 * 
 * @note The baud rate of the I2C device can be set in the Kconfig 
 * menu for the symbol `CONFIG_I2C_MANAGER_1_FREQ_HZ`. The default 
 * baud rate is 400KHz.
 *
 * @note The AXP192 PMU is configured to output 5v on the 
 * expansion port's VCC pin. Some peripherals require 3.3V and 
 * must have the voltage reduced to avoid damage.
 *
 * **Example:**
 * 
 * The example creates a tasks which initializes the port for I2C, 
 * and loops every 10 seconds to read from a heart-rate sensor with 
 * the 8-bit device address 0xA0 (does not need to be right-shifted 
 * by 1-bit to be a 7-bit address). Since the device doesn't take a 
 * register address, use the placeholder macro @ref I2C_NO_REGISTER_ADDR. 
 * At the end of the loop, the sensor reading is printed to the 
 * serial output.
 * @code{c}
 *  #include <stdio.h>
 *  #include <stdlib.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 *  
 *  #define DEVICE_ADDRESS 0xA0
 *  
 *  static const char *TAG = "MAIN_I2C_EXAMPLE";
 *  
 *  void i2c_test_task( void *pvParameters )
 *  {
 *      esp_err_t err = ESP_FAIL;
 *      
 *      err = core2foraws_expports_i2c_begin();
 *      
 *      if ( err == ESP_OK)
 *      {
 *          uint8_t heart_rate = 255;
 * 
 *          for( ;; )
 *          {
 *              err = core2foraws_expports_i2c_read( DEVICE_ADDRESS, I2C_NO_REG, &heart_rate, 1 );
 *              if ( !err )
 *              {
 *                  ESP_LOGI(TAG, "Heart Rate — %ubpm", heart_rate);    
 *              }
 * 
 *              vTaskDelay( pdMS_TO_TICKS( 10000 ) );
 *          }
 *      }
 *      
 *      core2foraws_expports_i2c_close();
 *      vTaskDelete( NULL );
 *  }
 *  
 *  void app_main()
 *  {
 *      core2foraws_init();
 *      xTaskCreatePinnedToCore(&i2c_test_task, "i2c_test_task", 1024*4, NULL, 2, NULL, 1);
 *  }
 * @endcode
 * 
 * @param[in] device_address The 8-bit I2C peripheral address.
 * @param[in] register_address The data register address.
 * @param[out] data Pointer to the data read from the I2C peripheral.
 * @param[in] length The number of bytes to read.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_expport_i2c_read] */
esp_err_t core2foraws_expports_i2c_read( uint16_t device_address, uint32_t register_address, uint8_t *data, uint16_t length );
/* @[declare_core2foraws_expport_i2c_read] */

/**
 * @brief Write to the I2C peripheral attatched to expansion port A.
 *
 * A thread-safe method to write to the I2C peripheral.
 * 
 * Read more about [I2C capabilities of the ESP32 microcontroller 
 * and available APIs in the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/peripherals/i2c.html).
 * 
 * @note The baud rate of the I2C device can be set in the Kconfig 
 * menu for the symbol `CONFIG_I2C_MANAGER_1_FREQ_HZ`. The default 
 * baud rate is 400KHz.
 *
 * @note The AXP192 PMU is configured to output 5v on the 
 * expansion port's VCC pin. Some peripherals require 3.3V and 
 * must have the voltage reduced to avoid damage.
 *
 * **Example:**
 * 
 * The example creates a tasks which initializes the port for I2C, 
 * and writes a single byte — 0x01 — to register 0x12 to a device 
 * with the address 0x29.
 * @code{c}
 *  #include <stdio.h>
 *  #include <stdlib.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 *  
 *  #define DEVICE_ADDRESS 0x29
 *  
 *  static const char *TAG = "MAIN_I2C_EXAMPLE";
 *  
 *  void app_main()
 *  {
 *      core2foraws_init();
 *      esp_err_t err = ESP_FAIL;
 *      uint8_t register_address = 0x12;
 *      const uint8_t write_byte = 0x01;
 *      
 *      err = core2foraws_expports_i2c_begin();
 *      if ( err == ESP_OK )
 *      {
 *          core2foraws_expports_i2c_write( DEVICE_ADDRESS, register_address, &write_byte, sizeof( write_byte ) )
 *      }
 *  }
 * @endcode
 * 
 * @param[in] device_address The 8-bit I2C peripheral address.
 * @param[in] register_address The data register address.
 * @param[out] data Pointer to the data to write to the I2C 
 * peripheral.
 * @param[in] length The number of bytes to write.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_expport_i2c_write] */
esp_err_t core2foraws_expports_i2c_write( uint16_t device_address, uint32_t register_address, const uint8_t *data, uint16_t length );
/* @[declare_core2foraws_expport_i2c_write] */

/**
 * @brief Removes the thread-safe I2C port mutex.
 *
 * An optional function that can be called to remove the mutex
 * associated with the external port.
 * 
 * Read more about [I2C capabilities of the ESP32 microcontroller 
 * and available APIs in the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/peripherals/i2c.html).
 *
 * @note The AXP192 PMU is configured to output 5v on the 
 * expansion port's VCC pin. Some peripherals require 3.3V and 
 * must have the voltage reduced to avoid damage.
 *
 * **Example:**
 * 
 * The example creates a tasks which initializes the port for I2C, 
 * and loops every 10 seconds to read from a heart-rate sensor with 
 * the 8-bit device address 0xA0 (does not need to be right-shifted 
 * by 1-bit to be a 7-bit address). Since the device doesn't take a 
 * register address, use the placeholder macro @ref I2C_NO_REGISTER_ADDR. 
 * At the end of the loop, the sensor reading is printed to the 
 * serial output. If @ref core2foraws_expports_i2c_begin returns an
 * error, call `core2foraws_expports_i2c_close` and reset the pin.
 * @code{c}
 *  #include <stdio.h>
 *  #include <stdlib.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_log.h>
 * 
 *  #include "core2foraws.h"
 *  
 *  #define DEVICE_ADDRESS 0xA0
 *  
 *  static const char *TAG = "MAIN_I2C_EXAMPLE";
 *  
 *  void i2c_test_task( void *pvParameters )
 *  {
 *      esp_err_t err = ESP_FAIL;
 *      
 *      err = core2foraws_expports_i2c_begin();
 *      
 *      if ( err == ESP_OK)
 *      {
 *          uint8_t heart_rate = 255;
 * 
 *          for( ;; )
 *          {
 *              err = core2foraws_expports_i2c_read( DEVICE_ADDRESS, I2C_NO_REG, &heart_rate, 1 );
 *              if ( !err )
 *              {
 *                  ESP_LOGI(TAG, "Heart Rate — %ubpm", heart_rate);    
 *              }
 * 
 *              vTaskDelay( pdMS_TO_TICKS( 10000 ) );
 *          }
 *      }
 *      
 *      core2foraws_expports_i2c_close();
 *      vTaskDelete( NULL );
 *  }
 *  
 *  void app_main()
 *  {
 *      core2foraws_init();
 *      xTaskCreatePinnedToCore(&i2c_test_task, "i2c_test_task", 1024*4, NULL, 2, NULL, 1);
 *  }
 * @endcode
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK                : Success
 *  - ESP_ERR_INVALID_ARG	: Driver parameter error
 */
/* @[declare_core2foraws_expport_i2c_close] */
esp_err_t core2foraws_expports_i2c_close( void );
/* @[declare_core2foraws_expport_i2c_close] */

/**
 * @brief Read the raw ADC value from @ref PORT_B_ADC_PIN (GPIO 36).
 *
 * This function reads the raw ADC value from Port B's
 * Analog-to-Digital-Converter (ADC) on GPIO36. GPIO36 is the only 
 * pin on the external ports with access to the ADC.
 * Read more about using [ADCs with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html).
 * 
 * @note Uses the etched eFuse VRef calibration.
 *
 * **Example:**
 * 
 * The example creates a FreeRTOS task which periodically (1s) reads 
 * the raw ADC value of the [M5Stack EARTH](https://shop.m5stack.com/products/earth-sensor-unit) 
 * moisture sensor connected on Port B. It prints out the value to 
 * the serial output.
 * @code{c}
 *  #include <stdio.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_log.h>
 *
 *  #include "core2foraws.h"
 *
 *  static const char *TAG = "RAW_ADC_READ_DEMO";
 *
 *  void read_moisture_task( void *pvParameters )
 *  {
 *      int adc_val = 0;
 *      for( ;; )
 *      {
 *          core2foraws_expports_adc_read( &adc_val );
 *          ESP_LOGI( TAG, "\tMoisture ADC raw: %d", adc_val );
 * 
 *          vTaskDelay( pdMS_TO_TICKS( 1000 ) );
 *      }
 *  }
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      xTaskCreatePinnedToCore(read_moisture_task, "moisture_raw", 1024*3, NULL, 1, NULL, 1);
 *  }
 * @endcode
 *
 * @param[out] raw_adc_value A pointer to the raw ADC reading.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL	: Failed to write
 */
/* @[declare_core2foraws_expport_adc_read] */
esp_err_t core2foraws_expports_adc_read( int *raw_adc_value );
/* @[declare_core2foraws_expport_adc_read] */

/**
 * @brief Read the calibrated ADC voltage from @ref PORT_B_ADC_PIN 
 * (GPIO 36).
 *
 * This function reads the raw ADC value from Port B's
 * Analog-to-Digital-Converter (ADC) on GPIO36 and converts it to
 * millivolts using the VRef calibration. GPIO36 is the only pin on
 * the external ports with access to the ADC.
 * Read more about using [ADCs with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html).
 * 
 * @note Uses the etched eFuse VRef calibration.
 *
 * **Example:**
 * 
 * The example creates a FreeRTOS task which periodically (1s) reads 
 * the ADC voltage of the [M5Stack EARTH](https://shop.m5stack.com/products/earth-sensor-unit) 
 * moisture sensor connected on Port B. It prints out the value to 
 * the serial output.
 * @code{c}
 *  #include <stdio.h>
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_log.h>
 *
 *  #include "core2foraws.h"
 *
 *  static const char *TAG = "ADC_VOLTAGE_READ_DEMO";
 *
 *  void read_moisture_task( void *pvParameters )
 *  {
 *      uint32 adc_volts = 0;
 *      for( ;; )
 *      {
 *          core2foraws_expports_adc_mv_read( &adc_volts );
 *          ESP_LOGI( TAG, "\tMoisture ADC millivolts: %d", adc_volts );
 * 
 *          vTaskDelay( pdMS_TO_TICKS( 1000 ) );
 *      }
 *  }
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      xTaskCreatePinnedToCore(read_moisture_task, "moisture_raw", 1024*3, NULL, 1, NULL, 1);
 *  }
 * @endcode
 *
 * @param[out] adc_mvolts Pointer to the voltage reading from the 
 * ADC in millivolts.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL	: Failed to write
 */
/* @[declare_core2foraws_expport_adc_mv_read] */
esp_err_t core2foraws_expports_adc_mv_read( uint32_t *adc_mvolts );
/* @[declare_core2foraws_expport_adc_mv_read] */

/**
 * @brief Outputs the specified voltage (millivolts) to the DAC on 
 * @ref PORT_B_DAC_PIN (GPIO 26).
 *
 * This function accepts a range from 0 to a maximum of VDD3P3_RTC
 * (~3300mV).
 *
 * **Example:**
 * 
 * The example code sets the PORT_B_DAC_PIN (GPIO 26) pin mode
 * to DAC and outputs 1,500mV.
 * @code{c}
 *  #include <stdio.h>
 *  #include <esp_log.h>
 *
 *  #include "core2foraws.h"
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      core2foraws_expports_dac_mv_write( 1500 );
 *  }
 * @endcode
 *
 * @param[in] dac_mvolts The voltage level to set the DAC pin in millivolts.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL	: Failed to write
 */
/* @[declare_core2foraws_expport_adc_mv_write] */
esp_err_t core2foraws_expports_dac_mv_write( const uint16_t dac_mvolts );
/* @[declare_core2foraws_expport_adc_mv_write] */

/**
 * @brief Configures the UART interface on for @ref 
 * PORT_C_UART_RX_PIN (GPIO 13) and @ref PORT_C_UART_TX_PIN (GPIO 14)
 * at the specified baud rate.
 *
 * This is a helper function to simplify building applications
 * with UART. It preconfigures the UART communications with 8 bit 
 * word length, no parity bit, 1 bit for the number of stop bits, 
 * and hardware flow control disabled. It sets the baud rate to the 
 * user defined value and starts reading from the UART channel 2 and 
 * storing data into the ring buffer of length @ref UART_RX_BUF_SIZE.
 *
 * @note Usage of the UART convenience methods provided in this BSP aims 
 * to simplify development at the expense of compatibility and 
 * performance. The configuration above may not be suitable for your 
 * application or attached peripheral. For more information about 
 * UART communications on the Core2 for AWS IoT EduKit using the 
 * ESP32 and how to create your own configuration, visit Espressif's 
 * official [documentation](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/peripherals/uart.html).
 *
 * @note The ESP32 is a 3.3v device and requires 3.3v on the UART 
 * TX/RX lines. Higher voltages requires the use of a level shifter.
 *
 * **Example:**
 * 
 * The example below sets @ref PORT_C_UART_TX_PIN (GPIO 14 to 
 * transmit) and PORT_C_UART_RX_PIN (GPIO 13 to receive), and sets 
 * the UART baud rate to 115200. It then starts two FreeRTOS tasks — 
 * one that transmits "Hello from AWS IoT EduKit" every two seconds 
 * and the other receives the message. The receiver task outputs the 
 * number of bytes read from the buffer, the number of bytes that 
 * remains unread in the ring buffer, and the message.
 *
 * @note To receive the messages transmitted on the same device, run a
 * female-female jumper wire from Port C's TX pin to PORT C's RX pin.
 * @code{c}
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include "driver/uart.h"
 *  #include "driver/gpio.h"
 *  #include <esp_log.h>
 *
 *  #include "core2foraws.h"
 *
 *  static const char *TAG = "MAIN_UART_DEMO";
 *
 *  static void uart_tx_task( void *pvParameters )
 *  {
 *      while ( 1 )
 *      {
 *          const char *message = "Hello from AWS IoT EduKit";
 *          size_t message_len = strlen( message ) + 1;
 *          size_t written_len = 0;
 * 
 *          core2foraws_expports_uart_write( message, message_len, &written_len );
 *          ESP_LOGI( TAG, "\tWrote %d out of % bytes", written_len, message_len );
 * 
 *          vTaskDelay( pdMS_TO_TICKS( 2000 ) );
 *      }
 *  }
 *
 *  static void uart_rx_task( void *arg )
 *  {
 *      size_t rxBytes;
 *      uint8_t *data = heap_caps_malloc( UART_RX_BUF_SIZE, MALLOC_CAP_SPIRAM ); // Allocate space for message in external RAM
 *      while ( 1 )
 *      {
 *          esp_err_t err = core2foraws_expports_uart_read( data, &rxBytes );
 *          if ( err == ESP_OK )
 *          {
 *              ESP_LOGI( TAG, "\tRead %d bytes from UART. Received: '%s'", rxBytes, data );
 *          }
 *          vTaskDelay( pdMS_TO_TICKS( 500 ) ); // Read more frequently than transmit to ensure the messages are not erased from buffer.
 *      }
 *      free(data); // Free memory from external RAM
 *  }
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      esp_err_t err = ESP_FAIL;
 *      err = core2foraws_expports_uart_begin( 115200 );
 *      if ( err == ESP_OK )
 *      {
 *          xTaskCreatePinnedToCore( uart_rx_task, "uart_rx", 1024*2, NULL, configMAX_PRIORITIES-2, NULL, 1);
 *          xTaskCreatePinnedToCore( uart_tx_task, "uart_tx", 1024*2, NULL, configMAX_PRIORITIES-3, NULL, 1);
 *      }
 *  }
 * @endcode
 *
 * @param[in] baud The baud rate (bits per second) of the UART2 channel.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL	: Failed to write
 */
/* @[declare_core2foraws_expport_uart_begin] */
esp_err_t core2foraws_expports_uart_begin( uint32_t baud );
/* @[declare_core2foraws_expport_uart_begin] */

/**
 * @brief Writes message to the UART2 interface through @ref 
 * PORT_C_UART_TX_PIN (GPIO 14).
 *
 * This is a helper function to simplify building applications
 * with UART using 8 bit word length, no parity bit, 1 bit for the 
 * stop bit, and hardware flow control disabled. It sets the baud 
 * rate to the user defined value and starts reading from the UART 
 * channel 2 and storing data into the ring buffer of length @ref 
 * UART_RX_BUF_SIZE.
 *
 * @note Usage of the UART convenience methods provided in this BSP aims 
 * to simplify development at the expense of compatibility and 
 * performance. The configuration above may not be suitable for your 
 * application or attached peripheral. For more information about 
 * UART communications on the Core2 for AWS IoT EduKit using the 
 * ESP32 and how to create your own configuration, visit Espressif's 
 * official [documentation](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/peripherals/uart.html).
 *
 * @note The ESP32 is a 3.3v device and requires 3.3v on the UART 
 * TX/RX lines. Higher voltages requires the use of a level shifter.
 *
 * **Example:**
 * 
 * The example below sets @ref PORT_C_UART_TX_PIN (GPIO 14 to 
 * transmit) and PORT_C_UART_RX_PIN (GPIO 13 to receive), and sets 
 * the UART baud rate to 115200. It then starts two FreeRTOS tasks — 
 * one that transmits "Hello from AWS IoT EduKit" every two seconds 
 * and the other receives the message. The receiver task outputs the 
 * number of bytes read from the buffer, the number of bytes that 
 * remains unread in the ring buffer, and the message.
 *
 * @note To receive the messages transmitted on the same device, run a
 * female-female jumper wire from Port C's TX pin to PORT C's RX pin.
 * @code{c}
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_log.h>
 *
 *  #include "core2foraws.h"
 *
 *  static const char *TAG = "MAIN_UART_DEMO";
 *
 *  static void uart_tx_task( void *pvParameters )
 *  {
 *      while ( 1 )
 *      {
 *          const char *message = "Hello from AWS IoT EduKit";
 *          size_t message_len = strlen( message ) + 1;
 *          size_t written_len = 0;
 * 
 *          core2foraws_expports_uart_write( message, message_len, &written_len );
 *          ESP_LOGI( TAG, "\tWrote %d out of % bytes", written_len, message_len );
 * 
 *          vTaskDelay( pdMS_TO_TICKS( 2000 ) );
 *      }
 *  }
 *
 *  static void uart_rx_task( void *arg )
 *  {
 *      size_t rxBytes;
 *      uint8_t *data = heap_caps_malloc( UART_RX_BUF_SIZE, MALLOC_CAP_SPIRAM ); // Allocate space for message in external RAM
 *      while ( 1 )
 *      {
 *          esp_err_t err = core2foraws_expports_uart_read( data, &rxBytes );
 *          if ( err == ESP_OK )
 *          {
 *              ESP_LOGI( TAG, "\tRead %d bytes from UART. Received: '%s'", rxBytes, data );
 *          }
 *          vTaskDelay( pdMS_TO_TICKS( 500 ) ); // Read more frequently than transmit to ensure the messages are not erased from buffer.
 *      }
 *      free(data); // Free memory from external RAM
 *  }
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      esp_err_t err = ESP_FAIL;
 *      err = core2foraws_expports_uart_begin( 115200 );
 *      if ( err == ESP_OK )
 *      {
 *          xTaskCreatePinnedToCore( uart_rx_task, "uart_rx", 1024*2, NULL, configMAX_PRIORITIES-2, NULL, 1);
 *          xTaskCreatePinnedToCore( uart_tx_task, "uart_tx", 1024*2, NULL, configMAX_PRIORITIES-3, NULL, 1);
 *      }
 *  }
 * @endcode
 *
 * @param[in] message A pointer to the message to be transmitted 
 * over UART2.
 * @param[in] length The size of the message that you want to 
 * transmit.
 * @param[out] was_written_length Pointer to the number of bytes 
 * sent over @ref PORT_C_UART_TX_PIN (GPIO 14).
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL	: Failed to write
 */
/* @[declare_core2foraws_expport_uart_write] */
esp_err_t core2foraws_expports_uart_write( const char *message, size_t length, size_t *was_written_length );
/* @[declare_core2foraws_expport_uart_write] */

/**
 * @brief Read bytes from the UART2 interface through @ref 
 * PORT_C_UART_RX_PIN (GPIO 13).
 *
 * This is a helper function to simplify building applications
 * with UART using 8 bit word length, no parity bit, 1 bit for the 
 * stop bit, and hardware flow control disabled. It sets the baud 
 * rate to the user defined value and starts reading from the UART 
 * channel 2 and storing data into the ring buffer of length @ref 
 * UART_RX_BUF_SIZE.
 *
 * @note Usage of the UART convenience methods provided in this BSP aims 
 * to simplify development at the expense of compatibility and 
 * performance. The configuration above may not be suitable for your 
 * application or attached peripheral. For more information about 
 * UART communications on the Core2 for AWS IoT EduKit using the 
 * ESP32 and how to create your own configuration, visit Espressif's 
 * official [documentation](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/peripherals/uart.html).
 *
 * @note The ESP32 is a 3.3v device and requires 3.3v on the UART 
 * TX/RX lines. Higher voltages requires the use of a level shifter.
 *
 * **Example:**
 * 
 * The example below sets @ref PORT_C_UART_TX_PIN (GPIO 14 to 
 * transmit) and PORT_C_UART_RX_PIN (GPIO 13 to receive), and sets 
 * the UART baud rate to 115200. It then starts two FreeRTOS tasks — 
 * one that transmits "Hello from AWS IoT EduKit" every two seconds 
 * and the other receives the message. The receiver task outputs the 
 * number of bytes read from the buffer, the number of bytes that 
 * remains unread in the ring buffer, and the message.
 *
 * @note To receive the messages transmitted on the same device, run a
 * female-female jumper wire from Port C's TX pin to PORT C's RX pin.
 * @code{c}
 *  #include <freertos/FreeRTOS.h>
 *  #include <freertos/task.h>
 *  #include <esp_log.h>
 *
 *  #include "core2foraws.h"
 *
 *  static const char *TAG = "MAIN_UART_DEMO";
 *
 *  static void uart_tx_task( void *pvParameters )
 *  {
 *      while ( 1 )
 *      {
 *          const char *message = "Hello from AWS IoT EduKit";
 *          size_t message_len = strlen( message ) + 1;
 *          size_t written_len = 0;
 * 
 *          core2foraws_expports_uart_write( message, message_len, &written_len );
 *          ESP_LOGI( TAG, "\tWrote %d out of % bytes", written_len, message_len );
 * 
 *          vTaskDelay( pdMS_TO_TICKS( 2000 ) );
 *      }
 *  }
 *
 *  static void uart_rx_task( void *arg )
 *  {
 *      size_t rxBytes;
 *      uint8_t *data = heap_caps_malloc( UART_RX_BUF_SIZE, MALLOC_CAP_SPIRAM ); // Allocate space for message in external RAM
 *      while ( 1 )
 *      {
 *          esp_err_t err = core2foraws_expports_uart_read( data, &rxBytes );
 *          if ( err == ESP_OK )
 *          {
 *              ESP_LOGI( TAG, "\tRead %d bytes from UART. Received: '%s'", rxBytes, data );
 *          }
 *          vTaskDelay( pdMS_TO_TICKS( 500 ) ); // Read more frequently than transmit to ensure the messages are not erased from buffer.
 *      }
 *      free(data); // Free memory from external RAM
 *  }
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *      esp_err_t err = ESP_FAIL;
 *      err = core2foraws_expports_uart_begin( 115200 );
 *      if ( err == ESP_OK )
 *      {
 *          xTaskCreatePinnedToCore( uart_rx_task, "uart_rx", 1024*2, NULL, configMAX_PRIORITIES-2, NULL, 1);
 *          xTaskCreatePinnedToCore( uart_tx_task, "uart_tx", 1024*2, NULL, configMAX_PRIORITIES-3, NULL, 1);
 *      }
 *  }
 * @endcode
 *
 * @param[out] message_buffer A pointer to the buffer read from 
 * UART2.
 * @param[out] was_read_length Pointer to the number of bytes read 
 * from @ref PORT_C_UART_RX_PIN (GPIO 13).
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK    : Success
 *  - ESP_FAIL	: Failed to write
 */
/* @[declare_core2foraws_expport_uart_read] */
esp_err_t core2foraws_expports_uart_read( uint8_t *message_buffer, size_t *was_read_length );
/* @[declare_core2foraws_expport_uart_read] */

#ifdef __cplusplus
}
#endif
#endif
