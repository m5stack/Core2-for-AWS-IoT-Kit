/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include <stdbool.h>
#include <stdatomic.h>
#include "esp_err.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#define CORE2FORAWS_SPI_LOCK_TIMEOUT_MS 500
#define CORE2FORAWS_DELAY_MS_TO_TICKS(ms) \
    ((TickType_t)((((uint64_t)(ms) * configTICK_RATE_HZ) + 999U) / 1000U) + 1U)

enum
{
    CORE2FORAWS_MUTEX_UNINITIALIZED = 0,
    CORE2FORAWS_MUTEX_INITIALIZING  = 1,
    CORE2FORAWS_MUTEX_READY         = 2,
};

static inline SemaphoreHandle_t _core2foraws_common_semaphore_ensure(
    atomic_uchar *state, SemaphoreHandle_t *mutex,
    StaticSemaphore_t *storage,
    SemaphoreHandle_t (*create_fn)(StaticSemaphore_t *))
{
    if (atomic_load(state) == CORE2FORAWS_MUTEX_READY)
    {
        return *mutex;
    }

    unsigned char expected = CORE2FORAWS_MUTEX_UNINITIALIZED;
    if (atomic_compare_exchange_strong(state, &expected,
                                       CORE2FORAWS_MUTEX_INITIALIZING))
    {
        *mutex = create_fn(storage);
        atomic_store(state, *mutex != NULL ? CORE2FORAWS_MUTEX_READY
                                           : CORE2FORAWS_MUTEX_UNINITIALIZED);
        return *mutex;
    }

    while (atomic_load(state) == CORE2FORAWS_MUTEX_INITIALIZING)
    {
        vTaskDelay(1);
    }

    return atomic_load(state) == CORE2FORAWS_MUTEX_READY ? *mutex : NULL;
}

static inline SemaphoreHandle_t _core2foraws_common_create_mutex_static(
    StaticSemaphore_t *storage)
{
    return xSemaphoreCreateMutexStatic(storage);
}

static inline SemaphoreHandle_t
_core2foraws_common_create_recursive_mutex_static(StaticSemaphore_t *storage)
{
    return xSemaphoreCreateRecursiveMutexStatic(storage);
}

static inline SemaphoreHandle_t core2foraws_common_mutex_ensure(
    atomic_uchar *state, SemaphoreHandle_t *mutex,
    StaticSemaphore_t *storage)
{
    return _core2foraws_common_semaphore_ensure(
        state, mutex, storage, _core2foraws_common_create_mutex_static);
}

static inline SemaphoreHandle_t core2foraws_common_recursive_mutex_ensure(
    atomic_uchar *state, SemaphoreHandle_t *mutex,
    StaticSemaphore_t *storage)
{
    return _core2foraws_common_semaphore_ensure(
        state, mutex, storage,
        _core2foraws_common_create_recursive_mutex_static);
}

extern SemaphoreHandle_t core2foraws_common_spi_semaphore;
esp_err_t core2foraws_common_spi_bus_init(void);
esp_err_t core2foraws_common_error(esp_err_t error);