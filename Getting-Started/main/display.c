/*
 * AWS IoT EduKit - Getting Started v1.0.2
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
#include "display.h"
#include <esp_log.h>
#include <esp_rmaker_core.h>
#include "core2forAWS.h"
#include "fan.h"

#define TAG "display"

LV_IMG_DECLARE(house_on);
LV_IMG_DECLARE(house_off);
LV_IMG_DECLARE(thermometer);
LV_IMG_DECLARE(fan_off);
LV_IMG_DECLARE(fan_1);
LV_IMG_DECLARE(fan_2);
LV_IMG_DECLARE(fan_3);
LV_IMG_DECLARE(fan_4);
LV_IMG_DECLARE(fan_5);
LV_IMG_DECLARE(fan_6);

static const lv_img_dsc_t *fanImages[] = {&fan_1,&fan_2,&fan_3,&fan_4,&fan_5,&fan_6};

#define CANVAS_WIDTH 100
#define CANVAS_HEIGHT 60
static lv_color_t window_buffer[LV_CANVAS_BUF_SIZE_INDEXED_1BIT(CANVAS_WIDTH,CANVAS_HEIGHT)];
static lv_obj_t *window_object;

static lv_obj_t *light_object;
static lv_obj_t *fan_object;

static lv_obj_t *fan_strength_slider;
static lv_obj_t *fan_sw1;

static lv_obj_t *temperature_object;
#define THREAD_WIDTH 10
#define THREAD_HEIGHT 112
static lv_color_t temperature_buffer[LV_CANVAS_BUF_SIZE_INDEXED_1BIT(THREAD_WIDTH, THREAD_HEIGHT)];
static lv_obj_t *thread_object;
static lv_obj_t *temperature_label;

static int g_fan_speed = 0;
static bool g_fan_power = false;

static void spin_update(void *priv)
{
    int fan_index = 0;
    int speed_skip = 0;
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(50));
        if(pdTRUE == xSemaphoreTake(xGuiSemaphore, 0))
        {
            if(g_fan_speed && g_fan_power)
            {
                if(speed_skip == 0)
                {
                    lv_img_set_src(fan_object, fanImages[fan_index++]);
                    if(fan_index >= (sizeof(fanImages)/sizeof(*fanImages))) fan_index = 0;
                    speed_skip = 5 - g_fan_speed;
                }
                else
                {
                    speed_skip --;
                }
            }
            else
            {
                lv_img_set_src(fan_object, &fan_off);
            }
            xSemaphoreGive(xGuiSemaphore);
        }
    }
}

static void strength_slider_event_cb(lv_obj_t * slider, lv_event_t event)
{

    if(event == LV_EVENT_VALUE_CHANGED) {
        fan_set_speed(lv_slider_get_value(slider));
        fan_speed_report();
    }
}

static void sw1_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        fan_set_power(lv_switch_get_state(obj));
        fan_power_report();
    }
}

void display_init()
{
    Core2ForAWS_Display_SetBrightness(100);
}

void display_fan_init()
{
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);

    ESP_LOGI(TAG,"configuring the fan");

    fan_object = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(fan_object, &fan_off);
    lv_obj_align(fan_object, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -20, 0);
    ESP_LOGI(TAG,"configured fan_object");

    fan_strength_slider = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_set_width(fan_strength_slider, 8);
    lv_obj_set_height(fan_strength_slider, 80);
    lv_obj_align(fan_strength_slider, fan_object, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_set_event_cb(fan_strength_slider, strength_slider_event_cb);
    lv_slider_set_value(fan_strength_slider, 0, LV_ANIM_OFF);
    lv_slider_set_range(fan_strength_slider, 0, 5);

    ESP_LOGI(TAG,"configured fan_strength_slider");

    fan_sw1 = lv_switch_create(lv_scr_act(), NULL);
    lv_obj_set_size(fan_sw1, 60, 20);
    lv_obj_align(fan_sw1, fan_object, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_event_cb(fan_sw1, sw1_event_handler);
    lv_switch_off(fan_sw1, LV_ANIM_OFF);
    ESP_LOGI(TAG,"configured fan_sw1");

    xSemaphoreGive(xGuiSemaphore);

    xTaskCreatePinnedToCore(spin_update, "fan", 4096, NULL, 1, NULL, 1);

    ESP_LOGI(TAG,"fan configured");
}

void display_house_init(void)
{
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    ESP_LOGI(TAG,"configuring the house");

    light_object = lv_img_create(lv_scr_act(),NULL);
    lv_img_set_src(light_object, &house_off);
    lv_obj_align(light_object,lv_scr_act(),LV_ALIGN_IN_TOP_LEFT,0,0);

    window_object = lv_canvas_create(lv_scr_act(),NULL);
    lv_obj_align(window_object, light_object, LV_ALIGN_CENTER,-CANVAS_WIDTH/2,-20);
    lv_canvas_set_buffer(window_object, window_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_INDEXED_1BIT);
    lv_canvas_set_palette(window_object,0,LV_COLOR_TRANSP);
    lv_canvas_set_palette(window_object,1,LV_COLOR_RED);
    lv_color_t c;
    c.full = 1;
    lv_canvas_fill_bg(window_object,c,LV_OPA_100);

    lv_obj_move_background(window_object);
    lv_img_set_src(light_object, &house_off);
    lv_obj_align(light_object,lv_scr_act(),LV_ALIGN_IN_TOP_LEFT,0,0);

    xSemaphoreGive(xGuiSemaphore);
    ESP_LOGI(TAG,"house configured");
}

void display_temperature_init(void)
{
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    ESP_LOGI(TAG,"configuring the temperature");
    temperature_object = lv_img_create(lv_scr_act(),NULL);
    lv_img_set_src(temperature_object, &thermometer);
    lv_obj_align(temperature_object,lv_scr_act(),LV_ALIGN_IN_BOTTOM_RIGHT,0,-10);

    thread_object = lv_canvas_create(lv_scr_act(),NULL);
    lv_obj_align(thread_object, temperature_object, LV_ALIGN_IN_TOP_LEFT,15,0);
    lv_canvas_set_buffer(thread_object, temperature_buffer, THREAD_WIDTH, THREAD_HEIGHT, LV_IMG_CF_INDEXED_1BIT);
    lv_canvas_set_palette(thread_object,0,LV_COLOR_GRAY);
    lv_canvas_set_palette(thread_object,1,LV_COLOR_ORANGE);
    lv_color_t c;
    c.full = 0;
    lv_canvas_fill_bg(thread_object,c,LV_OPA_100);

    lv_obj_move_background(thread_object);

    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(label, temperature_object, LV_ALIGN_IN_RIGHT_MID, -75, -15);
    lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(label, "Internal\nTemperature");

    temperature_label = lv_label_create(lv_scr_act(),NULL);
    lv_obj_align(temperature_label, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_set_width(temperature_label,75);
    _lv_obj_set_style_local_ptr(temperature_label, LV_OBJ_PART_MAIN, LV_STYLE_TEXT_FONT, &lv_font_montserrat_16);  /*Set a larger font*/
    lv_label_set_align(temperature_label,LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(temperature_label,"");


    xSemaphoreGive(xGuiSemaphore);
    ESP_LOGI(TAG,"temperature configured");
}

