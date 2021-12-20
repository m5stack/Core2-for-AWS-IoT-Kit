
/*
 * Core2 for AWS IoT EduKit BSP v2.0.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 *
 * https://aws.amazon.com/iot/edukit
 *
 */

/**
 * @file core2foraws_rgb_led.c
 * @brief Core2 for AWS IoT EduKit RGB LEDs hardware driver APIs
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "led_strip.h"
#include "rgb.h"
#include "core2foraws_common.h"
#include "core2foraws_rgb_led.h"

static const char *_s_TAG = "CORE2FORAWS_RGB_LED";

#define RGB_LED_TYPE LED_STRIP_SK6812
#define RGB_LED_GPIO_DATA_PIN GPIO_NUM_25
#define RGB_LED_BRIGHTNESS_DEFAULT 255

static led_strip_t _s_led_strip = 
{
    .type = RGB_LED_TYPE,
    .length = RGB_LED_NUMS,
    .gpio = RGB_LED_GPIO_DATA_PIN,
    .buf = NULL,
    .brightness = RGB_LED_BRIGHTNESS_DEFAULT,
};

esp_err_t core2foraws_rgb_led_init( void )
{
    ESP_LOGI( _s_TAG, "\tInitializing" );
    esp_err_t err = ESP_OK;

    led_strip_install();

    err = led_strip_init(&_s_led_strip);

    return err;
}

esp_err_t core2foraws_rgb_led_single_color_set( uint8_t led_num, uint32_t color )
{
    esp_err_t err = ESP_OK;

    err = led_strip_set_pixel( &_s_led_strip, led_num, rgb_from_code( color ) );

    return err;
}

esp_err_t core2foraws_rgb_led_side_color_set( rgb_led_side_type_t side, uint32_t color )
{
    esp_err_t err = ESP_OK;

    if ( side == RGB_LED_SIDE_RIGHT )
    {
        err |= led_strip_fill( &_s_led_strip, 0, RGB_LED_NUMS / 2, rgb_from_code( color ) );
    }
    else
    {
        err |= led_strip_fill( &_s_led_strip, RGB_LED_NUMS / 2, RGB_LED_NUMS / 2, rgb_from_code( color ) );
    }

    return core2foraws_common_error( err );
}

esp_err_t core2foraws_rgb_led_brightness_set( uint8_t brightness )
{
    esp_err_t err = ESP_OK;

    _s_led_strip.brightness = brightness;

    return err;
}

esp_err_t core2foraws_rgb_led_write( void )
{
    esp_err_t err = ESP_OK;

    err = led_strip_flush( &_s_led_strip );

    return err;
}

esp_err_t core2foraws_rgb_led_clear( void )
{
    esp_err_t err = ESP_OK;
    
    err = led_strip_fill( &_s_led_strip, 0, RGB_LED_NUMS, rgb_from_code( 0x00000000 ) );

    return err;
}

esp_err_t core2foraws_rgb_led_deinit( void )
{
    esp_err_t err = ESP_OK;
    
    err = led_strip_free( &_s_led_strip );

    return err;
}