/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include <freertos/FreeRTOS.h>

typedef uint32_t EventBits_t;
typedef struct { EventBits_t bits; } StaticEventGroup_t;
typedef StaticEventGroup_t *EventGroupHandle_t;

#ifndef pdFALSE
#define pdFALSE 0
#endif
#define portMAX_DELAY 0xffffffffU
#define BIT0 0x01U
#define BIT1 0x02U
#define BIT2 0x04U
#define BIT3 0x08U
#define BIT4 0x10U
#define BIT5 0x20U
#define BIT6 0x40U

EventGroupHandle_t xEventGroupCreateStatic(StaticEventGroup_t *storage);
EventBits_t xEventGroupSetBits(EventGroupHandle_t group, EventBits_t bits);
EventBits_t xEventGroupClearBits(EventGroupHandle_t group, EventBits_t bits);
EventBits_t xEventGroupGetBits(EventGroupHandle_t group);
EventBits_t xEventGroupWaitBits(EventGroupHandle_t group, EventBits_t bits, BaseType_t clear,
                                BaseType_t all, TickType_t timeout);
