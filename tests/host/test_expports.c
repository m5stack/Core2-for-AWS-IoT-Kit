/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdio.h>
#include "core2foraws_i2c.h"
#define COMMON_I2C_EXTERNAL CORE2FORAWS_I2C_EXTERNAL
#include "../../lib/expports/core2foraws_expports.c"

static bool i2c_active, uart_active;
static int routing[40];
static esp_err_t close_error, uart_pin_error, uart_config_error;
static unsigned int bus_creations, uart_creations;

SemaphoreHandle_t xSemaphoreCreateRecursiveMutexStatic(StaticSemaphore_t *storage) { return storage; }
BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t mutex, TickType_t timeout) { assert(timeout == 100); mutex->depth++; return pdTRUE; }
BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t mutex) { assert(mutex->depth > 0); mutex->depth--; return pdTRUE; }
void vTaskDelay(TickType_t ticks) { assert(ticks == 1); }
esp_err_t gpio_reset_pin(int pin)
{
    assert(!((pin == 32 || pin == 33) && i2c_active));
    assert(!((pin == 13 || pin == 14) && uart_active));
    routing[pin] = NONE;
    return ESP_OK;
}
esp_err_t gpio_config(const gpio_config_t *config)
{
    for (unsigned int pin = 0; pin < 40; ++pin)
        if (config->pin_bit_mask & (UINT64_C(1) << pin)) routing[pin] = config->mode == GPIO_MODE_OUTPUT ? OUTPUT : INPUT;
    return ESP_OK;
}
int gpio_get_level(gpio_num_t pin) { assert(routing[pin] == INPUT); return 0; }
esp_err_t gpio_set_level(gpio_num_t pin, int level) { (void)level; assert(routing[pin] == OUTPUT); return ESP_OK; }
esp_err_t core2foraws_i2c_init(core2foraws_i2c_port_t port)
{
    assert(port == CORE2FORAWS_I2C_EXTERNAL);
    if (!i2c_active)
    {
        assert(routing[32] == NONE && routing[33] == NONE);
        routing[32] = routing[33] = I2C;
        i2c_active = true;
        bus_creations++;
    }
    return ESP_OK;
}
esp_err_t core2foraws_i2c_deinit(core2foraws_i2c_port_t port)
{
    assert(port == CORE2FORAWS_I2C_EXTERNAL);
    if (close_error != ESP_OK) return close_error;
    i2c_active = false;
    return ESP_OK;
}
esp_err_t core2foraws_i2c_device_add(core2foraws_i2c_port_t port, uint16_t address, uint32_t speed, i2c_master_dev_handle_t *handle)
{ (void)port; (void)address; (void)speed; (void)handle; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t core2foraws_i2c_device_remove(i2c_master_dev_handle_t handle) { (void)handle; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t core2foraws_i2c_read(core2foraws_i2c_port_t port, i2c_master_dev_handle_t handle, uint32_t reg, uint8_t *data, uint16_t size)
{ (void)port; (void)handle; (void)reg; (void)data; (void)size; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t core2foraws_i2c_write(core2foraws_i2c_port_t port, i2c_master_dev_handle_t handle, uint32_t reg, const uint8_t *data, uint16_t size)
{ (void)port; (void)handle; (void)reg; (void)data; (void)size; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t uart_driver_install(int port, int rx_size, int tx_size, int queue_size, void *queue, int flags)
{
    (void)rx_size; (void)tx_size; (void)queue_size; (void)queue; (void)flags;
    assert(port == 2 && !uart_active && routing[13] == NONE && routing[14] == NONE);
    uart_active = true;
    uart_creations++;
    return ESP_OK;
}
esp_err_t uart_driver_delete(int port)
{
    assert(port == 2 && uart_active);
    if (close_error != ESP_OK) return close_error;
    uart_active = false;
    return ESP_OK;
}
esp_err_t uart_set_pin(int port, int tx, int rx, int rts, int cts)
{
    (void)rts; (void)cts;
    assert(port == 2 && uart_active && tx == 14 && rx == 13);
    if (uart_pin_error != ESP_OK) return uart_pin_error;
    routing[13] = routing[14] = UART;
    return ESP_OK;
}
esp_err_t uart_param_config(int port, const uart_config_t *config) { assert(port == 2 && config->baud_rate > 0); return uart_config_error; }
esp_err_t uart_get_buffered_data_len(int port, size_t *length) { (void)port; *length = 0; return ESP_OK; }
int uart_read_bytes(int port, void *buffer, size_t size, TickType_t timeout) { (void)port; (void)buffer; (void)size; (void)timeout; return 0; }
int uart_write_bytes(int port, const void *buffer, size_t size) { (void)port; (void)buffer; return (int)size; }
esp_err_t uart_wait_tx_done(int port, TickType_t timeout) { (void)port; (void)timeout; return ESP_OK; }
esp_err_t uart_flush_input(int port) { (void)port; return ESP_OK; }
esp_err_t dac_oneshot_new_channel(const dac_oneshot_config_t *config, dac_oneshot_handle_t *handle) { (void)config; (void)handle; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t dac_oneshot_del_channel(dac_oneshot_handle_t handle) { (void)handle; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t dac_oneshot_output_voltage(dac_oneshot_handle_t handle, int value) { (void)handle; (void)value; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t adc_oneshot_new_unit(const adc_oneshot_unit_init_cfg_t *config, adc_oneshot_unit_handle_t *handle) { (void)config; (void)handle; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t adc_oneshot_config_channel(adc_oneshot_unit_handle_t handle, int channel, const adc_oneshot_chan_cfg_t *config) { (void)handle; (void)channel; (void)config; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t adc_oneshot_del_unit(adc_oneshot_unit_handle_t handle) { (void)handle; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t adc_oneshot_read(adc_oneshot_unit_handle_t handle, int channel, int *value) { (void)handle; (void)channel; (void)value; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t adc_cali_raw_to_voltage(adc_cali_handle_t handle, int raw, int *voltage) { (void)handle; (void)raw; (void)voltage; return ESP_ERR_NOT_SUPPORTED; }

int main(void)
{
    assert(core2foraws_expports_digital_write(32, false) == ESP_OK);
    assert(core2foraws_expports_digital_write(33, false) == ESP_OK);
    assert(core2foraws_expports_i2c_begin() == ESP_OK);
    assert(core2foraws_expports_i2c_begin() == ESP_OK && bus_creations == 1);
    close_error = ESP_ERR_TIMEOUT;
    assert(core2foraws_expports_pin_reset(32) == ESP_ERR_TIMEOUT);
    assert(core2foraws_expports_i2c_close() == ESP_ERR_TIMEOUT);
    assert(routing[32] == I2C && routing[33] == I2C && i2c_active);
    close_error = ESP_OK;
    assert(core2foraws_expports_digital_write(33, false) == ESP_OK);
    assert(!i2c_active && routing[32] == NONE && routing[33] == OUTPUT);
    assert(core2foraws_expports_i2c_begin() == ESP_OK && bus_creations == 2);
    assert(routing[32] == I2C && routing[33] == I2C);
    assert(core2foraws_expports_i2c_close() == ESP_OK);
    assert(core2foraws_expports_i2c_close() == ESP_OK);
    assert(core2foraws_expports_digital_write(13, false) == ESP_OK);
    assert(core2foraws_expports_digital_write(14, false) == ESP_OK);
    assert(core2foraws_expports_uart_begin(115200) == ESP_OK);
    assert(core2foraws_expports_uart_begin(9600) == ESP_OK && uart_creations == 1);
    close_error = ESP_ERR_TIMEOUT;
    assert(core2foraws_expports_pin_reset(14) == ESP_ERR_TIMEOUT);
    assert(routing[13] == UART && routing[14] == UART && uart_active);
    close_error = ESP_OK;
    assert(core2foraws_expports_digital_write(13, false) == ESP_OK);
    assert(!uart_active && routing[13] == OUTPUT && routing[14] == NONE);
    assert(core2foraws_expports_uart_begin(115200) == ESP_OK && uart_creations == 2);
    assert(core2foraws_expports_pin_reset(14) == ESP_OK);
    uart_pin_error = ESP_FAIL;
    close_error = ESP_ERR_TIMEOUT;
    assert(core2foraws_expports_uart_begin(115200) == ESP_FAIL);
    assert(uart_active);
    close_error = ESP_OK;
    uart_pin_error = ESP_OK;
    assert(core2foraws_expports_uart_begin(115200) == ESP_OK);
    assert(routing[13] == UART && routing[14] == UART);
    assert(core2foraws_expports_pin_reset(13) == ESP_OK && !uart_active);
    uart_config_error = ESP_FAIL;
    assert(core2foraws_expports_uart_begin(115200) == ESP_FAIL && !uart_active);
    assert(routing[13] == NONE && routing[14] == NONE);
    assert(_expports_mutex->depth == 0);
    puts("Expansion paired routing, reopen, delete failure and startup rollback tests passed");
    return 0;
}