/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit
 * Factory Firmware v2.1.0
 * mic.c
 * 
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

#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "driver/i2s.h"

#include "core2forAWS.h"

#include "mic.h"
#include "fft.h"
#include "color_map.c"

#define CANVAS_WIDTH 240
#define CANVAS_HEIGHT 60

static const char *TAG = MICROPHONE_TAB_NAME;

static long map(long x, long in_min, long in_max, long out_min, long out_max) {
    long divisor = (in_max - in_min);
    if(divisor == 0){
        return -1;
    }
    return (x - in_min) * (out_max - out_min) / divisor + out_min;
}

void display_microphone_tab(lv_obj_t* tv){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);   // Takes (blocks) the xGuiSemaphore mutex from being read/written by another task.

    lv_obj_t* mic_tab = lv_tabview_add_tab(tv, MICROPHONE_TAB_NAME);  // Create a tab

    /* Create the main body object and set background within the tab*/
    static lv_style_t bg_style;
    lv_obj_t* mic_bg = lv_obj_create(mic_tab, NULL);
    lv_obj_align(mic_bg, NULL, LV_ALIGN_IN_TOP_LEFT, 16, 36);
    lv_obj_set_size(mic_bg, 290, 190);
    lv_obj_set_click(mic_bg, false);
    lv_style_init(&bg_style);
    lv_style_set_bg_color(&bg_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_add_style(mic_bg, LV_OBJ_PART_MAIN, &bg_style);

    /* Create the title within the main body object */
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
    lv_style_set_text_color(&title_style, LV_STATE_DEFAULT, LV_COLOR_LIME);
    lv_obj_t* tab_title_label = lv_label_create(mic_bg, NULL);
    lv_obj_add_style(tab_title_label, LV_OBJ_PART_MAIN, &title_style);
    lv_label_set_static_text(tab_title_label, "SPM1423 Microphone");
    lv_obj_align(tab_title_label, mic_bg, LV_ALIGN_IN_TOP_MID, 0, 10);

    /* Create the sensor information label object */
    lv_obj_t* body_label = lv_label_create(mic_bg, NULL);
    lv_label_set_long_mode(body_label, LV_LABEL_LONG_BREAK);
    lv_label_set_static_text(body_label, "The SPM1423 is an enhanced far-field MEMS microphone.\n\nSay \"Hi EduKit\"");
    lv_obj_set_width(body_label, 252);
    lv_obj_align(body_label, mic_bg, LV_ALIGN_IN_TOP_LEFT, 20, 40);

    static lv_style_t body_style;
    lv_style_init(&body_style);
    lv_style_set_text_color(&body_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_add_style(body_label, LV_OBJ_PART_MAIN, &body_style);

    xSemaphoreGive(xGuiSemaphore);
    
    xTaskCreatePinnedToCore(fft_show_task, "fftShowTask", 4096 * 2, (void*)mic_tab, 1, &FFT_handle, 1);
}

void microphoneTask(void* pvParameters) {
    vTaskSuspend(NULL);

    static int8_t i2s_readraw_buff[1024];
    size_t bytesread;
    int16_t* buffptr;
    double data = 0;
    uint8_t* fft_dis_buff = NULL;
    Microphone_Init();
    QueueHandle_t queue = (QueueHandle_t) pvParameters;

    for (;;) {
        fft_dis_buff = (uint8_t*)heap_caps_malloc(CANVAS_HEIGHT * sizeof(uint8_t), MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM);
        memset(fft_dis_buff, 0, CANVAS_HEIGHT);
        fft_config_t* real_fft_plan = fft_init(512, FFT_REAL, FFT_FORWARD, NULL, NULL);
        i2s_read(I2S_NUM_0, (char*)i2s_readraw_buff, 1024, &bytesread, pdMS_TO_TICKS(100));
        buffptr = (int16_t*)i2s_readraw_buff;
        for (uint16_t count_n = 0; count_n < real_fft_plan->size; count_n++) {
            real_fft_plan->input[count_n] = (float)map(buffptr[count_n], INT16_MIN, INT16_MAX, -1000, 1000);
        }
        fft_execute(real_fft_plan);

        for (uint16_t count_n = 1; count_n < CANVAS_HEIGHT; count_n++) {
            data = sqrt(real_fft_plan->output[2 * count_n] * real_fft_plan->output[2 * count_n] + real_fft_plan->output[2 * count_n + 1] * real_fft_plan->output[2 * count_n + 1]);
            fft_dis_buff[CANVAS_HEIGHT - count_n]  = map(data, 0, 2000, 0, 256);
        }
        fft_destroy(real_fft_plan);
        if(xQueueSend(queue, &fft_dis_buff, 0) != pdPASS) {
            free(fft_dis_buff);
        }
    }
    vTaskDelete(NULL); // Should never get to here...
}

void fft_show_task(void* pvParameters) {    
    QueueHandle_t mic_queue = xQueueCreate(2, sizeof(uint8_t*));
    xTaskCreatePinnedToCore(microphoneTask, "microphoneTask", 4096 * 2, (void*) mic_queue, 1, &mic_handle, 1);
    
    vTaskSuspend(NULL);
    static uint16_t position_data = 0;
    uint16_t color_position;
    uint8_t* fft_dis_buff = heap_caps_malloc(sizeof (uint8_t) * CANVAS_HEIGHT, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM);
    
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    lv_obj_t* canvas = lv_canvas_create((lv_obj_t*)pvParameters, NULL);
    lv_color_t* cbuf = heap_caps_malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT), MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM);
    lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_fill_bg(canvas, LV_COLOR_BLACK, LV_OPA_COVER);
    lv_obj_align(canvas, (lv_obj_t*)pvParameters, LV_ALIGN_IN_BOTTOM_MID, 0, -18);
    xSemaphoreGive(xGuiSemaphore);
    
    for (;;) {
        if(mic_queue != NULL){
            xQueueReceive(mic_queue, &fft_dis_buff, 0);
            for (uint16_t count_y = 0; count_y < CANVAS_HEIGHT; count_y++) {
                color_position = fft_dis_buff[count_y];
                xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
                lv_canvas_set_px(canvas, position_data, count_y, LV_COLOR_MAKE(color_map[color_position * 3 + 0], color_map[color_position * 3 + 1], color_map[color_position * 3 + 2]));
                xSemaphoreGive(xGuiSemaphore);
            }
            position_data ++;
            if (position_data == CANVAS_WIDTH) {
                position_data = 0;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}