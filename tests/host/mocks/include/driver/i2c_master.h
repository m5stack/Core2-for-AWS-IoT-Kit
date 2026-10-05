/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "gpio.h"

typedef int i2c_port_num_t;
typedef struct test_bus *i2c_master_bus_handle_t;
typedef struct test_device *i2c_master_dev_handle_t;
enum { I2C_NUM_0, I2C_NUM_1 };
#define I2C_CLK_SRC_DEFAULT 0
#define I2C_ADDR_BIT_LEN_7 0
typedef struct {
    int i2c_port, sda_io_num, scl_io_num, clk_source, glitch_ignore_cnt;
    struct { bool enable_internal_pullup; } flags;
} i2c_master_bus_config_t;
typedef struct {
    int dev_addr_length;
    uint16_t device_address;
    uint32_t scl_speed_hz;
} i2c_device_config_t;

esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *config, i2c_master_bus_handle_t *bus);
esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus);
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus, const i2c_device_config_t *config, i2c_master_dev_handle_t *device);
esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t device);
esp_err_t i2c_master_receive(i2c_master_dev_handle_t device, uint8_t *data, size_t size, int timeout);
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t device, const uint8_t *data, size_t size, int timeout);
esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t device, const uint8_t *reg, size_t reg_size, uint8_t *data, size_t size, int timeout);