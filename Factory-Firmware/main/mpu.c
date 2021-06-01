/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit
 * Factory Firmware v2.0.3
 * mpu.c
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
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "core2forAWS.h"

#include "mpu.h"
#include "gauge_hand.c"

/* 
If you want to use Sebastian Madgwick's algorithym to calculate roll, pitch, yaw, add the libs from: 
https://x-io.co.uk/open-source-imu-and-ahrs-algorithms/

Note that since there is no magnometer (and it wouldn't work correctly due to magnets in the housing) 
there will be a drift in the Yaw. The drift cannot be correct without creating a custom algorithm to 
serve as a filter for the drift. Uncomment the commented code below. You will need to make modifications
to the imu-and-ahrs-algorithms library for it to write to the pitch, yaw, roll pointers.
    
Note: Sebastian Madgwick's algorithym implementation library is GPL licensed.
*/

// #include "MahonyAHRS.h"

// #define DEGREES_TO_RADIANS M_PI/180
// #define RADIANS_TO_DEGREES 180/M_PI

static const char* TAG = MPU_TAB_NAME;

void display_mpu_tab(lv_obj_t* tv){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);   // Takes (blocks) the xGuiSemaphore mutex from being read/written by another task.
    
    lv_obj_t* mpu_tab = lv_tabview_add_tab(tv, MPU_TAB_NAME); // Create a tab
    /* Create the main body object and set background within the tab*/
    static lv_style_t bg_style;
    lv_obj_t* mpu_bg = lv_obj_create(mpu_tab, NULL);
    lv_obj_align(mpu_bg, NULL, LV_ALIGN_IN_TOP_LEFT, 16, 36);
    lv_obj_set_size(mpu_bg, 290, 190);
    lv_obj_set_click(mpu_bg, false);
    lv_style_init(&bg_style);
    lv_style_set_bg_color(&bg_style, LV_STATE_DEFAULT, lv_color_make(169, 0, 103));
    lv_obj_add_style(mpu_bg, LV_OBJ_PART_MAIN, &bg_style);

    /* Create the title within the main body object */
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
    lv_style_set_text_color(&title_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_t* tab_title_label = lv_label_create(mpu_bg, NULL);
    lv_obj_add_style(tab_title_label, LV_OBJ_PART_MAIN, &title_style);
    lv_label_set_static_text(tab_title_label, "MPU6886 IMU Sensor");
    lv_obj_align(tab_title_label, mpu_bg, LV_ALIGN_IN_TOP_MID, 0, 10);

    /* Create the sensor information label object */
    lv_obj_t* body_label = lv_label_create(mpu_bg, NULL);
    lv_label_set_long_mode(body_label, LV_LABEL_LONG_BREAK);
    lv_label_set_static_text(body_label, "The Inertial Measurement Unit (IMU) senses the motion of the device.");
    lv_obj_set_width(body_label, 120);
    lv_obj_align(body_label, mpu_bg, LV_ALIGN_IN_LEFT_MID, 20, 0);

    static lv_style_t body_style;
    lv_style_init(&body_style);
    lv_style_set_text_color(&body_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_add_style(body_label, LV_OBJ_PART_MAIN, &body_style);

    /* Create the sensor color legend */
    lv_obj_t* lgnd_bg = lv_obj_create(mpu_bg, NULL);
    lv_obj_set_size(lgnd_bg, 200, 24);
    lv_obj_align(lgnd_bg, mpu_bg, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    lv_obj_t* legend_label = lv_label_create(lgnd_bg, NULL);
    lv_label_set_recolor(legend_label, true); // Enable recoloring of the text within the label with color HEX
    lv_label_set_static_text(legend_label, "#ff0000 Rot_X#    #008000 Rot_Y#    #0000ff Rot_Z#");
    lv_label_set_align(legend_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(legend_label, lgnd_bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color( lgnd_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE );
    
    /* Create a gauge */
    static lv_color_t gauge_needle_colors[3];
    gauge_needle_colors[0] = LV_COLOR_RED;
    gauge_needle_colors[1] = LV_COLOR_GREEN;
    gauge_needle_colors[2] = LV_COLOR_BLUE;

    LV_IMG_DECLARE(gauge_hand);

    lv_obj_t* gauge = lv_gauge_create(mpu_bg, NULL);
    lv_obj_set_click(gauge, false);
    lv_obj_set_size(gauge, 106, 106);
    lv_gauge_set_scale(gauge, 300, 10, 0);
    lv_gauge_set_range(gauge, -400, 400);
    lv_gauge_set_critical_value(gauge, 2001);
    lv_gauge_set_needle_count(gauge, 3, gauge_needle_colors);
    lv_gauge_set_needle_img(gauge, &gauge_hand, 5, 4);
    lv_obj_set_style_local_image_recolor_opa(gauge, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, LV_OPA_COVER);

    lv_obj_align(gauge, NULL, LV_ALIGN_IN_RIGHT_MID, -20, 0);
    xSemaphoreGive(xGuiSemaphore);
    
    /* 
    Create a task to read the MPU values running on the 2nd Core. 
    Pass in pointer to the gauge object to display value on the gauge.
    */
    xTaskCreatePinnedToCore(MPU_task, "MPUTask", 2048, (void*) gauge, 1, &MPU_handle, 1);
}

void MPU_task(void* pvParameters){
    float calib_gx = 0.00;
    float calib_gy = 0.00;
    float calib_gz = 0.00;

    float calib_ax = 0.00;
    float calib_ay = 0.00;
    float calib_az = 0.00;

    MPU6886_GetAccelData(&calib_ax, &calib_ay, &calib_az);
    MPU6886_GetGyroData(&calib_gx, &calib_gy, &calib_gz);
    
    vTaskSuspend(NULL);

    for (;;) {
        float gx, gy, gz;
        float ax, ay, az;
        MPU6886_GetAccelData(&ax, &ay, &az);
        MPU6886_GetGyroData(&gx, &gy, &gz);
        

        // float pitch, roll, yaw;
        // MahonyAHRSupdateIMU(gx * DEGREES_TO_RADIANS, gy * DEGREES_TO_RADIANS, gz * DEGREES_TO_RADIANS, ax, ay, az, &pitch, &roll, &yaw);
        // ESP_LOGI(TAG, "Pitch: %.6f Roll: %.6f Yaw: %.6f | Raw Accel: X-%.6f Y-%.6f Z-%.6f | Gyro: X-%.6f Y-%.6fZ- %.6f", pitch, yaw, roll, ax, ay, az, gx, gy, gz);

        ESP_LOGI(TAG, "Raw Accel: X-%.6f Y-%.6f Z-%.6f | Gyro: X-%.6f Y-%.6fZ- %.6f", ax, ay, az, gx, gy, gz);

        lv_obj_t* gauges = (lv_obj_t*) pvParameters;
        
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        lv_gauge_set_value(gauges, 0, (int) (gx-calib_gx));
        lv_gauge_set_value(gauges, 1, (int) (gy-calib_gy));
        lv_gauge_set_value(gauges, 2, (int) (gz-calib_gz));
        xSemaphoreGive(xGuiSemaphore); 
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL); // Should never get to here...
}