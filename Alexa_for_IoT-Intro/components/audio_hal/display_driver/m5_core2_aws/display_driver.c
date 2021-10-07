// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#include <esp_log.h>
#include <core2forAWS.h>
#include <lvgl/lvgl.h>
#include "alexa_logo.c"

#include <display_driver.h>

#define TAG "display_driver_m5_core2"

static void brightness_slider_event_cb(lv_obj_t * slider, lv_event_t event) {
    if(event == LV_EVENT_VALUE_CHANGED) {
        Core2ForAWS_Display_SetBrightness(lv_slider_get_value(slider));
    }
}

void display_function()
{
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);

    // Display Alexa Logo
    lv_obj_t *alexa_logo_mark = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(alexa_logo_mark, &alexa_logo);
    lv_obj_align(alexa_logo_mark, NULL, LV_ALIGN_IN_TOP_MID, 0, 30);

    // Screen brightness slider 
    lv_obj_t * brightness_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(brightness_label, "Screen brightness");
    lv_obj_align(brightness_label, NULL, LV_ALIGN_CENTER, 0, 10);

    lv_obj_t * brightness_slider = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_set_width(brightness_slider, 136);
    lv_obj_align(brightness_slider, brightness_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_set_event_cb(brightness_slider, brightness_slider_event_cb);
    lv_slider_set_value(brightness_slider, 80, LV_ANIM_OFF);
    lv_slider_set_range(brightness_slider, 30, 100);

    // Touch button labels
    lv_obj_t * ptt_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(ptt_label, "Push to Talk");
    lv_obj_align(ptt_label,NULL,LV_ALIGN_IN_BOTTOM_LEFT, 4, -4);

    lv_obj_t * mute_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(mute_label, "Mute");
    lv_obj_align(mute_label,NULL,LV_ALIGN_IN_BOTTOM_MID, 0, -4);

    xSemaphoreGive(xGuiSemaphore);
}

esp_err_t display_driver_init()
{
    /*M5StackCore2 specific Touch and display drivers are already initalized in Core2ForAWS_Init()*/

    display_function();

    return ESP_OK;
}
