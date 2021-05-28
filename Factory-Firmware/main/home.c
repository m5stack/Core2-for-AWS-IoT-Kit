/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit
 * Factory Firmware v2.0.2
 * home.c
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

#include "home.h"

static const char* TAG = HOME_TAB_NAME;

void display_home_tab(lv_obj_t* tv){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);   // Takes (blocks) the xGuiSemaphore mutex from being read/written by another task.
    
    lv_obj_t* home_tab = lv_tabview_add_tab(tv, HOME_TAB_NAME);   // Create a tab

    /* Create the title within the tab */
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
    lv_style_set_text_color(&title_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    
    lv_obj_t* tab_title_label = lv_label_create(home_tab, NULL);
    lv_obj_add_style(tab_title_label, LV_OBJ_PART_MAIN, &title_style);
    lv_label_set_static_text(tab_title_label, "M5Stack\nCore2 for AWS IoT EduKit");
    lv_label_set_align(tab_title_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(tab_title_label, home_tab, LV_ALIGN_IN_TOP_MID, 0, 50);

    lv_obj_t* body_label = lv_label_create(home_tab, NULL);
    lv_label_set_long_mode(body_label, LV_LABEL_LONG_BREAK);
    lv_label_set_static_text(body_label, "Swipe through to learn about some of the hardware features.");
    lv_obj_set_width(body_label, 280);
    lv_obj_align(body_label, home_tab, LV_ALIGN_CENTER, 0 , 10);
    
    lv_obj_t* arrow_label = lv_label_create(home_tab, NULL);
    lv_label_set_recolor(arrow_label, true);
    size_t arrow_buf_len = snprintf (NULL, 0, 
        "#ff9900 %1$s       %1$s       %1$s#       #232f3e SWIPE #      #ff9900 %1$s       %1$s       %1$s#       #232f3e SWIPE #", 
        LV_SYMBOL_LEFT);
    char arrow_buf[++arrow_buf_len];
    snprintf (arrow_buf, arrow_buf_len, 
        "#ff9900 %1$s       %1$s       %1$s#       #232f3e SWIPE #      #ff9900 %1$s       %1$s       %1$s#       #232f3e SWIPE #", 
        LV_SYMBOL_LEFT);
    lv_label_set_text(arrow_label, arrow_buf);
    lv_label_set_long_mode(arrow_label, LV_LABEL_LONG_SROLL_CIRC);
    lv_label_set_anim_speed(arrow_label, 50);
    lv_obj_set_size(arrow_label, 290, 20);
    lv_label_set_align(arrow_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(arrow_label, home_tab, LV_ALIGN_IN_BOTTOM_MID, 0 , -40);
    xSemaphoreGive(xGuiSemaphore);
    
    ESP_LOGI(TAG, "\n\nWelcome to your M5Stack Core2 for AWS IoT EduKit reference hardware! Visit https://edukit.workshop.aws to view the tutorials and start learning how to build IoT solutions using AWS services.\n\n");
}