/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "core2foraws_i2c.h"
#define COMMON_I2C_INTERNAL CORE2FORAWS_I2C_INTERNAL
#include "../../lib/power/core2foraws_power.c"

static uint8_t registers[256];
static unsigned int bus_writes;
static unsigned int lock_depth;

esp_err_t core2foraws_i2c_device_add(core2foraws_i2c_port_t port, uint16_t address,
                                  uint32_t speed, i2c_master_dev_handle_t *device)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && address == 0x34 && speed == 400000);
    *device = (i2c_master_dev_handle_t)registers;
    return ESP_OK;
}
esp_err_t core2foraws_i2c_read(core2foraws_i2c_port_t port, i2c_master_dev_handle_t device,
                             uint32_t reg, uint8_t *buffer, uint16_t size)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && device != NULL && reg + size <= 256);
    memcpy(buffer, registers + reg, size);
    return ESP_OK;
}
esp_err_t core2foraws_i2c_write(core2foraws_i2c_port_t port, i2c_master_dev_handle_t device,
                              uint32_t reg, const uint8_t *buffer, uint16_t size)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && device != NULL && reg + size <= 256);
    bus_writes++;
    memcpy(registers + reg, buffer, size);
    return ESP_OK;
}
esp_err_t core2foraws_i2c_lock(core2foraws_i2c_port_t port)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL);
    lock_depth++;
    return ESP_OK;
}
esp_err_t core2foraws_i2c_unlock(core2foraws_i2c_port_t port)
{
    assert(port == CORE2FORAWS_I2C_INTERNAL && lock_depth > 0);
    lock_depth--;
    return ESP_OK;
}
esp_err_t core2foraws_common_error(esp_err_t error) { return error == ESP_OK ? ESP_OK : ESP_FAIL; }
static int64_t fake_time_us;
int64_t esp_timer_get_time(void) { return fake_time_us; }
/* vTaskDelay( n ) may return up to one tick early; model the worst case. */
void vTaskDelay(TickType_t ticks) { fake_time_us += (int64_t)(ticks - 1) * 10000; }

int main(void)
{
    registers[0x12] = 0x01;
    registers[0x26] = 0x68;
    assert(core2foraws_power_init() == ESP_OK);
    assert((registers[0x12] & 1) != 0 && (registers[0x26] & 0x7f) == 0x6a);
    /* Reset low for >= 100 ms, but init no longer blocks for the 300 ms boot. */
    assert(fake_time_us >= 100000 && fake_time_us < 200000);
    int64_t released_us = fake_time_us;
    core2foraws_power_lcd_ready_wait();
    assert(fake_time_us - released_us >= 300000);
    int64_t after_wait = fake_time_us;
    core2foraws_power_lcd_ready_wait();
    assert(fake_time_us == after_wait);
    unsigned int before = bus_writes;
    assert(core2foraws_power_init() == ESP_OK && bus_writes == before);
    assert(core2foraws_power_rail_state_set(POWER_RAIL_ESP32, false) == ESP_ERR_NOT_SUPPORTED);
    const uint16_t unsafe_mv[] = { 0, 700, 3300, 3500, UINT16_MAX };
    for (size_t index = 0; index < sizeof(unsafe_mv) / sizeof(unsafe_mv[0]); ++index)
        assert(core2foraws_power_rail_mv_set(POWER_RAIL_ESP32, unsafe_mv[index]) == ESP_ERR_NOT_SUPPORTED);
    assert(core2foraws_power_axp_reg_set(0x12, 0xfe) == ESP_ERR_NOT_SUPPORTED);
    assert(core2foraws_power_axp_reg_set(0x26, 0x68) == ESP_ERR_NOT_SUPPORTED);
    uint8_t unsafe_value = 0;
    assert(core2foraws_power_axp_write(0x12, &unsafe_value) == ESP_ERR_NOT_SUPPORTED);
    assert(core2foraws_power_axp_write(0x26, &unsafe_value) == ESP_ERR_NOT_SUPPORTED);
    assert(core2foraws_power_axp_twiddle(0x12, 1, 0) == ESP_ERR_NOT_SUPPORTED);
    assert(core2foraws_power_axp_twiddle(0x26, 0x7f, 0x68) == ESP_ERR_NOT_SUPPORTED);
    const uint8_t crossing_voltage[] = { 0, 0 };
    assert(_axp192_i2c_write(NULL, 0x34, 0x25, crossing_voltage, 2) == ESP_ERR_NOT_SUPPORTED);
    assert(_axp192_i2c_write(NULL, 0x34, 0xff, crossing_voltage, 2) == ESP_ERR_INVALID_ARG);
    assert(bus_writes == before);
    assert((registers[0x12] & 1) != 0 && (registers[0x26] & 0x7f) == 0x6a);

    assert(core2foraws_power_rail_mv_set(POWER_RAIL_ESP32, POWER_MCU_MILLIVOLTS) == ESP_OK);
    assert(core2foraws_power_rail_state_set(POWER_RAIL_ESP32, true) == ESP_OK);
    assert(core2foraws_power_backlight_set(0) == ESP_OK);
    assert((registers[0x12] & 1) != 0 && (registers[0x12] & 2) == 0);
    assert(core2foraws_power_backlight_set(25) == ESP_OK && registers[0x27] == 0x47);
    assert(core2foraws_power_vibration_enable(true) == ESP_OK);
    assert(core2foraws_power_vibration_enable(false) == ESP_OK);
    assert(core2foraws_power_led_enable(false) == ESP_OK);
    assert(core2foraws_power_led_enable(true) == ESP_OK);
    assert((registers[0x12] & 1) != 0 && (registers[0x26] & 0x7f) == 0x6a);
    assert(lock_depth == 0);
    registers[0x32] = 0x46;
    uint8_t snapshot[sizeof(registers)];
    memcpy(snapshot, registers, sizeof(registers));
    before = bus_writes;
    assert(core2foraws_power_off() == ESP_OK);
    assert(bus_writes == before + 1);
    snapshot[0x32] = 0xc6;
    assert(memcmp(snapshot, registers, sizeof(registers)) == 0);
    assert(lock_depth == 0);
    registers[0x32] = 0x46;
    assert(core2foraws_power_axp_reg_set(0x32, 0xc6) == ESP_OK);
    assert(core2foraws_power_axp_write(0x32, &snapshot[0x32]) == ESP_OK);
    puts("MCU rail guard, recoverable shutdown, startup, LCD start-up overlap, and peripheral-control tests passed");
    return 0;
}