// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/timers.h>
#include <esp_log.h>

#include <voice_assistant.h>
#include <esp_audio_mem.h>
#include <va_nvs_utils.h>
#include <media_hal.h>

#include "va_dsp.h"
#include "va_button.h"
#include "hollow_dsp_init.h"
#include "esp_pm.h"
#include "driver/i2s.h"

#define EVENTQ_LENGTH   10
#define STACK_SIZE      6 * 1024
#define DSP_NVS_KEY "dsp_mute"

static const char *TAG = "[va_dsp]";

static int8_t dsp_mute_en;

#ifdef CONFIG_PM_ENABLE
#define LOW_POWER_TIMER_SECS 10  //Custom DSP will be put into low power mode if no audio playback has occurred in last LOW_POWER_TIMER_SECS
static bool is_first_playback;
static bool do_power_switch;
static TimerHandle_t playback_timer;
static esp_pm_lock_handle_t playback_pm;
#endif

enum va_dsp_state {
    STREAMING,
    STOPPED,
    MUTED,
};

static struct va_dsp_data_t {
    va_dsp_record_cb_t va_dsp_record_cb;
    va_dsp_recognize_cb_t va_dsp_recognize_cb;
    enum va_dsp_state dsp_state;
    QueueHandle_t cmd_queue;
    TaskHandle_t xHandle;
    uint8_t *audio_buf;
    bool va_dsp_booted;
} va_dsp_data = {
    .va_dsp_record_cb = NULL,
    .va_dsp_recognize_cb = NULL,
    .xHandle = NULL,
    .va_dsp_booted = false,
};


static inline void _enable_power_switch()
{
#ifdef CONFIG_PM_ENABLE
    do_power_switch = true;
#endif
}

static inline void _disable_power_switch()
{
#ifdef CONFIG_PM_ENABLE
    do_power_switch = false;
#endif
}

static inline void _va_dsp_stop_streaming()
{
    custom_dsp_stop_capture();
    va_dsp_data.dsp_state = STOPPED;
}

static inline void _va_dsp_start_streaming()
{
    custom_dsp_start_capture();
    va_dsp_data.dsp_state = STREAMING;
}

static inline void _va_dsp_mute_mic()
{
    if (va_dsp_data.dsp_state == STREAMING) {
        custom_dsp_stop_capture();
    }
    custom_dsp_mic_mute();
    _disable_power_switch();
    va_dsp_data.dsp_state = MUTED;
}

static inline void _va_dsp_unmute_mic()
{
    custom_dsp_mic_unmute();
    va_dsp_data.dsp_state = STOPPED;
}

static inline void _va_dsp_low_power()
{
#ifdef CONFIG_PM_ENABLE
    ESP_LOGI(TAG, "Entering DSP Low Power");
    if (va_dsp_data.dsp_state != MUTED) {
        custom_dsp_enter_low_power();
    }
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_GPIO0);
    gpio_set_level(GPIO_NUM_0, 0);
    _enable_power_switch();
    is_first_playback = true;
    esp_pm_lock_release(playback_pm);
#else
    ESP_LOGI(TAG, "Error: Low Power Mode disabled");
#endif
}

#ifdef CONFIG_PM_ENABLE
static void timer_cb(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG, "One Shot Timer Initiated");
    struct dsp_event_data new_event = {
        .event = POWER_SAVE
    };
    xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
}
#endif

