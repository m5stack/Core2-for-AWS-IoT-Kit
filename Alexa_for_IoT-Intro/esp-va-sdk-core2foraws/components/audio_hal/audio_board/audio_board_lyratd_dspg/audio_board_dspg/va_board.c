/*
*
* Copyright 2015-2018 Espressif Systems (Shanghai) PTE LTD
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include <string.h>
#include <esp_log.h>
#include <audio_board.h>
#include <led_pattern.h>
#include <is31fl3236.h>
#include <va_board.h>
#include <dspg_init.h>
#include <media_hal.h>
#include <media_hal_playback.h>

#define VA_TAG "AUDIO_BOARD"

#define I2S_PORT_NUM I2S_NUM_0

#define DBMD5_PIN_CONF_DEFAULT()         \
{                           \
    .apr_rst_pin = 21,      \
    .vt_rst_pin = 4,      \
    .apr_wakeup_pin = 2,      \
    .vt_intr_pin = 36,      \
    .pa_pin = 22,      \
    .level_shift = -1,  \
    .spi_mosi_pin = 19,      \
    .spi_miso_pin = 27,      \
    .spi_clk_pin = 13,      \
    .apr_cs_pin = 32,      \
    .vt_cs_pin = 33,       \
};

#ifdef CONFIG_PM_ENABLE
#define DBMD5_FW_CONF_DEFAULT()        \
{      \
    .fw_type = ALEXA_3_MIC,      \
    .enable_low_power = true,      \
    .grace_period = 5,             \
};
#else
#define DBMD5_FW_CONF_DEFAULT()        \
{      \
    .fw_type = ALEXA_3_MIC,      \
    .enable_low_power = false,      \
};
#endif

bool ab_but_mute = false;

static int but_cb_reg_handlr(int ui_but_evt)
{
    return 1;
}

static esp_err_t va_board_button_init()
{
    button_cfg_t *ab_button_conf = NULL;
    ab_button_conf = (button_cfg_t *)calloc(1, sizeof(button_cfg_t));
    ab_button_conf->is_adc = true;
    ab_button_conf->va_button_adc_ch_num = ADC1_CHANNEL_3;
    ab_button_conf->va_button_adc_val[VA_BUTTON_TAP_TO_TALK] = 600;
    ab_button_conf->va_button_adc_val[VA_BUTTON_VOLUME_UP] = 2480;
    ab_button_conf->va_button_adc_val[VA_BUTTON_VOLUME_DOWN] = 1830;
    ab_button_conf->va_button_adc_val[VA_BUTTON_VAL_IDLE] = 2700;
    ab_button_conf->va_button_adc_val[VA_BUTTON_MIC_MUTE] = 1230;
    ab_button_conf->va_button_adc_val[VA_BUTTON_FACTORY_RST] = 1530;
    ab_button_conf->va_button_adc_val[VA_BUTTON_CUSTOM_1] = -1;
    ab_button_conf->va_button_adc_val[VA_BUTTON_CUSTOM_2] = -1;
    ab_button_conf->tolerance = 80;
    va_button_init(ab_button_conf, but_cb_reg_handlr);
    //app_button_gpio_init();
    return ESP_OK;
}

extern esp_err_t is31fl3236_init(is31fl3236_led_config_t *led_config);

static esp_err_t va_board_led_init()
{
    led_pattern_config_t *ab_led_conf = NULL;
    led_pattern_init(&ab_led_conf);
    is31fl3236_led_config_t led_config = {
        .num_of_leds = 12,
        .start_pin = 0,
        .led_order = LED_COLOR_RED_GREEN_BLUE,
    };
    is31fl3236_init(&led_config);
    va_led_init((led_pattern_config_t *)ab_led_conf);
    return ESP_OK;
}

int va_board_init()
{
    int ret;
    i2s_config_t i2s_cfg = {};
    audio_board_i2s_init_default(&i2s_cfg);

    media_hal_playback_cfg_t cfg = {
        .channels = 2,
        .sample_rate = 48000,
        .i2s_port_num = I2S_NUM_0,
        .bits_per_sample = 16,
    };
    media_hal_init_playback(&cfg);

    ret = i2s_driver_install(I2S_PORT_NUM, &i2s_cfg, 0, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(VA_TAG, "Error installing i2s driver for stream");
    } else {
        i2s_pin_config_t pf_i2s_pin = {0};
        audio_board_i2s_pin_config(I2S_PORT_NUM, &pf_i2s_pin);
        i2s_set_pin(I2S_PORT_NUM, &pf_i2s_pin);
    }
    ret = i2s_zero_dma_buffer(I2S_PORT_NUM);

    static dbmd5_pin_conf_t pin_conf = DBMD5_PIN_CONF_DEFAULT();
    static dbmd5_firmware_conf_t fw_conf = DBMD5_FW_CONF_DEFAULT();
    dbmd5_configure(&pin_conf, &fw_conf);

    static media_hal_config_t media_hal_conf = MEDIA_HAL_DEFAULT();
    media_hal_init(&media_hal_conf);

    va_board_button_init();
    va_board_led_init();

    return ret;
}
