// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <esp_log.h>
#include <media_hal.h>
#include <voice_assistant.h>
#include <esp_audio_mem.h>
#include <va_button.h>
#include <va_nvs_utils.h>
#include <va_dsp.h>
#include <m5stackcore2_init.h>
#include "audio_board.h"

#define AUDIO_BUF_SIZE (320)

#define EVENTQ_LENGTH   10
#define STACK_SIZE      6 * 1024
#define DSP_NVS_KEY "dsp_mute"

static const char *TAG = "[va_dsp]";

enum va_dsp_state {
    STREAMING,
    STOPPED,
    MUTED,
};

static int8_t dsp_mute_en;
static int8_t dsp_mic_enabled = 0;

static struct va_dsp_data_t {
    va_dsp_record_cb_t va_dsp_record_cb;
    va_dsp_recognize_cb_t va_dsp_recognize_cb;
    enum va_dsp_state dsp_state;
    QueueHandle_t cmd_queue;
    uint8_t audio_buf[AUDIO_BUF_SIZE];
    bool va_dsp_booted;
} va_dsp_data = {
    .va_dsp_record_cb = NULL,
    .va_dsp_recognize_cb = NULL,
    .va_dsp_booted = false,
};

static inline void _va_dsp_stop_streaming()
{
    va_dsp_data.dsp_state = STOPPED;
    m5stackcore2_stop_capture();
}

static inline void _va_dsp_start_streaming()
{
    va_dsp_data.dsp_state = STREAMING;
    m5stackcore2_start_capture();
}

static inline int _va_dsp_stream_audio(uint8_t *buffer, int size, int wait)
{
    return m5stackcore2_stream_audio(buffer, size, wait);
}

static inline void _va_dsp_mute_mic()
{
    va_dsp_data.dsp_state = MUTED;
    if (va_dsp_data.dsp_state == STREAMING) {
        m5stackcore2_stop_capture();
    }
    m5stackcore2_mic_mute();
}

static inline void _va_dsp_unmute_mic()
{
    va_dsp_data.dsp_state = STOPPED;
    m5stackcore2_mic_unmute();
}

static void va_dsp_thread(void *arg)
{
    struct dsp_event_data event_data;
    while(1) {
        xQueueReceive(va_dsp_data.cmd_queue, &event_data, portMAX_DELAY);
        switch (va_dsp_data.dsp_state) {
            case STREAMING:
                switch (event_data.event) {
                    case TAP_TO_TALK:
                        /* Stop the streaming */
                        _va_dsp_stop_streaming();
                        break;
                    case GET_AUDIO: {
                        int read_len = _va_dsp_stream_audio(va_dsp_data.audio_buf, AUDIO_BUF_SIZE, portMAX_DELAY);
                        if (read_len > 0) {
                            va_dsp_data.va_dsp_record_cb(va_dsp_data.audio_buf, read_len);
                            struct dsp_event_data new_event = {
                                .event = GET_AUDIO
                            };
                            xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
                        } else {
                            _va_dsp_stop_streaming();
                        }
                        break;
                    }
                    case STOP_MIC:
                        xSemaphoreTake(mic_state, portMAX_DELAY);
                        dsp_mic_enabled = 0;
                        xSemaphoreGive(mic_state);

                        ESP_LOGI(TAG, "Event %d STOP_MIC received", event_data.event);
                        _va_dsp_stop_streaming();
                        break;
                    case MUTE:
                        _va_dsp_mute_mic();
                        break;
                    case WW:
                    case START_MIC:
                    case UNMUTE:
                    default:
                        ESP_LOGI(TAG, "Event %d unsupported in STREAMING state", event_data.event);
                        break;
                }
                break;
            case STOPPED:
                switch (event_data.event) {
                    case WW: {
                        size_t phrase_length = 0;
                        if (phrase_length == 0) {
                            /*XXX: Should we close the stream here?*/
                            break;
                        }
                        if (va_dsp_data.va_dsp_recognize_cb(phrase_length, WAKEWORD) == 0) {
                            struct dsp_event_data new_event = {
                                .event = GET_AUDIO
                            };
                            xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
                            va_dsp_data.dsp_state = STREAMING;
                        } else {
                            ESP_LOGI(TAG, "Error starting a new dialog..stopping capture");
                            _va_dsp_stop_streaming();
                        }
                        break;
                    }
                    case TAP_TO_TALK:
                        ESP_LOGI(TAG, "Enabling Mic for Push-to-Talk Button %d", __LINE__);

                        xSemaphoreTake(mic_state, portMAX_DELAY);
                        dsp_mic_enabled = 1;
                        audio_board_i2s_set_spk_mic_mode(MODE_MIC);
                        xSemaphoreGive(mic_state);

                        if (va_dsp_data.va_dsp_recognize_cb(0, TAP) == 0) {
                            _va_dsp_start_streaming();
                            struct dsp_event_data new_event = {
                                .event = GET_AUDIO
                            };
                            xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
                        }
                        break;
                    case START_MIC:
                        ESP_LOGI(TAG, "Enabling Mic for multi-turn conversation");
                        xSemaphoreTake(mic_state, portMAX_DELAY);
                        dsp_mic_enabled = 1;
                        audio_board_i2s_set_spk_mic_mode(MODE_MIC);
                        xSemaphoreGive(mic_state);

                        _va_dsp_start_streaming();
                        struct dsp_event_data new_event = {
                            .event = GET_AUDIO
                        };
                        xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
                        break;
                    case MUTE:
                        _va_dsp_mute_mic();
                        break;
                    case GET_AUDIO:
                    case STOP_MIC:
                    case UNMUTE:
                    default:
                        ESP_LOGI(TAG, "Event %d unsupported in STOPPED state", event_data.event);
                        break;
                }
                break;
            case MUTED:
                switch (event_data.event) {
                    case UNMUTE:
                        _va_dsp_unmute_mic();
                        break;
                    case WW:
                    case TAP_TO_TALK:
                    case GET_AUDIO:
                    case START_MIC:
                    case STOP_MIC:
                    case MUTE:
                    default:
                        ESP_LOGI(TAG, "Event %d unsupported in MUTE state", event_data.event);
                        break;
                }
                break;

            default:
                ESP_LOGI(TAG, "Unknown state %d with Event %d", va_dsp_data.dsp_state, event_data.event);
                break;
        }
    }
}