static void va_dsp_thread(void *arg)
{
    struct dsp_event_data event_data;
    //uint32_t timeout = portMAX_DELAY;
    while(1) {
        xQueueReceive(va_dsp_data.cmd_queue, &event_data, portMAX_DELAY);
        //timeout = 0;
        switch (va_dsp_data.dsp_state) {
            case STREAMING:
                switch (event_data.event) {
                    case TAP_TO_TALK:
                        /* Stop the streaming */
                        _va_dsp_stop_streaming();
                        break;
                    case GET_AUDIO: {
                        int read_len = custom_dsp_stream_audio(va_dsp_data.audio_buf, AUDIO_BUF_SIZE);
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
                        _va_dsp_stop_streaming();
                        break;
                    case MUTE:
                        _va_dsp_mute_mic();
                        break;
                    case POWER_SAVE:
                        va_dsp_playback_starting();
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
                        size_t phrase_length = custom_dsp_get_ww_len();
                        if (phrase_length == 0) {
                            /*XXX: Should we close the stream here?*/
                            break;
                        }
                        _disable_power_switch();
                        if (va_dsp_data.va_dsp_recognize_cb(phrase_length, WAKEWORD) == 0) {
                            struct dsp_event_data new_event = {
                                .event = GET_AUDIO
                            };
                            xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
                            va_dsp_data.dsp_state = STREAMING;
                        } else {
                            ESP_LOGI(TAG, "Error starting a new dialog..stopping capture");
                            custom_dsp_stop_capture();
                            _enable_power_switch();
                        }
                        break;
                    }
                    case TAP_TO_TALK:
                        _disable_power_switch();
                        if (va_dsp_data.va_dsp_recognize_cb(0, TAP) == 0) {
                            _va_dsp_start_streaming();
                            struct dsp_event_data new_event = {
                                .event = GET_AUDIO
                            };
                            xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
                        } else {
                            ESP_LOGI(TAG, "Error starting a new dialog");
                            _enable_power_switch();
                        }
                        break;
                    case START_MIC:
                        _va_dsp_start_streaming();
                        struct dsp_event_data new_event = {
                            .event = GET_AUDIO
                        };
                        xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
                        break;
                    case MUTE:
                        _va_dsp_mute_mic();
                        break;
                    case POWER_SAVE:
                        _va_dsp_low_power();
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
                    case POWER_SAVE:
                        _va_dsp_low_power();
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
    ESP_LOGI(TAG, "Sending stop command");
    struct dsp_event_data new_event = {
        .event = STOP_MIC
    };
    xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
    return 0;
}

int va_app_speech_start()
{
    ESP_LOGI(TAG, "Sending start speech command");
    struct dsp_event_data new_event = {
        .event = START_MIC
    };
    xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
    return 0;
}

int va_dsp_playback_starting()
{
#ifdef CONFIG_PM_ENABLE
    if (is_first_playback) {
        esp_pm_lock_acquire(playback_pm);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
        is_first_playback = false;
    }

    if (do_power_switch) {
        custom_dsp_aec_init();
        _disable_power_switch();
    }

    if (!playback_timer) {
        ESP_LOGE(TAG, "Playback timer not created");
        return -1;
    }

    xTimerReset(playback_timer, 0);
#endif
    return 0;
}

int va_dsp_tap_to_talk_start()
{
    ESP_LOGI(TAG, "Sending start for tap to talk command");
    struct dsp_event_data new_event = {
        .event = TAP_TO_TALK
    };
    xQueueSend(va_dsp_data.cmd_queue, &new_event, portMAX_DELAY);
    return ESP_OK;
}

void va_dsp_reset()
{
    if (va_dsp_data.va_dsp_booted == true) {
        custom_dsp_reset();
    }
}

void va_dsp_mic_mute(bool mute)
{
#ifdef CONFIG_PM_ENABLE
    if (is_first_playback) {
        esp_pm_lock_acquire(playback_pm);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
        _disable_power_switch();
        is_first_playback = false;
    }
#endif

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

    StackType_t *task_stack = (StackType_t *)heap_caps_calloc(1, STACK_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    static StaticTask_t task_buf;

    va_dsp_data.cmd_queue = xQueueCreate(10, sizeof(struct dsp_event_data));
    if (!va_dsp_data.cmd_queue) {
        ESP_LOGE(TAG, "Error creating va_dsp queue");
        return;
    }

    va_dsp_data.audio_buf = heap_caps_calloc(1, AUDIO_BUF_SIZE, MALLOC_CAP_DMA);
    if (!va_dsp_data.audio_buf) {
        ESP_LOGE(TAG, "Error allocating audio buffer in DMA region");
        return;
    }

    custom_dsp_init(va_dsp_data.cmd_queue);

    //vTaskDelay(2000/portTICK_RATE_MS);
#ifdef CONFIG_PM_ENABLE
    esp_pm_lock_create(ESP_PM_APB_FREQ_MAX, 0, "Playback Lock", &playback_pm);
    playback_timer = xTimerCreate("playback_timer", LOW_POWER_TIMER_SECS * 1000 / portTICK_RATE_MS, pdFALSE, NULL, timer_cb);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_GPIO0);
    gpio_set_level(GPIO_NUM_0, 0);
    _enable_power_switch();
    is_first_playback = true;
#else
    custom_dsp_aec_init();
#endif
    va_dsp_data.dsp_state = STOPPED;
    //Check if device was Mute or Unmute before power off / reboot and set the last state device was in
    if (va_nvs_get_i8(DSP_NVS_KEY, &dsp_mute_en) == ESP_OK) {
        if (dsp_mute_en) {
            va_dsp_mic_mute(dsp_mute_en);
            va_button_notify_mute(dsp_mute_en);
        }
    }

    va_dsp_data.xHandle = xTaskCreateStatic(va_dsp_thread, "dsp-thread", STACK_SIZE,
                                NULL, CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, task_stack, &task_buf);
    if (va_dsp_data.xHandle == NULL) {
        ESP_LOGE(TAG, "Couldn't create thead");
    }
    //Notify Alexa layer that DSP initialization is finished
    va_boot_dsp_signal();
    va_dsp_data.va_dsp_booted = true;
}
