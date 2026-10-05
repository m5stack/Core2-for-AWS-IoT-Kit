/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include "FreeRTOS.h"

void taskYIELD(void);
void vTaskDelay(TickType_t ticks);