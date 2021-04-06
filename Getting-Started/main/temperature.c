/*
 * AWS IoT EduKit - Getting Started v1.0.1
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

#include "temperature.h"

#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>

#include "user_parameters.h"

#include "display.h"

#include "mpu6886.h"

static esp_rmaker_device_t *temp_sensor_device;
static esp_timer_handle_t sensor_timer;

static double floatFIR(double inVal)
{
    #define len  5
    static double x[len] = {0.0};
    const double coef[len] = {0.07145877489754458,0.24397551840576512,0.3480951588693804,0.24397551840576512,0.07145877489754458};

    double y = 0.0;
    for (int i = (len-1) ; i > 0 ; i--)
    {
         x[i] = x[i-1];
         y = y + (coef[i] * x[i]);
    }
    x[0] = inVal;
    y = y + (coef[0] * x[0]);
    return y;
}

static void temperature_sensor_update(void *priv)
{
    const int reportReload = REPORTING_PERIOD * 4;
    static int reportingCount = reportReload;
    float temperature;
    float avg_temperature = 0;

    MPU6886_GetTempData(&temperature);

    avg_temperature = floatFIR(temperature);

    display_temperature(avg_temperature);

    reportingCount --;
    if(!reportingCount)
    {
        esp_rmaker_param_update_and_report(
                    esp_rmaker_device_get_param_by_type(temp_sensor_device, ESP_RMAKER_PARAM_TEMPERATURE),
                    esp_rmaker_float(avg_temperature));
        reportingCount = reportReload;
    }
}

void temperature_init(esp_rmaker_node_t *node)
{
    MPU6886_Init();
    /* Create a Temperature Sensor device and add the relevant parameters to it */
    temp_sensor_device = esp_rmaker_temp_sensor_device_create("Temperature Sensor", NULL, DEFAULT_TEMPERATURE);
    esp_rmaker_node_add_device(node, temp_sensor_device);

    /* Start the update task to send the temperature every so often */
    esp_timer_create_args_t sensor_timer_conf = {
        .callback = temperature_sensor_update,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "temperature_sensor_update_tm"
    };
    if (esp_timer_create(&sensor_timer_conf, &sensor_timer) == ESP_OK) {
        esp_timer_start_periodic(sensor_timer, 250000U);
    }

    display_temperature_init();
}

