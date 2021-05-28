#include "freertos/FreeRTOS.h"
#include "esp_idf_version.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "microphone.h"

#define I2S_LRCK_PIN 0
#define I2S_DATA_IN_PIN 34

void Microphone_Init() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 1, 0)
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
#else
		.communication_format = I2S_COMM_FORMAT_I2S,
#endif
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = 128,
    };

    i2s_pin_config_t pin_config;
    pin_config.bck_io_num = I2S_PIN_NO_CHANGE;
    pin_config.ws_io_num = I2S_LRCK_PIN;
    pin_config.data_out_num = I2S_PIN_NO_CHANGE;
    pin_config.data_in_num = I2S_DATA_IN_PIN;
    
    i2s_driver_install(MIC_I2S_NUMBER, &i2s_config, 0, NULL);
    i2s_set_pin(MIC_I2S_NUMBER, &pin_config);
    i2s_set_clk(MIC_I2S_NUMBER, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void Microphone_Deinit() {
    i2s_driver_uninstall(MIC_I2S_NUMBER);
    gpio_reset_pin(GPIO_NUM_0);
    gpio_reset_pin(GPIO_NUM_34);
}