/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdio.h>
#include "../../lib/common/core2foraws_i2c.c"

struct test_bus { bool active; };
struct test_device { bool active; };
static struct test_bus buses[2];
static struct test_device device_storage;
static int remove_calls;
static int transfer_calls;
static bool fail_remove;
static void (*before_lock)(void);
static uint8_t last_tx[128];
static size_t last_tx_size;
static TickType_t expected_lock_ticks = pdMS_TO_TICKS(1000);

int64_t esp_timer_get_time(void) { return 1; }

/* Open-drain bus model: a target holds SDA low for held_clocks SCL rising edges. */
static int sda_out = 1, scl_out = 1, held_clocks, rising_edges, stop_count, gpio_configs, pin_resets;
static gpio_num_t sda_pin = -1;
void esp_rom_delay_us(unsigned int microseconds) { (void)microseconds; }
esp_err_t gpio_config(const gpio_config_t *config)
{
    assert(config->mode == GPIO_MODE_INPUT_OUTPUT_OD && config->pull_up_en == GPIO_PULLUP_ENABLE);
    assert(sda_out == 1 && scl_out == 1);
    gpio_configs++;
    return ESP_OK;
}
esp_err_t gpio_reset_pin(int pin)
{
    (void)pin;
    pin_resets++;
    return ESP_OK;
}
int gpio_get_level(gpio_num_t pin)
{
    assert(pin == sda_pin);
    return sda_out && rising_edges >= held_clocks;
}
esp_err_t gpio_set_level(gpio_num_t pin, int level)
{
    if (pin == sda_pin)
    {
        if (!sda_out && level && scl_out) stop_count++;
        sda_out = level;
    }
    else
    {
        if (!scl_out && level) rising_edges++;
        scl_out = level;
    }
    return ESP_OK;
}
static void reset_bus_model(gpio_num_t sda, int held)
{
    sda_pin = sda;
    sda_out = scl_out = 1;
    held_clocks = held;
    rising_edges = stop_count = gpio_configs = pin_resets = 0;
}

SemaphoreHandle_t xSemaphoreCreateRecursiveMutexStatic(StaticSemaphore_t *storage)
{
    storage->depth = 0;
    return storage;
}
BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t mutex, TickType_t timeout)
{
    assert(timeout == expected_lock_ticks);
    if (before_lock != NULL)
    {
        void (*hook)(void) = before_lock;
        before_lock = NULL;
        hook();
    }
    mutex->depth++;
    return pdTRUE;
}
BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t mutex)
{
    assert(mutex->depth > 0);
    mutex->depth--;
    return pdTRUE;
}
TaskHandle_t xSemaphoreGetMutexHolder(SemaphoreHandle_t mutex) { (void)mutex; return NULL; }
void taskYIELD(void) { assert(!"First-use wait must block, not spin"); }
void vTaskDelay(TickType_t ticks)
{
    assert(ticks == 1);
    core2foraws_i2c_bus_state_t *state = &_bus_state[CORE2FORAWS_I2C_EXTERNAL];
    assert(atomic_load(&state->mutex_state) == I2C_MUTEX_INITIALIZING);
    state->mutex = xSemaphoreCreateRecursiveMutexStatic(&state->mutex_storage);
    atomic_store(&state->mutex_state, I2C_MUTEX_READY);
}

esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *config, i2c_master_bus_handle_t *bus)
{
    *bus = &buses[config->i2c_port];
    assert(!(*bus)->active);
    (*bus)->active = true;
    return ESP_OK;
}
esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus)
{
    assert(bus != NULL && bus->active);
    bus->active = false;
    return ESP_OK;
}
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus, const i2c_device_config_t *config, i2c_master_dev_handle_t *device)
{
    (void)config;
    assert(bus != NULL && bus->active);
    assert(!device_storage.active);
    device_storage.active = true;
    *device = &device_storage;
    return ESP_OK;
}
esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t device)
{
    assert(device->active);
    remove_calls++;
    if (fail_remove) return ESP_ERR_TIMEOUT;
    device->active = false;
    return ESP_OK;
}
esp_err_t i2c_master_receive(i2c_master_dev_handle_t device, uint8_t *data, size_t size, int timeout)
{
    assert(device->active && timeout == 1000);
    transfer_calls++;
    memset(data, 0x5a, size);
    return ESP_OK;
}
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t device, const uint8_t *data, size_t size, int timeout)
{
    assert(device->active && timeout == 1000 && size <= sizeof(last_tx));
    transfer_calls++;
    memcpy(last_tx, data, size);
    last_tx_size = size;
    return ESP_OK;
}
esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t device, const uint8_t *reg, size_t reg_size, uint8_t *data, size_t size, int timeout)
{
    assert(i2c_master_transmit(device, reg, reg_size, timeout) == ESP_OK);
    return i2c_master_receive(device, data, size, timeout);
}

static i2c_master_dev_handle_t open_device(void)
{
    i2c_master_dev_handle_t device = NULL;
    reset_bus_model(GPIO_NUM_32, 0);
    assert(core2foraws_i2c_init(CORE2FORAWS_I2C_EXTERNAL) == ESP_OK);
    assert(core2foraws_i2c_device_add(CORE2FORAWS_I2C_EXTERNAL, 0x42, 100000, &device) == ESP_OK);
    return device;
}
static void close_bus(void)
{
    assert(core2foraws_i2c_deinit(CORE2FORAWS_I2C_EXTERNAL) == ESP_OK);
}

