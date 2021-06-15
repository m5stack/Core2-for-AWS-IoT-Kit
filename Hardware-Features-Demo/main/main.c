#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "driver/gpio.h"

#include "core2forAWS.h"

#include "atecc608_test.h"
#include "sk6812_test.h"
#include "mic_fft_test.h"

static void brightness_slider_event_cb(lv_obj_t * slider, lv_event_t event);
static void strength_slider_event_cb(lv_obj_t * slider, lv_event_t event);
static void led_event_handler(lv_obj_t * obj, lv_event_t event);

static void speakerTask(void *arg);
static void ateccTask(void *arg);
static void sdcardTest();

void app_main(void)
{
    esp_log_level_set("gpio", ESP_LOG_NONE);
    esp_log_level_set("ILI9341", ESP_LOG_NONE);

    Core2ForAWS_Init();

    sdcardTest();
    sk6812Test();

    xTaskCreatePinnedToCore(speakerTask, "speak", 4096*2, NULL, 4, NULL, 1);
    
    rtc_date_t date;
    date.year = 2020;
    date.month = 9;
    date.day = 30;

    date.hour = 13;
    date.minute = 40;
    date.second = 10;    
    BM8563_SetTime(&date);

    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);

    lv_obj_t * time_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_pos(time_label, 10, 5);
    lv_label_set_align(time_label, LV_LABEL_ALIGN_LEFT);

    lv_obj_t * mpu6886_lable = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(mpu6886_lable, time_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t * touch_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(touch_label, mpu6886_lable, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t * pmu_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(pmu_label, touch_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t * led_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(led_label, pmu_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_label_set_text(led_label, "Power LED & SK6812");
    
    lv_obj_t *sw1 = lv_switch_create(lv_scr_act(), NULL);
    lv_obj_set_size(sw1, 60, 20);
    lv_obj_align(sw1, led_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_set_event_cb(sw1, led_event_handler);
    lv_switch_on(sw1, LV_ANIM_ON);

    Core2ForAWS_LED_Enable(1);
    sk6812TaskResume();

    lv_obj_t * brightness_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(brightness_label, led_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_label_set_text(brightness_label, "Screen brightness");

    lv_obj_t * brightness_slider = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_set_width(brightness_slider, 130);
    lv_obj_align(brightness_slider, brightness_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_set_event_cb(brightness_slider, brightness_slider_event_cb);
    lv_slider_set_value(brightness_slider, 50, LV_ANIM_OFF);
    lv_slider_set_range(brightness_slider, 30, 100);

    lv_obj_t * motor_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(motor_label, brightness_label, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    lv_label_set_text(motor_label, "Motor strength");

    lv_obj_t * strength_slider = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_set_width(strength_slider, 130);
    lv_obj_align(strength_slider, motor_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_event_cb(strength_slider, strength_slider_event_cb);
    lv_slider_set_value(strength_slider, 0, LV_ANIM_OFF);
    lv_slider_set_range(strength_slider, 0, 100);

    xSemaphoreGive(xGuiSemaphore);

    xTaskCreatePinnedToCore(ateccTask, "ateccTask", 4096*2, NULL, 1, NULL, 1);

    char label_stash[200];
    for (;;) {
        BM8563_GetTime(&date);
        sprintf(label_stash, "Time: %d-%02d-%02d %02d:%02d:%02d\r\n",
                date.year, date.month, date.day, date.hour, date.minute, date.second);
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        lv_label_set_text(time_label, label_stash);
        xSemaphoreGive(xGuiSemaphore);

        float ax, ay, az;
        MPU6886_GetAccelData(&ax, &ay, &az);
        sprintf(label_stash, "MPU6886 Acc x: %.2f, y: %.2f, z: %.2f\r\n", ax, ay, az);
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        lv_label_set_text(mpu6886_lable, label_stash);
        xSemaphoreGive(xGuiSemaphore);

        uint16_t x, y;
        bool press;
        FT6336U_GetTouch(&x, &y, &press);
        sprintf(label_stash, "Touch x: %d, y: %d, press: %d\r\n", x, y, press);
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        lv_label_set_text(touch_label, label_stash);
        xSemaphoreGive(xGuiSemaphore);

        sprintf(label_stash, "Bat %.3f V, %.3f mA\r\n", Core2ForAWS_PMU_GetBatVolt(), Core2ForAWS_PMU_GetBatCurrent());
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        lv_label_set_text(pmu_label, label_stash);
        xSemaphoreGive(xGuiSemaphore);
        
        vTaskDelay(pdMS_TO_TICKS(100));

        if (Button_WasPressed(button_left)) {
            printf("button left press\r\n");
        }
        if (Button_WasReleased(button_middle)) {
            printf("button middle release\r\n");
        }
        if (Button_WasLongPress(button_right, 500)) {
            printf("button right long pressed\r\n");
        }
    }
}

static void brightness_slider_event_cb(lv_obj_t * slider, lv_event_t event) {
    if(event == LV_EVENT_VALUE_CHANGED) {
        Core2ForAWS_Display_SetBrightness(lv_slider_get_value(slider));
    }
}

static void strength_slider_event_cb(lv_obj_t * slider, lv_event_t event) {
    if(event == LV_EVENT_VALUE_CHANGED) {
        Core2ForAWS_Motor_SetStrength(lv_slider_get_value(slider));
    }
}

static void led_event_handler(lv_obj_t * obj, lv_event_t event) {
    if(event == LV_EVENT_VALUE_CHANGED) {
        Core2ForAWS_LED_Enable(lv_switch_get_state(obj));
        if (lv_switch_get_state(obj)) {
            sk6812TaskResume();
        } else {
            sk6812TaskSuspend();
        }
    }
}

static void speakerTask(void *arg) {
    Speaker_Init();
    Core2ForAWS_Speaker_Enable(1);
    extern const unsigned char music[120264];
    Speaker_WriteBuff((uint8_t *)music, 120264, portMAX_DELAY);
    Core2ForAWS_Speaker_Enable(0);
    Speaker_Deinit();
    xTaskCreatePinnedToCore(microphoneTask, "microphoneTask", 4096 * 2, NULL, 1, NULL, 0);
    vTaskDelete(NULL);
}

static void ateccTask(void *arg) {
    atecc_test();

    vTaskDelete(NULL);
}

/*
//  note: Because the SD card and the screen use the same spi bus so if want use sd card api, must
xSemaphoreTake(spi_mutex, portMAX_DELAY);
// call spi_poll to solve SPI bus sharing
spi_poll();
// call sd card api
xSemaphoreGive(spi_mutex);
*/
static void sdcardTest() {
    #define MOUNT_POINT "/sdcard"
    sdmmc_card_t* card;
    esp_err_t ret;

    ret = Core2ForAWS_SDcard_Mount(MOUNT_POINT, &card);

    if (ret != ESP_OK) {
        ESP_LOGE("sdcard", "Failed to initialize the sd card");
        return;
    } 

    ESP_LOGI("SDCARD", "Success to initialize the sd card");
    sdmmc_card_print_info(stdout, card);

    xSemaphoreTake(spi_mutex, portMAX_DELAY);
    spi_poll();
    char test_file[] =  MOUNT_POINT"/hello.txt";
    ESP_LOGI("SDCARD", "Write file to %s", test_file);
    FILE* f = fopen(test_file, "w+");
    if (f == NULL) {
        ESP_LOGE("SDCARD", "Failed to open file for writing");
        xSemaphoreGive(spi_mutex);
        return;
    }
    ESP_LOGI("SDCARD", "Write -> Hello %s!\n", "SD Card");
    fprintf(f, "Hello %s!\r\n", "SD Card");
    fclose(f);

    ESP_LOGI("SDCARD", "Reading file %s", test_file);
    f = fopen(test_file, "r");
    if (f == NULL) {
        ESP_LOGE("SDCARD", "Failed to open file for reading");
        xSemaphoreGive(spi_mutex);
        return;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    ESP_LOGI("SDCARD", "Read <- %s", line);
    xSemaphoreGive(spi_mutex);
}