void display_lights_off(void)
{
    ESP_LOGI(TAG,"lights off");
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);

    lv_img_set_src(light_object, &house_off);

    xSemaphoreGive(xGuiSemaphore);
}

void display_lights_on(int h, int s, int v)
{
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);

    lv_color_t c = lv_color_hsv_to_rgb(h,s,v);

    lv_canvas_set_palette(window_object,1,c);

    lv_img_set_src(light_object, &house_on);

    xSemaphoreGive(xGuiSemaphore);
}

void display_fan_speed(int s)
{
    lv_slider_set_value(fan_strength_slider,s,LV_ANIM_OFF);
    ESP_LOGI(TAG,"fan spinning %d",s);
    g_fan_speed = s;
}

void display_fan_off()
{
    ESP_LOGI(TAG,"fan off");
    lv_switch_off(fan_sw1, LV_ANIM_OFF);
    g_fan_power = false;
}

void display_fan_on()
{
    ESP_LOGI(TAG,"switch on");
    lv_switch_on(fan_sw1, LV_ANIM_OFF);
    g_fan_power = true;
}

void display_switch_on()
{
    ESP_LOGI(TAG,"switch on");
}

void display_switch_off()
{
    ESP_LOGI(TAG,"switch off");
}

void display_temperature(float c)
{
    const float maxTemp_c = 100.0;
    const float minTemp_c = 0.0;

    int rect_height = (int)(((float)THREAD_HEIGHT * (c - minTemp_c)) / (maxTemp_c - minTemp_c));

    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);

    lv_color_t tc;
    tc.full = 0;
    lv_canvas_fill_bg(thread_object,tc,LV_OPA_100);

    tc.full = 1;

    for(int x=0;x<THREAD_WIDTH;x++)
    {
        for(int y=0;y<rect_height;y++)
        {
            lv_canvas_set_px(thread_object,x,THREAD_HEIGHT-y,tc);
        }
    }

    lv_label_set_text_fmt(temperature_label,"%dC",(int)c);

    xSemaphoreGive(xGuiSemaphore);
}
