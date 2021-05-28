
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

#include "light.h"
#include <string.h>

#include <esp_log.h>

#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_standard_types.h> 
#include <esp_rmaker_standard_params.h> 

#include "user_parameters.h"

#include "display.h"

#include "hsv2rgb.h"

#include "core2forAWS.h"

#define TAG "light"

#define DEFAULT_HUE         180
#define DEFAULT_SATURATION  100
#define DEFAULT_BRIGHTNESS  25

#define LED_COUNT 10

static esp_rmaker_device_t *light_device;

static bool isOn = true;
static uint8_t hue = DEFAULT_HUE;
static uint8_t saturation = DEFAULT_SATURATION;
static uint8_t brightness = DEFAULT_BRIGHTNESS;

static esp_err_t light_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
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
        light_set_on(val.val.b);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        light_set_brightness(val.val.i);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_HUE_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        light_set_hue(val.val.i);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_SATURATION_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        light_set_saturation(val.val.i);
    } else {
        /* Silently ignoring invalid params */
        return ESP_OK;
    }
    esp_rmaker_param_update_and_report(param, val);
    return ESP_OK;
}

void light_init(esp_rmaker_node_t *node)
{
    display_house_init();

    Core2ForAWS_Sk6812_SetBrightness(255);

    light_set_on(false);

    /* Create a Light device and add the relevant parameters to it */
    light_device = esp_rmaker_lightbulb_device_create("Light", NULL, DEFAULT_LIGHT_POWER);
    esp_rmaker_device_add_cb(light_device, light_cb, NULL);
    
    esp_rmaker_device_add_param(light_device, esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME, DEFAULT_LIGHT_BRIGHTNESS));
    esp_rmaker_device_add_param(light_device, esp_rmaker_hue_param_create(ESP_RMAKER_DEF_HUE_NAME, DEFAULT_LIGHT_HUE));
    esp_rmaker_device_add_param(light_device, esp_rmaker_saturation_param_create(ESP_RMAKER_DEF_SATURATION_NAME, DEFAULT_LIGHT_SATURATION));

    esp_rmaker_device_add_attribute(light_device, "Serial Number", "012345");
    esp_rmaker_device_add_attribute(light_device, "MAC", "xx:yy:zz:aa:bb:cc");

    esp_rmaker_node_add_device(node, light_device);
}

void light_set_on(bool on)
{
    isOn = on;
    if(on)
    {
        //uint32_t color = hsb_to_rgb_int(hue,saturation,brightness);
        uint32_t color = hsv2rgb(hue,saturation, brightness);

        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT,  color);
        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, color);
        Core2ForAWS_Sk6812_Show();
        display_lights_on(hue,saturation,brightness);
    }
    else
    {
        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT,  0);
        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, 0);
        Core2ForAWS_Sk6812_Show();
        display_lights_off();
    }    
}

void light_set_hue(int h)
{
    hue = h;
    light_set_on(true);
}

void light_set_saturation(int s)
{
    saturation = s;
    light_set_on(true);
}

void light_set_brightness(int b)
{
    brightness = b;
    light_set_on(true);
}

bool light_is_on(void)
{
    return isOn;
}

int light_get_hue(void)
{
    return hue;
}

int light_get_saturation(void)
{
    return saturation;
}

int light_get_brightness(void)
{
    return brightness;
}

