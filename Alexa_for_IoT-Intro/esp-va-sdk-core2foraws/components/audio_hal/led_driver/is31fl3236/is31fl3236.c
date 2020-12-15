// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#include <va_mem_utils.h>
#include <is31fl3236.h>
#include <va_dsp.h>
#include "audio_board.h"

//#define EN_STACK_MEASUREMENT
#define UI_LED_QUEUE_LENGTH 3

static const char *UI_LED_TAG = "UI_LED";

SemaphoreHandle_t is31fl3236_led_mux ;

#define LED_TAG "LED"

#define IS31_ERROR_CHECK(con) if(!(con)) {ESP_LOGE(LED_TAG,"err line: %d", __LINE__); return ESP_FAIL;}
#define IS31_PARAM_CHECK(con) if(!(con)) {ESP_LOGE(LED_TAG,"Parameter error, "); return ESP_FAIL;}
#define IS31_CHECK_I2C_RES(res) if(ret == ESP_FAIL) {ESP_LOGE(LED_TAG, "Is31fl3216[%s]: FAIL\n", __FUNCTION__);} \
                                else if(ret == ESP_ERR_TIMEOUT) {ESP_LOGE(LED_TAG, "Is31fl3216[%s]: TIMEOUT\n", __FUNCTION__);}
#define LED_ASSERT(a, format, b, ...) \
    if ((a) != 0) { \
        ESP_LOGE(LED_TAG, format, ##__VA_ARGS__); \
        return b;\
    }

#define IS31FL3236_WRITE_BIT    0x00
#define IS31FL3236_READ_BIT     0x01
#define IS31FL3236_ACK_CHECK_EN 1

#define I2C_MASTER_SCL_IO    23        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO    18        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM     I2C_NUM_0   /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE   0  /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0  /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ    100000   /*!< I2C master clock frequency */


typedef struct {
 //   i2c_bus_handle_t bus;
    uint16_t addr;
} Is31fl3216Dev;

static is31fl3236_led_config_t is31fl3236_led;

static struct led_colors {
    int red;
    int green;
    int blue;
} led_color_offset[LED_COLOR_MAX_PATTERNS] = {
    [LED_COLOR_RED_BLUE_GREEN] = {1, 3, 2},
    [LED_COLOR_RED_GREEN_BLUE] = {1, 2, 3},
    [LED_COLOR_BLUE_RED_GREEN] = {2, 3, 1},
    [LED_COLOR_BLUE_GREEN_RED] = {3, 2, 1},
    [LED_COLOR_GREEN_RED_BLUE] = {2, 1, 3},
    [LED_COLOR_GREEN_BLUE_RED] = {3, 1, 2},
};

/**
 * @brief Initialization function for i2c
 */
static esp_err_t ui_led_i2c_init(int i2c_master_port)
{
    int res;
    i2c_config_t pf_i2c_pin = {0};
    
    res = audio_board_i2c_pin_config(i2c_master_port, &pf_i2c_pin);

    pf_i2c_pin.mode = I2C_MODE_MASTER;  
    pf_i2c_pin.master.clk_speed = I2C_MASTER_FREQ_HZ;
    
    res |= i2c_param_config(i2c_master_port, &pf_i2c_pin);
    res |= i2c_driver_install(i2c_master_port, pf_i2c_pin.mode, 0, 0, 0);
    return res;
}

/**
 * @brief Write IS31 register
 *
 * @param slave_add : slave address
 * @param reg_add    : register address
 * @param data      : data to write
 *
 * @return
 *     - (-1)  Error
 *     - (0)   Success
 */
static esp_err_t is31fl3236_write_reg(uint8_t slave_add, uint8_t reg_add, uint8_t data)
{
    int res = 0;
    xSemaphoreTake(is31fl3236_led_mux, portMAX_DELAY);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, slave_add, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, reg_add, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, data, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    xSemaphoreGive(is31fl3236_led_mux);
    LED_ASSERT(res, "is31fl3236_write_reg error", -1);
    return res;
}

/**
 * @brief Read IS31 register
 *
 * @param reg_add    : register address
 *
 * @return
 *     - (-1)     Error
 *     - (0)      Success
 */
static esp_err_t ui_led_read_reg(uint8_t reg_add, uint8_t *p_data)
{
    uint8_t data;
    esp_err_t res;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    res  = i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, IS31FL3236_ADDRESS, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, reg_add, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, IS31FL3236_ADDRESS | 0x01, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_read_byte(cmd, &data, 0x01/*NACK_VAL*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    LED_ASSERT(res, "ui_led_read_reg error", -1);
    *p_data = data;
    return res;
}

esp_err_t is31fl3236_init(is31fl3236_led_config_t *led_config)
{

    esp_err_t res;
    if (led_config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    is31fl3236_led.num_of_leds = led_config->num_of_leds;
    is31fl3236_led.start_pin = led_config->start_pin;
    is31fl3236_led.led_order = led_config->led_order;
    vSemaphoreCreateBinary(is31fl3236_led_mux);
    if (is31fl3236_led_mux == NULL) {
        ESP_LOGE(LED_TAG, "Could not create semaphore");
        return ESP_FAIL;
    }
    int i;
    res = is31fl3236_write_reg(IS31FL3236_ADDRESS, 0x00, 0x01);
    for( i = 0; i<36; i++) {
        res |= is31fl3236_write_reg(IS31FL3236_ADDRESS, (0x26 + i), 0x01);
        //is31fl3236_write_reg(IS31FL3236_ADDRESS, 0x25, 0x00);
    }
    res = is31fl3236_write_reg(IS31FL3236_ADDRESS, 0x4A, 0x00);
    return res;
}

void va_led_set_pwm(const uint32_t * led_value)
{
    int i;
    uint8_t red_led_val, blue_led_val, green_led_val;
    for (i = 0; i < is31fl3236_led.num_of_leds; i++) {
        blue_led_val  = 0xFF & (led_value[i]);
        green_led_val = 0xFF & (led_value[i] >> 8);
        red_led_val   = 0xFF & (led_value[i] >> 16);
        is31fl3236_write_reg(IS31FL3236_ADDRESS, (((i + is31fl3236_led.start_pin) * 3) + led_color_offset[is31fl3236_led.led_order].red), red_led_val);
        //is31fl3236_write_reg(IS31FL3236_ADDRESS, 0x25, 0x00);
        is31fl3236_write_reg(IS31FL3236_ADDRESS, (((i + is31fl3236_led.start_pin) * 3) + led_color_offset[is31fl3236_led.led_order].green), green_led_val);
        //is31fl3236_write_reg(IS31FL3236_ADDRESS, 0x25, 0x00);
        is31fl3236_write_reg(IS31FL3236_ADDRESS, (((i + is31fl3236_led.start_pin) * 3) + led_color_offset[is31fl3236_led.led_order].blue), blue_led_val);
    }
    is31fl3236_write_reg(IS31FL3236_ADDRESS, 0x25, 0x00);
}

