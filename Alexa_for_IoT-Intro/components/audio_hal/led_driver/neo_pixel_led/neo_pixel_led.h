/*
*
* Copyright 2015-2018 Espressif Systems (Shanghai) PTE LTD
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#ifndef NEO_PIXEL_LED_H
#define NEO_PIXEL_LED_H

#include <stdint.h>

int leds_init(int cnt, int gpio, int no);
void leds_send(uint8_t *data);
void init_led_colour();
void glow_led(uint8_t red, uint8_t green, uint8_t blue, uint8_t position);
esp_err_t neo_pixel_led_init(int led_cnt, int gpio_no);
void va_led_set_pwm(const uint32_t *led_value);

#endif /* NEO_PIXEL_LED_H */