int main(void)
{
    const core2foraws_i2c_port_t port = CORE2FORAWS_I2C_EXTERNAL;
    i2c_master_bus_handle_t uninitialized_bus = NULL;
    i2c_master_dev_handle_t uninitialized_device = &device_storage;
    uint8_t uninitialized_data = 0;
    assert(core2foraws_i2c_get_bus_handle(port, &uninitialized_bus) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_i2c_device_add(port, 0x42, 100000, &uninitialized_device) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_i2c_read(port, uninitialized_device, 0, &uninitialized_data, 1) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_i2c_write(port, uninitialized_device, 0, &uninitialized_data, 1) == ESP_ERR_INVALID_STATE);
    atomic_store(&_bus_state[port].mutex_state, I2C_MUTEX_INITIALIZING);
    i2c_master_dev_handle_t device = open_device();
    i2c_master_dev_handle_t shared = NULL;
    assert(core2foraws_i2c_device_add(port, 0x42, 100000, &shared) == ESP_OK);
    assert(shared == device);
    assert(core2foraws_i2c_device_remove(shared) == ESP_OK && remove_calls == 0);
    fail_remove = true;
    assert(core2foraws_i2c_device_remove(device) == ESP_ERR_TIMEOUT);
    fail_remove = false;
    assert(core2foraws_i2c_device_remove(device) == ESP_OK);
    assert(remove_calls == 2 && !device_storage.active);
    uint8_t data[64] = { 0x12, 0x34 };
    assert(core2foraws_i2c_read(port, device, 0, data, 1) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_i2c_write(port, device, 0, data, 1) == ESP_ERR_INVALID_STATE);
    assert(transfer_calls == 0);
    close_bus();

    device = open_device();
    assert(core2foraws_i2c_write(port, device, CORE2FORAWS_I2C_REG_16 | 0xabcd, data, 2) == ESP_OK);
    assert(last_tx_size == 4 && last_tx[0] == 0xab && last_tx[1] == 0xcd && last_tx[2] == 0x12);
    assert(core2foraws_i2c_write(port, device, 0x20, data, sizeof(data)) == ESP_OK);
    assert(last_tx_size == 65 && last_tx[0] == 0x20);
    assert(core2foraws_i2c_read(port, device, CORE2FORAWS_I2C_NO_REG, data, 1) == ESP_OK);
    assert(data[0] == 0x5a);
    assert(core2foraws_i2c_write(port, device, 0x11, NULL, 0) == ESP_OK);
    assert(last_tx_size == 1 && last_tx[0] == 0x11);
    close_bus();

    device = open_device();
    before_lock = close_bus;
    assert(core2foraws_i2c_read(port, device, 0, data, 1) == ESP_ERR_INVALID_STATE);
    device = open_device();
    before_lock = close_bus;
    assert(core2foraws_i2c_write(port, device, 0, data, 1) == ESP_ERR_INVALID_STATE);
    device = open_device();
    before_lock = close_bus;
    assert(core2foraws_i2c_device_add(port, 0x43, 100000, &shared) == ESP_ERR_INVALID_STATE);
    assert(!device_storage.active);
    assert(core2foraws_i2c_deinit(CORE2FORAWS_I2C_INTERNAL) == ESP_ERR_NOT_SUPPORTED);
    assert(core2foraws_i2c_init((core2foraws_i2c_port_t)-1) == ESP_ERR_INVALID_ARG);

    expected_lock_ticks = CORE2FORAWS_DELAY_MS_TO_TICKS(20);
    assert(core2foraws_i2c_lock_timeout(port, 20) == ESP_OK);
    expected_lock_ticks = pdMS_TO_TICKS(1000);
    assert(core2foraws_i2c_unlock(port) == ESP_OK);

    assert(core2foraws_i2c_init(port) == ESP_OK);
    i2c_master_dev_handle_t wake = NULL;
    assert(core2foraws_i2c_device_add(port, 0x00, 100000, &wake) == ESP_OK);
    uint8_t token = 0;
    assert(_bus_state[port].last_general_call_us == 0);
    assert(core2foraws_i2c_write(port, wake, CORE2FORAWS_I2C_NO_REG, &token, 1) == ESP_OK);
    assert(_bus_state[port].last_general_call_us == 1);
    close_bus();

    reset_bus_model(GPIO_NUM_32, 0);
    assert(core2foraws_i2c_init(port) == ESP_OK);
    assert(gpio_configs == 1 && pin_resets == 2 && rising_edges == 0 && stop_count == 0);
    close_bus();

    reset_bus_model(GPIO_NUM_32, 3);
    assert(core2foraws_i2c_init(port) == ESP_OK);
    assert(rising_edges == 4 && stop_count == 1 && pin_resets == 2 && sda_out == 1 && scl_out == 1);
    close_bus();

    reset_bus_model(GPIO_NUM_32, 100);
    assert(core2foraws_i2c_init(port) == ESP_OK);
    assert(rising_edges == I2C_BUS_CLEAR_PULSES + 1 && sda_out == 1 && scl_out == 1);
    close_bus();

    assert(_bus_state[port].mutex->depth == 0);
    puts("I2C lifetime, retry, transfer, close-interleaving, bounded lock, wake-token timing and bus-clear tests passed");
    return 0;
}