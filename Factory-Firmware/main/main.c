/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit Factory Firmware v1.0.0
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_freertos_hooks.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"

#include "core2forAWS.h"
#include "lvgl/lvgl.h"
#include "ft6336u.h"
#include "speaker.h"
#include "bm8563.h"
#include "mpu6886.h"
#include "axp192.h"
#include "cryptoauthlib.h"
#include "ledBarAnimation.h"
#include "i2c_device.h"
#include "powered_by_aws_logo.c"
#include "core2forAWS_qr_code.c"
#include "music.c"
#include "atecc608.h"

static void speakerTask(void *arg);
static const char *TAG = "Core2ForAWS";
extern SemaphoreHandle_t xGuiSemaphore;
extern SemaphoreHandle_t spi_mutex;

extern void spi_poll();

void app_main(void)
{
    ESP_LOGI(TAG, "Firmware Version: %s", FIRMWARE_VERSION);

    esp_log_level_set("gpio", ESP_LOG_NONE);
    esp_log_level_set("ILI9341", ESP_LOG_NONE);

    spi_mutex = xSemaphoreCreateMutex();

    Core2ForAWS_Init();
    FT6336U_Init();
    Core2ForAWS_LCD_Init();
    Core2ForAWS_Button_Init();
    Core2ForAWS_Sk6812_Init();
    Core2ForAWS_LCD_SetBrightness(80);
    Atecc608_Init();

    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    lv_obj_t * opener_scr = lv_scr_act();
    lv_obj_t * aws_img_src = lv_img_create(opener_scr, NULL);
    lv_img_set_src(aws_img_src, &powered_by_aws_logo);
    // lv_obj_set_pos(img_src, 10, 10);      /* Set the positions */
    lv_obj_align(aws_img_src, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(opener_scr, LV_OBJ_PART_MAIN, 0, LV_COLOR_WHITE);
    xSemaphoreGive(xGuiSemaphore);

    char * device_serial = malloc(ATCA_SERIAL_NUM_SIZE * 2 + 1);
    i2c_take_port(ATECC608_I2C_PORT, portMAX_DELAY);
    ATCA_STATUS ret = Atecc608_GetSerialString(device_serial);
    i2c_free_port(ATECC608_I2C_PORT);
    if (ret == ATCA_SUCCESS){
        ESP_LOGI(TAG,"**************************************\n");
        ESP_LOGI(TAG, "Device Serial: %s\n", device_serial);
        ESP_LOGI(TAG,"**************************************\n");
        free(device_serial);
    }    

    vTaskDelay(1500 / portTICK_PERIOD_MS);

    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    lv_obj_clean(opener_scr);
    lv_obj_t * core2forAWS_scr = lv_obj_create(NULL, NULL);
    lv_obj_t * qr_img_src = lv_img_create(core2forAWS_scr, NULL); /* Crate an image object */
    lv_img_set_src(qr_img_src, &core2forAWS_qr_code);  /* Set the created file as image (a red flower) */
    lv_obj_align(qr_img_src, NULL, LV_ALIGN_CENTER, 0, -10);
    lv_obj_t * url_label = lv_label_create(core2forAWS_scr, NULL);
    lv_label_set_static_text(url_label, "View AWS IoT EduKit content at:\nhttps://edukit.workshop.aws");
    lv_label_set_align(url_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(url_label, qr_img_src, LV_ALIGN_OUT_BOTTOM_MID, 0 , 10);
    lv_obj_set_style_local_bg_color(core2forAWS_scr, LV_OBJ_PART_MAIN, 0, LV_COLOR_WHITE);
    // lv_obj_set_pos(url_label, 10, 5);
    lv_scr_load_anim(core2forAWS_scr, LV_SCR_LOAD_ANIM_MOVE_LEFT, 400, 0, false);
    xSemaphoreGive(xGuiSemaphore);
    
    xTaskCreatePinnedToCore(speakerTask, "speak", 4096*2, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(sk6812ShowTask, "sk6812ShowTask", 4096*2, NULL, 1, NULL, 1);
}

static void speakerTask(void *arg) {
    Speaker_Init();
    Core2ForAWS_Speaker_Enable(1);
    extern const unsigned char music[120264];
    Speaker_WriteBuff((uint8_t *)music, 120264, portMAX_DELAY);
    Core2ForAWS_Speaker_Enable(0);
    Speaker_Deinit();
    vTaskDelete(NULL);
}