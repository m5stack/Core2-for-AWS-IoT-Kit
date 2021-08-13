#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

/**
 * @brief Used when the I2C peripheral does not use registers 
 * for read/write operations.
 * 
 * This macro is used in situations when the I2C peripheral
 * connected to port A does not use registers to perform read
 * or write operations to it. Since register addresses are
 * uint8, this 32-bit value exceeds the range of possible 
 * register addresses and is used to trigger logic to bypass 
 * the write to the register address.
 *
 */
/* @[declare_i2c_no_reg] */
#define I2C_NO_REG  ( 1 << 30 )
/* @[declare_i2c_no_reg] */

/**
 * @brief A pointer to the I2C peripheral configuration.
 * 
 * Contains configuration related to the peripheral in 
 * the following structure:
 * - addr       (device address)
 * - i2c_port
 *  - port     (I2C port number — 0 or 1)
 *  - sda      (I2C port's SDA pin)
 *  - scl      (I2C port's SCL pin)
 *  - freq     (I2C port frequency)
 *
 */
/* @[declare_i2cdevice_t] */
typedef void * I2CDevice_t;
/* @[declare_i2cdevice_t] */

I2CDevice_t i2c_malloc_device(i2c_port_t i2c_num, gpio_num_t sda, gpio_num_t scl, uint32_t freq, uint8_t device_addr);

void i2c_free_device(I2CDevice_t i2c_device);

esp_err_t i2c_apply_bus(I2CDevice_t i2c_device);

esp_err_t i2c_free_bus(I2CDevice_t i2c_device);

esp_err_t i2c_device_change_freq(I2CDevice_t i2c_device, uint32_t freq);

esp_err_t i2c_read_bytes(I2CDevice_t i2c_device, uint32_t reg_addr, uint8_t *data, uint16_t length);

esp_err_t i2c_read_byte(I2CDevice_t i2c_device, uint32_t reg_addr, uint8_t* data);

esp_err_t i2c_read_bit(I2CDevice_t i2c_device, uint32_t reg_addr, uint8_t *data, uint8_t bit_pos);

/*
    Read bits from 8 bit reg
    bit_pos = 4, bit_length = 3
    read ->  0b|1|0|1|0|1|1|0|0| 
             0b|-|x|x|x|-|-|-|-|   
    data = 0b00000010
*/
esp_err_t i2c_read_bits(I2CDevice_t i2c_device, uint32_t reg_addr, uint8_t *data, uint8_t bit_pos, uint8_t bit_length);

esp_err_t i2c_write_bytes(I2CDevice_t i2c_device, uint32_t reg_addr, uint8_t *data, uint16_t length);

esp_err_t i2c_read_bytes_no_stop(I2CDevice_t i2c_device, uint32_t reg_addr, uint8_t *data, uint16_t length);

esp_err_t i2c_write_byte(I2CDevice_t i2c_device, uint32_t reg_addr, uint8_t data);

esp_err_t i2c_write_bit(I2CDevice_t i2c_device, uint32_t reg_addr, uint8_t data, uint8_t bit_pos);

/*
    Read before bits from 8 bit reg, then update write bits
    1. Read data 0b10101100
    2. write, 0b0101, bit_pos = 4, bit_length = 3 
    read ->  0b|1|0|1|0|1|1|0|0| 
             0b|-|x|x|x|-|-|-|-| 
    write -> 0b|1|1|0|1|1|1|0|0|  
    data = 0b00000101
*/
esp_err_t i2c_write_bits(I2CDevice_t i2c_device, uint32_t reg_addr, uint8_t data, uint8_t bit_pos, uint8_t bit_length);

esp_err_t i2c_device_valid(I2CDevice_t i2c_device);

BaseType_t i2c_take_port(i2c_port_t i2c_num, uint32_t timeout);

BaseType_t i2c_free_port(i2c_port_t i2c_num);


#ifdef __cplusplus
}
#endif