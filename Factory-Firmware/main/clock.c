/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit
 * Factory Firmware v2.0.3
 * clock.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "core2forAWS.h"

#include "clock.h"

static const char* TAG = CLOCK_TAB_NAME;

lv_obj_t* clock_tab;

static lv_obj_t* hour_roller;
static lv_obj_t* minute_roller;

static void hour_event_handler(lv_obj_t* obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        int hour = lv_roller_get_selected(obj);
        datetime.hour = hour;
        BM8563_SetTime(&datetime);
        update_roller_time();
    }
}

static void minute_event_handler(lv_obj_t* obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        int minute = lv_roller_get_selected(obj);
        datetime.minute = minute;
        BM8563_SetTime(&datetime);
        update_roller_time();
    }
}

void update_roller_time(){
    BM8563_GetTime(&datetime);
    lv_roller_set_selected(hour_roller, datetime.hour, LV_ANIM_OFF);
    lv_roller_set_selected(minute_roller, datetime.minute, LV_ANIM_OFF);

    ESP_LOGI(TAG, "Current Date: %d-%02d-%02d  Time: %02d:%02d:%02d", 
        datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second);
}

/*
Counts the number of digits using binary search. Not as elegant as recursive, but it's much faster.
*/
static uint16_t count_bsearch(int i)
{
    if (i < 0)
    {
        if (i == INT_MIN)
            return 10; // special case for -2^31 because 2^31 can't fit in a two's complement 32-bit integer
        i = -i;
    }
    if (i < 100000) {
        if (i < 1000) {
            if (i < 10) return 1;
            else if (i < 100) return 2;
            else return 3;
        } else {
            if (i < 10000) return 4;
            else return 5;
        }
    } else {
        if (i < 10000000) {
            if (i < 1000000) return 6;
            else return 7;
        } else {
            if (i < 100000000) return 8;
            else if (i < 1000000000) return 9;
            else return 10;
        }
    }
}

static char* generate_roller_str(int number)
{
    uint16_t temp_number = number;
    const uint16_t last_number = number - 1;
    const uint16_t last_number_digits = count_bsearch(last_number);
    size_t roller_str_len = 0;
    
    for(int i = last_number_digits - 1; i >= 0; i--){
        size_t number_cutoff = pow(10, i) - 1;
        roller_str_len += (i + 2) * (temp_number - number_cutoff);
        temp_number = number_cutoff;
    }
    roller_str_len--;
    
    char* roller_str = heap_caps_malloc(roller_str_len, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM);
    roller_str[0] = '\0';
    for(int i = 0; i < number; i++){
        size_t i_size = count_bsearch(i) + 2;
        char message[i_size];
        snprintf(message, i_size, "%d\n", i);
        strncat(roller_str, message, roller_str_len - strlen(roller_str) - 1);
    }
    return roller_str;
}

void display_clock_tab(lv_obj_t*tv, lv_obj_t* core2forAWS_screen_obj){
    clock_tab = lv_tabview_add_tab(tv, CLOCK_TAB_NAME);  // Create a tab

    /* Create the main body object and set background within the tab*/
    static lv_style_t bg_style;
    lv_obj_t* clock_bg = lv_obj_create(clock_tab, NULL);
    lv_obj_align(clock_bg, NULL, LV_ALIGN_IN_TOP_LEFT, 16, 36);
    lv_obj_set_size(clock_bg, 290, 190);
    lv_obj_set_click(clock_bg, false);
    lv_style_init(&bg_style);
    lv_style_set_bg_color(&bg_style, LV_STATE_DEFAULT, lv_color_make(254, 230, 0));
    lv_obj_add_style(clock_bg, LV_OBJ_PART_MAIN, &bg_style);

    /* Create the title within the main body object */
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
    lv_style_set_text_color(&title_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_t* tab_title_label = lv_label_create(clock_bg, NULL);
    lv_obj_add_style(tab_title_label, LV_OBJ_PART_MAIN, &title_style);
    lv_label_set_static_text(tab_title_label, "BM8563 Real-time Clock");
    lv_obj_align(tab_title_label, clock_bg, LV_ALIGN_IN_TOP_MID, 0, 10);

    /* Create the sensor information label object */
    lv_obj_t* body_label = lv_label_create(clock_bg, NULL);
    lv_label_set_long_mode(body_label, LV_LABEL_LONG_BREAK);
    lv_label_set_static_text(body_label, "The BM8563 is an accurate, low powered real-time clock. ▲");
    lv_obj_set_width(body_label, 252);
    lv_obj_align(body_label, clock_bg, LV_ALIGN_IN_TOP_LEFT, 20, 40);

    static lv_style_t body_style;
    lv_style_init(&body_style);
    lv_style_set_text_color(&body_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_add_style(body_label, LV_OBJ_PART_MAIN, &body_style);

    char* hours_str = generate_roller_str(24);
    hour_roller = lv_roller_create(clock_bg, NULL);
    lv_roller_set_options(hour_roller, hours_str, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(hour_roller, 2);
    lv_roller_set_auto_fit(hour_roller, false);
    lv_obj_set_width(hour_roller, 60);
    lv_obj_align(hour_roller, clock_bg, LV_ALIGN_IN_BOTTOM_MID, -40, -20);
    heap_caps_free(hours_str);

    lv_obj_t* separator_label = lv_label_create(clock_bg, NULL);
    lv_label_set_static_text(separator_label, ":");
    lv_obj_set_width(separator_label, 4);
    lv_obj_align(separator_label, clock_bg, LV_ALIGN_IN_BOTTOM_MID, 0, -50);

    char *minutes_str = generate_roller_str(60);
    minute_roller = lv_roller_create(clock_bg, hour_roller);
    lv_roller_set_options(minute_roller, minutes_str, LV_ROLLER_MODE_NORMAL);
    // lv_roller_set_visible_row_count(minute_roller, 2);
    lv_obj_align(minute_roller, clock_bg, LV_ALIGN_IN_BOTTOM_MID, 40, -20);
    heap_caps_free(minutes_str);

    lv_obj_set_event_cb(hour_roller, hour_event_handler);
    lv_obj_set_event_cb(minute_roller, minute_event_handler);

    xTaskCreatePinnedToCore(clock_task, "clockTask", configMINIMAL_STACK_SIZE * 3, (void*) core2forAWS_screen_obj, 0, &clock_handle, 1);
}

void clock_task(void* pvParameters){
    lv_obj_t* time_label = lv_label_create((lv_obj_t*)pvParameters, NULL);
    lv_label_set_text(time_label, "00:00:00 AM");
    lv_label_set_align(time_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(time_label, NULL, LV_ALIGN_IN_TOP_MID, 4, 10);

    for(;;){
        BM8563_GetTime(&datetime);
        char clock_buf[15];
        if((uint16_t) datetime.hour >= 12){
            if((uint16_t) datetime.hour > 12) snprintf(clock_buf, 15, "%02d:%02d:%02d PM", datetime.hour-12, datetime.minute, datetime.second);
            else snprintf(clock_buf, 15, "%02d:%02d:%02d PM", datetime.hour, datetime.minute, datetime.second);
        } else{
            snprintf(clock_buf, 15, "%02d:%02d:%02d AM", datetime.hour, datetime.minute, datetime.second);
        }
        lv_label_set_text(time_label, clock_buf);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL); // Should never get to here...
}