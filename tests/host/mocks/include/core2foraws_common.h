/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include "esp_err.h"
#include "freertos/semphr.h"
#define CORE2FORAWS_SPI_LOCK_TIMEOUT_MS 500
#define CORE2FORAWS_DELAY_MS_TO_TICKS(ms) \
    ((TickType_t)((((uint64_t)(ms) * configTICK_RATE_HZ) + 999U) / 1000U) + 1U)
extern SemaphoreHandle_t core2foraws_common_spi_semaphore;
esp_err_t core2foraws_common_spi_bus_init(void);
esp_err_t core2foraws_common_error(esp_err_t error);