// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include <esp_wwe.h>
#include <i2s_stream.h>
#include <audio_board.h>
#include <media_hal.h>
#include <esp_audio_mem.h>
#include <basic_rb.h>
#include <resampling.h>
#include <va_dsp.h>
#include <lyrat_init.h>

#define WWE_TASK_STACK (8 * 1024)
#define RB_TASK_STACK (8 * 1024)
#define RB_SIZE (4 * 1024)

#define DETECT_SAMP_RATE 16000UL
#define SAMP_BITS I2S_BITS_PER_SAMPLE_16BIT
#define SAMP_MS 20

static const char *TAG = "[lyrat_init]";

static struct dsp_data {
    int item_chunk_size;
    bool detect_wakeword;
    bool mic_mute_enabled;
    rb_handle_t raw_mic_data;
    rb_handle_t resampled_mic_data;
    audio_resample_config_t resample;
    i2s_stream_t *read_i2s_stream;
    TaskHandle_t ww_detection_task_handle;
    int16_t *data_buf;
    uint32_t data_sample_size;
    uint32_t sample_rate;
    int channels;
    int i2s_number; /* 0 or 1 */
    bool set_i2s_clk;
} dd = {
    .set_i2s_clk = true,
    .sample_rate = SAMPLE_RATE_DEFAULT,
    .channels = CHANNELS_DEFAULT,
    .i2s_number = I2S_NUMBER_DEFAULT
};

int lyrat_stream_audio(uint8_t *buffer, int size, int wait);

static void ww_detection_task(void *arg)
{
    int frequency = esp_wwe_get_sample_rate();
    int audio_chunksize = esp_wwe_get_sample_chunksize();

    int16_t *buffer = malloc(audio_chunksize*sizeof(int16_t));
    assert(buffer);
    int chunks=0;
    int priv_ms = 0;
    while(1) {
        if (dd.detect_wakeword) {
            lyrat_stream_audio((uint8_t *)buffer, (audio_chunksize * sizeof(int16_t)), portMAX_DELAY);
            int r = esp_wwe_detect(buffer);
            if (r && dd.detect_wakeword) {
                int new_ms = (chunks*audio_chunksize*1000)/frequency;
                printf("%.2f: Neural network detection triggered output %d.\n", (float)new_ms/1000.0, r);
                int x = (new_ms - priv_ms);
                priv_ms = new_ms;
                if(x != 20) {
                    va_dsp_tap_to_talk_start();
                }
            }
            chunks++;
        } else {
            memset(buffer, 0, (audio_chunksize * sizeof(int16_t)));
            vTaskDelay(100/portTICK_RATE_MS);
        }
    }
}

static esp_err_t reader_stream_event_handler(void *arg, int event, void *data)
{
    ESP_LOGI(TAG, "Reader stream event %d", event);
    return ESP_OK;
}

static ssize_t dsp_write_cb(void *h, void *data, int len, uint32_t wait)
{
    ssize_t sent_len;
    if(len <= 0) {
        return 0;
    }
    sent_len = rb_write(dd.raw_mic_data, data, len, wait);
    return sent_len;
}

static void resample_rb_data_task(void *arg)
{
    size_t sent_len;
    while(1) {
        sent_len = rb_read(dd.raw_mic_data, (uint8_t *)dd.data_buf, dd.data_sample_size * 2, portMAX_DELAY);
        if (dd.mic_mute_enabled) {
            // Drop the data.
            vTaskDelay(200/portTICK_RATE_MS);
        } else {
            sent_len = audio_resample((short *)dd.data_buf, (short *)dd.data_buf, dd.sample_rate, DETECT_SAMP_RATE,
                                      dd.data_sample_size, dd.data_sample_size, dd.channels, &dd.resample);
            if (dd.channels == 2) {
                sent_len = audio_resample_down_channel((short *)dd.data_buf, (short *)dd.data_buf, DETECT_SAMP_RATE,
                                                       DETECT_SAMP_RATE, sent_len, dd.data_sample_size, 0, &dd.resample);
            }
            sent_len = sent_len * 2;  //convert 16bit length to number of bytes
            rb_write(dd.resampled_mic_data, (uint8_t *)dd.data_buf, sent_len, portMAX_DELAY);
        }
    }
}

int lyrat_stream_audio(uint8_t *buffer, int size, int wait)
{
    return rb_read(dd.resampled_mic_data, buffer, size, wait);
}

void lyrat_stop_capture()
{
    dd.detect_wakeword = true;
}

void lyrat_start_capture()
{
    dd.detect_wakeword = false;
}

