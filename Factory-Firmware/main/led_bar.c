/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit
 * Factory Firmware v2.1.0
 * led_bar.c
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
#include "led_bar.h"

#define RED_AMAZON_ORANGE 255
#define GREEN_AMAZON_ORANGE 153
#define BLUE_AMAZON_ORANGE 0
#define AMAZON_ORANGE 16750848 // Amazon Orange in Decimal

static xSemaphoreHandle color_lock;

static uint8_t red = RED_AMAZON_ORANGE, green = GREEN_AMAZON_ORANGE, blue = BLUE_AMAZON_ORANGE;

static const char* TAG = LED_BAR_TAB_NAME;

static void red_event_handler(lv_obj_t* slider, lv_event_t event);
static void green_event_handler(lv_obj_t* slider, lv_event_t event);
static void blue_event_handler(lv_obj_t* slider, lv_event_t event);

void display_LED_bar_tab(lv_obj_t* tv){
    color_lock = xSemaphoreCreateMutex();

    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);

    lv_obj_t* led_bar_tab = lv_tabview_add_tab(tv, LED_BAR_TAB_NAME);

    /* Create the main body object and set background within the tab*/
    lv_obj_t* led_bar_bg = lv_obj_create(led_bar_tab, NULL);
    lv_obj_align(led_bar_bg, NULL, LV_ALIGN_IN_TOP_LEFT, 16, 36);
    lv_obj_set_size(led_bar_bg, 290, 190);
    lv_obj_set_click(led_bar_bg, false);
    
    /* Create the main body object and set background within the tab*/
    static lv_style_t bg_style;
    lv_style_init(&bg_style);
    lv_style_set_bg_color(&bg_style, LV_STATE_DEFAULT, lv_color_make(236, 216, 218));
    lv_obj_add_style(led_bar_bg, LV_OBJ_PART_MAIN, &bg_style);

    /* Create the title within the main body object */
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
    lv_style_set_text_color(&title_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_t* tab_title_label = lv_label_create(led_bar_bg, NULL);
    lv_obj_add_style(tab_title_label, LV_OBJ_PART_MAIN, &title_style);
    lv_label_set_static_text(tab_title_label, "SK6812 LED Bars");
    lv_obj_align(tab_title_label, led_bar_bg, LV_ALIGN_IN_TOP_MID, 0, 10);

    /* Create the sensor information label object */
    lv_obj_t* body_label = lv_label_create(led_bar_bg, NULL);
    lv_label_set_long_mode(body_label, LV_LABEL_LONG_BREAK);
    lv_label_set_static_text(body_label, "The ten SK6812s allow you to control each of the RGB LEDs brightness & color individually.");
    lv_obj_set_width(body_label, 252);
    lv_obj_align(body_label, led_bar_bg, LV_ALIGN_IN_TOP_LEFT, 20, 40);

    static lv_style_t body_style;
    lv_style_init(&body_style);
    lv_style_set_text_color(&body_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_add_style(body_label, LV_OBJ_PART_MAIN, &body_style);

    lv_obj_t* instruction_label = lv_label_create(led_bar_bg, NULL);
    lv_label_set_static_text(instruction_label, "Tap or hold to change color:");
    lv_obj_align(instruction_label, body_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_add_style(instruction_label, LV_OBJ_PART_MAIN, &body_style);


    lv_obj_t* red_label = lv_label_create(led_bar_bg, NULL);
    lv_label_set_static_text(red_label, "Red");
    lv_obj_align(red_label, led_bar_bg, LV_ALIGN_IN_BOTTOM_LEFT, 20, -4);

    static lv_style_t red_lmeter_style;
    lv_style_init(&red_lmeter_style);
    lv_style_set_line_color(&red_lmeter_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_scale_grad_color(&red_lmeter_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_border_opa(&red_lmeter_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_bg_opa(&red_lmeter_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_scale_end_color(&red_lmeter_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    lv_obj_t* red_lmeter = lv_linemeter_create(led_bar_bg, NULL);
    lv_linemeter_set_range(red_lmeter, 0, 255);                   /*Set the range*/
    lv_linemeter_set_value(red_lmeter, RED_AMAZON_ORANGE);                       /*Set the current value*/
    lv_linemeter_set_scale(red_lmeter, 240, 41);                  /*Set the angle and number of lines*/
    lv_obj_add_style(red_lmeter, LV_LINEMETER_PART_MAIN, &red_lmeter_style);
    lv_obj_set_size(red_lmeter, 70, 70);
    lv_obj_align(red_lmeter, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, -4);
    lv_obj_set_event_cb(red_lmeter, red_event_handler);
    
    lv_obj_t* green_label = lv_label_create(led_bar_bg, NULL);
    lv_label_set_static_text(green_label, "Green");
    lv_obj_align(green_label, led_bar_bg, LV_ALIGN_IN_BOTTOM_MID, 0, -8);
    
    static lv_style_t green_lmeter_style;
    lv_style_init(&green_lmeter_style);
    lv_style_set_scale_grad_color(&green_lmeter_style, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    
    lv_obj_t* green_lmeter = lv_linemeter_create(led_bar_bg, red_lmeter);
    lv_obj_add_style(green_lmeter, LV_LINEMETER_PART_MAIN, &green_lmeter_style);
    lv_obj_set_event_cb(green_lmeter, green_event_handler);
    lv_obj_align(green_lmeter, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -4);
    lv_linemeter_set_value(green_lmeter, GREEN_AMAZON_ORANGE);                       /*Set the current value*/

    lv_obj_t* blue_label = lv_label_create(led_bar_bg, NULL);
    lv_label_set_static_text(blue_label, "Blue");
    lv_obj_align(blue_label, led_bar_bg, LV_ALIGN_IN_BOTTOM_RIGHT, -20, -4);
    
    static lv_style_t blue_lmeter_style;
    lv_style_init(&blue_lmeter_style);
    lv_style_set_scale_grad_color(&blue_lmeter_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
    
    lv_obj_t* blue_lmeter = lv_linemeter_create(led_bar_bg, red_lmeter);
    lv_obj_add_style(blue_lmeter, LV_LINEMETER_PART_MAIN, &blue_lmeter_style);
    lv_obj_set_event_cb(blue_lmeter, blue_event_handler);
    lv_obj_align(blue_lmeter, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, -4);
    lv_linemeter_set_value(blue_lmeter, BLUE_AMAZON_ORANGE);                       /*Set the current value*/

    xSemaphoreGive(xGuiSemaphore);
    
    xTaskCreatePinnedToCore(sk6812_animation_task, "sk6812AnimationTask", configMINIMAL_STACK_SIZE * 3, NULL, 1, &led_bar_animation_handle, 1);
}

void update_color(){
    xSemaphoreTake(color_lock, pdMS_TO_TICKS(10));
    uint8_t current_red = red, current_green = green, current_blue = blue;
    xSemaphoreGive(color_lock);
    Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT, (current_red << 16) + (current_green << 8) + (current_blue));
    Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, (current_red << 16) + (current_green << 8) + (current_blue));
    Core2ForAWS_Sk6812_Show();
}

static void red_event_handler(lv_obj_t* lmeter, lv_event_t e){
    xSemaphoreTake(color_lock, pdMS_TO_TICKS(10));
    red = (uint8_t)lv_linemeter_get_value(lmeter);
    if(e == LV_EVENT_SHORT_CLICKED) {
        red += 10;
    } else if(e == LV_EVENT_LONG_PRESSED_REPEAT){
        red +=30;
    }
    lv_linemeter_set_value(lmeter, red);
    xSemaphoreGive(color_lock);
}

static void green_event_handler(lv_obj_t* lmeter, lv_event_t e){
    xSemaphoreTake(color_lock, pdMS_TO_TICKS(10));
    green = (uint8_t)lv_linemeter_get_value(lmeter);
    if(e == LV_EVENT_SHORT_CLICKED) {
        green += 10;
    } else if(e == LV_EVENT_LONG_PRESSED_REPEAT){
        green +=30;
    }
    lv_linemeter_set_value(lmeter, green);
    xSemaphoreGive(color_lock);
}

static void blue_event_handler(lv_obj_t* lmeter, lv_event_t e)
{
    xSemaphoreTake(color_lock, pdMS_TO_TICKS(10));
    blue = (uint8_t)lv_linemeter_get_value(lmeter);
    if(e == LV_EVENT_SHORT_CLICKED) {
        blue += 10;
    } else if(e == LV_EVENT_LONG_PRESSED_REPEAT){
        blue +=30;
    }
    lv_linemeter_set_value(lmeter, blue);
    xSemaphoreGive(color_lock);
}

void sk6812_solid_task(void* pvParameters){
    xSemaphoreTake(color_lock, pdMS_TO_TICKS(10));
    uint8_t current_red = red, current_green = green, current_blue = blue;
    xSemaphoreGive(color_lock);
    Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT, (current_red << 16) + (current_green << 8) + (current_blue));
    Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, (current_red << 16) + (current_green << 8) + (current_blue));
    Core2ForAWS_Sk6812_Show();
    
    while(1){
        if((current_red != red) || (current_green != green) || (current_blue != blue)){
            xSemaphoreTake(color_lock, pdMS_TO_TICKS(10));
            current_red = red, current_green = green, current_blue = blue;
            xSemaphoreGive(color_lock);
            Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT, (current_red << 16) + (current_green << 8) + (current_blue));
            Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, (current_red << 16) + (current_green << 8) + (current_blue));
            Core2ForAWS_Sk6812_Show();
            ESP_LOGI(TAG, "Color changed to #%.2x%.2x%.2x", current_red, current_green, current_blue);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    };
    
    vTaskDelete(NULL);
}

void sk6812_animation_task(void* pvParameters){
    while (1) {
        Core2ForAWS_Sk6812_Clear();
        Core2ForAWS_Sk6812_Show();

        for (uint8_t i = 0; i < 10; i++) {
            Core2ForAWS_Sk6812_SetColor(i, AMAZON_ORANGE);
            Core2ForAWS_Sk6812_Show();
            vTaskDelay(pdMS_TO_TICKS(70));
        }

        for (uint8_t i = 0; i < 10; i++) {
            Core2ForAWS_Sk6812_SetColor(i, 0x000000);
            Core2ForAWS_Sk6812_Show();
            vTaskDelay(pdMS_TO_TICKS(70));
        }

        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT, 0x232f3e);
        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, 0xffffff);
        Core2ForAWS_Sk6812_Show();

        for (uint8_t i = 40; i > 0; i--) {
            Core2ForAWS_Sk6812_SetBrightness(i);
            Core2ForAWS_Sk6812_Show();
            vTaskDelay(pdMS_TO_TICKS(25));
        }

        Core2ForAWS_Sk6812_SetBrightness(20);
    }
    vTaskDelete(NULL); // Should never get to here...
}