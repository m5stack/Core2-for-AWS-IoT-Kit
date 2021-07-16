/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit
 * Cloud Connected Blinky v1.3.1
 * blink.c
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "core2forAWS.h"
#include "blink.h"

static const char *TAG = "Blink";

void blink_task(void *arg) {
    vTaskSuspend( NULL );
    while (1) {
        Core2ForAWS_Sk6812_Clear();
        Core2ForAWS_Sk6812_Show();

        while (1) {      
            Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT, 0x000000);
            Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, 0xffffff);
            Core2ForAWS_Sk6812_Show();
            vTaskDelay(100);
            
            Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT, 0xffffff);
            Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, 0x000000);
            Core2ForAWS_Sk6812_Show();
            vTaskDelay(100);
        }
    }
    // Should never get here. FreeRTOS tasks loop forever.
    ESP_LOGE(TAG, "Error in blink task. Out of loop.");
    abort();
}