void lyrat_mic_mute()
{
    dd.mic_mute_enabled = true;
    dd.detect_wakeword = false;
}

void lyrat_mic_unmute()
{
    dd.mic_mute_enabled = false;
    dd.detect_wakeword = true;
}

void lyrat_configure(lyrat_config_t *cfg)
{
    if (cfg) {
        dd.sample_rate = cfg->sample_rate;
        dd.channels = cfg->channels;
        dd.i2s_number = cfg->i2s_number;
        dd.set_i2s_clk = cfg->set_i2s_clk;
    }
}

void lyrat_init()
{
    //Sample size for 20millisec data on 48KHz/16bit sampling. Division factor is (sectomillisec * bitsinbytes)
    dd.data_sample_size = ((dd.sample_rate * SAMP_BITS * SAMP_MS) / (1000 * 8));
    dd.data_buf = esp_audio_mem_calloc(1, dd.data_sample_size * (sizeof (int16_t)));
    if (dd.data_buf == NULL) {
        ESP_LOGE(TAG, "dd.data_buf allocation failed!");
        return;
    }
    dd.raw_mic_data = rb_init("raw-mic", RB_SIZE);
    if (dd.raw_mic_data == NULL) {
        ESP_LOGE(TAG, "dd.raw_mic_data rb_init failed!");
        goto lyrat_init_error_exit;
    }
    dd.resampled_mic_data = rb_init("resampled-mic", RB_SIZE);
    if (dd.resampled_mic_data == NULL) {
        ESP_LOGE(TAG, "dd.resampled_mic_data rb_init failed!");
        goto lyrat_init_error_exit;
    }
    i2s_stream_config_t i2s_cfg;
    memset(&i2s_cfg, 0, sizeof(i2s_cfg));
    i2s_cfg.i2s_num = dd.i2s_number;
    audio_board_i2s_init_default(&i2s_cfg.i2s_config);
    i2s_cfg.media_hal_cfg = media_hal_get_handle();

    dd.read_i2s_stream = i2s_reader_stream_create(&i2s_cfg);
    if (dd.read_i2s_stream) {
        ESP_LOGI(TAG, "Created I2S audio stream");
    } else {
        ESP_LOGE(TAG, "Failed creating I2S audio stream");
        goto lyrat_init_error_exit;
    }

    /* Override default stack size of 2.2K with 3K. 5K seems really unneccessary. */
    i2s_stream_set_stack_size(dd.read_i2s_stream, 3000);

    audio_io_fn_arg_t stream_reader_fn = {
        .func = dsp_write_cb,
        .arg = NULL,
    };
    audio_event_fn_arg_t stream_event_fn = {
        .func = reader_stream_event_handler,
    };
    if (audio_stream_init(&dd.read_i2s_stream->base, "i2s_reader", &stream_reader_fn, &stream_event_fn) != 0) {
        ESP_LOGE(TAG, "Failed creating audio stream");
        goto lyrat_init_error_exit;
    }
    //Initialize NN model
    if (esp_wwe_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ESP-WWE");
        return;
    }

    //Initialize sound source
    dd.item_chunk_size = esp_wwe_get_sample_chunksize() * sizeof(int16_t);

    audio_stream_start(&dd.read_i2s_stream->base);
    vTaskDelay(10/portTICK_RATE_MS);
    if (dd.set_i2s_clk) {
        audio_stream_stop(&dd.read_i2s_stream->base);
        i2s_set_clk(dd.i2s_number, dd.sample_rate, SAMP_BITS, dd.channels);
        vTaskDelay(10/portTICK_RATE_MS);
        audio_stream_start(&dd.read_i2s_stream->base);
    }
    dd.detect_wakeword = true;

    xTaskCreate(&ww_detection_task, "nn", WWE_TASK_STACK, NULL, (CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT - 1), &dd.ww_detection_task_handle);
    xTaskCreate(&resample_rb_data_task, "rb read task", RB_TASK_STACK, NULL, (CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT - 1), NULL);
    return;

lyrat_init_error_exit:
    if (dd.data_buf) {
        free(dd.data_buf);
        dd.data_buf = NULL;
    }
    if (dd.raw_mic_data) {
        rb_cleanup(dd.raw_mic_data);
        dd.raw_mic_data = NULL;
    }
    if (dd.resampled_mic_data) {
        rb_cleanup(dd.resampled_mic_data);
        dd.resampled_mic_data = NULL;
    }
    if (dd.read_i2s_stream) {
        i2s_stream_destroy(dd.read_i2s_stream);
        dd.read_i2s_stream = NULL;
    }
}