int va_app_speech_stop()
{
    ESP_LOGI(TAG, "Sending STOP_MIC command");
    struct dsp_event_data new_event = {
        .event = STOP_MIC
    };
    xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
    return 0;
}

int va_app_speech_start()
{
    ESP_LOGI(TAG, "Sending START_MIC command");
    struct dsp_event_data new_event = {
        .event = START_MIC
    };
    xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
    return 0;
}

int va_dsp_tap_to_talk_start()
{
    if (va_dsp_data.va_dsp_booted == false) {
        return -1;
    }
    printf("%s: Sending start for tap to talk command\n", TAG);
    struct dsp_event_data new_event = {
        .event = TAP_TO_TALK
    };
    xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
    return ESP_OK;
}

int va_dsp_playback_starting()
{
    xSemaphoreTake(mic_state, portMAX_DELAY);
    if(!dsp_mic_enabled && (i2s_mode != MODE_SPK)) 
    { 
        audio_board_i2s_set_spk_mic_mode(MODE_SPK);
    }
    xSemaphoreGive(mic_state);
    return 0;
}

void va_dsp_reset()
{
    if (va_dsp_data.va_dsp_booted == true) {
        struct dsp_event_data new_event;
        new_event.event = MUTE;
        xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
    }
}

void va_dsp_mic_mute(bool mute)
{
    struct dsp_event_data new_event;
    if (mute)
        new_event.event = MUTE;
    else
        new_event.event = UNMUTE;
    va_nvs_set_i8(DSP_NVS_KEY, mute);
    xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
}

void va_dsp_init(va_dsp_recognize_cb_t va_dsp_recognize_cb, va_dsp_record_cb_t va_dsp_record_cb)
{
    va_dsp_data.va_dsp_record_cb = va_dsp_record_cb;
    va_dsp_data.va_dsp_recognize_cb = va_dsp_recognize_cb;

    m5stackcore2_init();

    TaskHandle_t xHandle = NULL;
    StackType_t *task_stack = (StackType_t *) heap_caps_calloc(1, STACK_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    static StaticTask_t task_buf;

    va_dsp_data.cmd_queue = xQueueCreate(10, sizeof(struct dsp_event_data));
    if (!va_dsp_data.cmd_queue) {
        ESP_LOGE(TAG, "Error creating va_dsp queue");
        return;
    }

    va_dsp_data.dsp_state = STOPPED;
    if (va_nvs_get_i8(DSP_NVS_KEY, &dsp_mute_en) == ESP_OK) {
        if (dsp_mute_en) {
            //va_dsp_mic_mute(dsp_mute_en);
            //va_button_notify_mute(dsp_mute_en);
        }
    }

    xHandle = xTaskCreateStatic(va_dsp_thread, "dspg-thread", STACK_SIZE,
                                NULL, CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, task_stack, &task_buf);
    if (xHandle == NULL) {
        ESP_LOGE(TAG, "Couldn't create thead");
    }
    //Notify Alexa layer that DSP initialization is finished
    va_boot_dsp_signal();
    va_dsp_data.va_dsp_booted = true;
    //_va_dsp_unmute_mic();
}
