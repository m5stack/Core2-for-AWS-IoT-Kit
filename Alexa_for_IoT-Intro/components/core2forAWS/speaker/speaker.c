#include "freertos/FreeRTOS.h"
#include "speaker.h"
#include "driver/i2s.h"
#include "esp_idf_version.h"

#define I2S_BCK_PIN 12
#define I2S_LRCK_PIN 0
#define I2S_DATA_PIN 2
#define I2S_DATA_IN_PIN 34
#define SPEAKER_I2S_NUMBER I2S_NUM_0

void Speaker_Init() {
    esp_err_t err = ESP_OK;
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 1, 0)
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
#else
        .communication_format = I2S_COMM_FORMAT_I2S,
#endif
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = 128,
    };

    i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
    i2s_config.use_apll = false;
    i2s_config.tx_desc_auto_clear = true;
    err += i2s_driver_install(SPEAKER_I2S_NUMBER, &i2s_config, 0, NULL);

    i2s_pin_config_t tx_pin_config;
    tx_pin_config.bck_io_num = I2S_BCK_PIN;
    tx_pin_config.ws_io_num = I2S_LRCK_PIN;
    tx_pin_config.data_out_num = I2S_DATA_PIN;
    tx_pin_config.data_in_num = I2S_DATA_IN_PIN;
    err += i2s_set_pin(SPEAKER_I2S_NUMBER, &tx_pin_config);
    err += i2s_set_clk(SPEAKER_I2S_NUMBER, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void Speaker_WriteBuff(uint8_t* buff, uint32_t len, uint32_t timeout) {
    size_t bytes_written = 0;
    i2s_write(SPEAKER_I2S_NUMBER, buff, len, &bytes_written, portMAX_DELAY);
}

void Speaker_Deinit() {
    i2s_driver_uninstall(SPEAKER_I2S_NUMBER);
    gpio_reset_pin(I2S_LRCK_PIN);
    gpio_reset_pin(I2S_DATA_PIN);
    gpio_reset_pin(I2S_BCK_PIN);
    gpio_reset_pin(I2S_DATA_IN_PIN);
}