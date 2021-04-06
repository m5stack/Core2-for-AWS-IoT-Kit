/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit
 * Factory Firmware v2.0.1
 * touch.c
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "core2forAWS.h"
#include "lvgl/lvgl.h"
#include "ft6336u.h"

#include "touch.h"

static const char* TAG = TOUCH_TAB_NAME;

// Should create a struct to pass pointers to task, but globals are easier to understand.
static uint8_t r = 0, g = 70, b = 79;
static lv_style_t bg_style;
static lv_obj_t* touch_bg;
static lv_obj_t* coordinates_label;

static void touch_task(void* pvParameters);

void display_touch_tab(lv_obj_t* tv){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);

    lv_obj_t* touch_tab = lv_tabview_add_tab(tv, TOUCH_TAB_NAME);

    /* Create the main body object and set background within the tab*/
    touch_bg = lv_obj_create(touch_tab, NULL);
    lv_obj_align(touch_bg, NULL, LV_ALIGN_IN_TOP_LEFT, 16, 36);
    lv_obj_set_size(touch_bg, 290, 190);
    lv_obj_set_click(touch_bg, false);
    lv_style_init(&bg_style);
    lv_style_set_bg_color(&bg_style, LV_STATE_DEFAULT, lv_color_make(r, g, b));
    lv_obj_add_style(touch_bg, LV_OBJ_PART_MAIN, &bg_style);

    /* Create the title within the main body object */
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
    lv_style_set_text_color(&title_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_t* tab_title_label = lv_label_create(touch_bg, NULL);
    lv_obj_add_style(tab_title_label, LV_OBJ_PART_MAIN, &title_style);
    lv_label_set_static_text(tab_title_label, "FT6336U Capacitive Touch");
    lv_obj_align(tab_title_label, touch_bg, LV_ALIGN_IN_TOP_MID, 0, 10);

    /* Create the sensor information label object */
    lv_obj_t* body_label = lv_label_create(touch_bg, NULL);
    lv_label_set_long_mode(body_label, LV_LABEL_LONG_BREAK);
    lv_label_set_static_text(body_label, "The FT6336U is a capacitive touch panel controller that provides X and Y coordinates for touch input."
        "\n\n\n\nPress the touch buttons below.");
    lv_obj_set_width(body_label, 252);
    lv_obj_align(body_label, touch_bg, LV_ALIGN_IN_TOP_LEFT, 20, 40);

    static lv_style_t body_style;
    lv_style_init(&body_style);
    lv_style_set_text_color(&body_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_add_style(body_label, LV_OBJ_PART_MAIN, &body_style);
    
    coordinates_label = lv_label_create(touch_bg, NULL);
    lv_label_set_text(coordinates_label, "X: 000,   Y: 000      Pressed: 0");
    lv_label_set_align(coordinates_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(coordinates_label, touch_bg, LV_ALIGN_CENTER, 0, 44);

    /*Create an array for the points of the line*/
    static lv_point_t line_points[] = { {20, 0}, {70, 0} };

    /*Create style*/
    static lv_style_t red_line_style;
    lv_style_init(&red_line_style);
    lv_style_set_line_width(&red_line_style, LV_STATE_DEFAULT, 6);
    lv_style_set_line_color(&red_line_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_line_rounded(&red_line_style, LV_STATE_DEFAULT, true);

    static lv_style_t green_line_style;
    lv_style_init(&green_line_style);
    lv_style_set_line_width(&green_line_style, LV_STATE_DEFAULT, 6);
    lv_style_set_line_color(&green_line_style, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_style_set_line_rounded(&green_line_style, LV_STATE_DEFAULT, true);

    static lv_style_t blue_line_style;
    lv_style_init(&blue_line_style);
    lv_style_set_line_width(&blue_line_style, LV_STATE_DEFAULT, 6);
    lv_style_set_line_color(&blue_line_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
    lv_style_set_line_rounded(&blue_line_style, LV_STATE_DEFAULT, true);

    /*Create a line and apply the new style*/
    lv_obj_t* left_line = lv_line_create(touch_tab, NULL);
    lv_line_set_points(left_line, line_points, 2);
    lv_obj_add_style(left_line, LV_LINE_PART_MAIN, &red_line_style);
    lv_obj_align(left_line, NULL, LV_ALIGN_IN_LEFT_MID, 8, 108);

    lv_obj_t* middle_line = lv_line_create(touch_tab, NULL);
    lv_line_set_points(middle_line, line_points, 2);
    lv_obj_add_style(middle_line, LV_LINE_PART_MAIN, &green_line_style);
    lv_obj_align(middle_line, NULL, LV_ALIGN_CENTER, -12, 108);
    
    lv_obj_t* right_line = lv_line_create(touch_tab, NULL);
    lv_line_set_points(right_line, line_points, 2);
    lv_obj_add_style(right_line, LV_LINE_PART_MAIN, &blue_line_style);
    lv_obj_align(right_line, NULL, LV_ALIGN_IN_RIGHT_MID, -30, 108);

    xSemaphoreGive(xGuiSemaphore);

    xTaskCreatePinnedToCore(touch_task, "touchTask", configMINIMAL_STACK_SIZE * 3, NULL, 1, &touch_handle, 1);
}

void reset_touch_bg(){
    r=0x00, g=0x00, b=0x00;
    lv_style_set_bg_color(&bg_style, LV_STATE_DEFAULT, lv_color_make(r, g, b));
    lv_obj_add_style(touch_bg, LV_OBJ_PART_MAIN, &bg_style);
}

static void touch_task(void* pvParameters){

    vTaskSuspend(NULL);

    for(;;){
        uint16_t x, y;
        bool press;

        FT6336U_GetTouch(&x, &y, &press);
        char coordinates_str[200];
        sprintf(coordinates_str, "X: %d,   Y: %d      Pressed: %d", x, y, press);

        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        lv_label_set_text(coordinates_label, coordinates_str);
        xSemaphoreGive(xGuiSemaphore);
        
        if (Button_WasPressed(button_left)) {
            r+=0x10;
            lv_style_set_bg_color(&bg_style, LV_STATE_DEFAULT, lv_color_make(r, g, b));
            lv_obj_add_style(touch_bg, LV_OBJ_PART_MAIN, &bg_style);
            ESP_LOGI(TAG, "Left Button pressed. R: %x G: %x B:%x", r, g, b);
        }
        if (Button_WasPressed(button_middle)) {
            g+=0x10;
            lv_style_set_bg_color(&bg_style, LV_STATE_DEFAULT, lv_color_make(r, g, b));
            lv_obj_add_style(touch_bg, LV_OBJ_PART_MAIN, &bg_style);
            ESP_LOGI(TAG, "Middle Button pressed. R: %x G: %x B:%x", r, g, b);
        }
        if (Button_WasPressed(button_right)) {
            b+=0x10;
            lv_style_set_bg_color(&bg_style, LV_STATE_DEFAULT, lv_color_make(r, g, b));
            lv_obj_add_style(touch_bg, LV_OBJ_PART_MAIN, &bg_style);
            ESP_LOGI(TAG, "Right Button pressed. R: %x G: %x B:%x", r, g, b);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelete(NULL); // Should never get to here...
}