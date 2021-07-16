/*
 * AWS IoT EduKit - Getting Started v1.0.3
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


#include "fan.h"

#include <string.h>

#include <esp_log.h>

#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_standard_types.h> 
#include <esp_rmaker_standard_params.h> 

#include "display.h"

#include "core2forAWS.h"

#include "user_parameters.h"

#define TAG "FAN"

static esp_rmaker_device_t *fan_device;
static int g_fan_speed = DEFAULT_FAN_SPEED;
static bool g_fan_power = DEFAULT_FAN_POWER;

/* Callback to handle commands received from the RainMaker cloud */
static esp_err_t fan_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    const char *device_name = esp_rmaker_device_get_name(device);
    const char *param_name = esp_rmaker_param_get_name(param);
    if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                val.val.b? "true" : "false", device_name, param_name);
        fan_set_power(val.val.b);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_SPEED_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        fan_set_speed( val.val.i);
    } else {
        /* Silently ignoring invalid params */
        return ESP_OK;
    }
    esp_rmaker_param_update_and_report(param, val);
    return ESP_OK;
}

void fan_init(esp_rmaker_node_t *node)
{
    /* Create a Fan device and add the relevant parameters to it */
    fan_device = esp_rmaker_fan_device_create("Fan", NULL, DEFAULT_FAN_POWER);
    esp_rmaker_device_add_cb(fan_device, fan_cb, NULL);
    esp_rmaker_device_add_param(fan_device, esp_rmaker_speed_param_create(ESP_RMAKER_DEF_SPEED_NAME, DEFAULT_FAN_SPEED));
    esp_rmaker_node_add_device(node, fan_device);

    display_fan_init();
}

void fan_set_speed(int speed)
{
    g_fan_speed = speed;
    if(g_fan_power)
    {
        Core2ForAWS_Motor_SetStrength(g_fan_speed * 20);
    }
    display_fan_speed(g_fan_speed);
}

void fan_power_report(void)
{
    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_type(fan_device, ESP_RMAKER_PARAM_POWER),
            esp_rmaker_bool(g_fan_power));
}

void fan_speed_report(void)
{
    esp_rmaker_param_update_and_report(
        esp_rmaker_device_get_param_by_type(fan_device, ESP_RMAKER_PARAM_SPEED),
        esp_rmaker_int(g_fan_speed));
}

void fan_set_power(bool power)
{
    g_fan_power = power;
    if (power) {
        int speed = g_fan_speed * 20;
        ESP_LOGI(TAG,"power up, speed=%d",speed);
        Core2ForAWS_Motor_SetStrength(speed);
        display_fan_on();
    } else {
        Core2ForAWS_Motor_SetStrength(0);
        display_fan_off();
    }
}
