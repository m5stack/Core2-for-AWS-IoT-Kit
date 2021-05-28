/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit
 * Factory Firmware v2.0.2
 * crypto.c
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

#include "crypto.h"

static const char* TAG = CRYPTO_TAB_NAME;

void display_crypto_tab(lv_obj_t* tv){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);   // Takes (blocks) the xGuiSemaphore mutex from being read/written by another task.

    lv_obj_t* crypto_tab = lv_tabview_add_tab(tv, CRYPTO_TAB_NAME);  // Create a tab

    /* Create the main body object and set background within the tab*/
    static lv_style_t bg_style;
    lv_obj_t* crypto_bg = lv_obj_create(crypto_tab, NULL);
    lv_obj_align(crypto_bg, NULL, LV_ALIGN_IN_TOP_LEFT, 16, 36);
    lv_obj_set_size(crypto_bg, 290, 190);
    lv_obj_set_click(crypto_bg, false);
    lv_style_init(&bg_style);
    lv_style_set_bg_color(&bg_style, LV_STATE_DEFAULT, lv_color_make(4, 151, 150));
    lv_obj_add_style(crypto_bg, LV_OBJ_PART_MAIN, &bg_style);

    /* Create the title within the main body object */
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
    lv_style_set_text_color(&title_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_t* tab_title_label = lv_label_create(crypto_bg, NULL);
    lv_obj_add_style(tab_title_label, LV_OBJ_PART_MAIN, &title_style);
    lv_label_set_static_text(tab_title_label, "ATECC608 Crypto-Auth");
    lv_obj_align(tab_title_label, crypto_bg, LV_ALIGN_IN_TOP_MID, 0, 10);

    /* Create the sensor information label object */
    lv_obj_t* body_label = lv_label_create(crypto_bg, NULL);
    lv_label_set_long_mode(body_label, LV_LABEL_LONG_BREAK);
    lv_label_set_static_text(body_label, "The ATECC608 comes with pre-provisioned static certificates, along with Elliptic Curve Digital Signature Algorithm (ECDSA) sign/verify capability.");
    lv_obj_set_width(body_label, 252);
    lv_obj_align(body_label, crypto_bg, LV_ALIGN_IN_TOP_LEFT, 20, 40);

    static lv_style_t body_style;
    lv_style_init(&body_style);
    lv_style_set_text_color(&body_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_add_style(body_label, LV_OBJ_PART_MAIN, &body_style);

    xSemaphoreGive(xGuiSemaphore);

    static const uint16_t device_serial_len = ATCA_SERIAL_NUM_SIZE * 2 + 1;
    char* device_serial = heap_caps_malloc(device_serial_len, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM); // Dynamically allocate enough memory to store the serial number string. ATCA_SERIAL_NUM_SIZE is the size of the hexadecimal serial number, which has two bytes per value and a string needs a trailing null terminator at the end.
    ATCA_STATUS ret = Atecc608_GetSerialString(device_serial); // Gets the serial number. If successful, it will return ATCA_SUCCESS, which has a value of 0.
    if (ret == ATCA_SUCCESS){
        char sn_label_text[device_serial_len + 9];
        snprintf(sn_label_text, device_serial_len + 9, "Serial # %s", device_serial);
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        lv_obj_t* serial_label = lv_label_create(crypto_bg, NULL);
        lv_label_set_text(serial_label, sn_label_text);
        lv_label_set_align(serial_label, LV_LABEL_ALIGN_CENTER);
        lv_obj_align(serial_label, crypto_bg, LV_ALIGN_IN_BOTTOM_MID, 0, -14);
        lv_obj_add_style(serial_label, LV_OBJ_PART_MAIN, &body_style);
        xSemaphoreGive(xGuiSemaphore);
        heap_caps_free(device_serial);
    } else{
        ESP_LOGE(TAG, "Secure element failure. Error code: %d", ret);
    }
}