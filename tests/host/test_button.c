/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_err.h"

#define pdPASS pdTRUE
#define portMAX_DELAY 0xFFFFFFFFU
typedef void (*TaskFunction_t)(void *);

TickType_t xTaskGetTickCount(void);
BaseType_t xTaskCreatePinnedToCore(TaskFunction_t task_fn, const char *name,
                                   uint32_t stack_depth, void *params,
                                   unsigned int priority, TaskHandle_t *handle,
                                   int core_id);
esp_err_t core2foraws_common_task_stack_watermark(const char *tag,
                                                  TaskHandle_t task,
                                                  size_t *watermark_bytes);

#include "core2foraws_display.h"
#include "../../lib/button/core2foraws_button.c"

static TickType_t current_ticks;
static bool mutex_fail, task_create_fail, lock_timeout;
static TaskFunction_t captured_task_fn;
static unsigned int touch_reads, watermark_calls;
static uint8_t mock_touch_cnt;
static esp_lcd_touch_point_data_t mock_points[2];
static unsigned int press_events[3], release_events[3], longpress_events[3];
static jmp_buf poll_exit_jmp;
static unsigned int poll_iterations_remaining;

SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *storage)
{
    return mutex_fail ? NULL : storage;
}
BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t timeout)
{
    assert(mutex != NULL);
    assert(timeout == portMAX_DELAY || timeout == 101);
    if (lock_timeout) return 0;
    mutex->depth++;
    return pdPASS;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex)
{
    assert(mutex != NULL && mutex->depth > 0);
    mutex->depth--;
    return pdPASS;
}
void vTaskDelay(TickType_t ticks)
{
    if (ticks == CORE2FORAWS_DELAY_MS_TO_TICKS(BUTTON_POLL_INTERVAL_MS))
    {
        if (poll_iterations_remaining == 0)
        {
            longjmp(poll_exit_jmp, 1);
        }
        poll_iterations_remaining--;
    }
}
TickType_t xTaskGetTickCount(void) { return current_ticks; }
BaseType_t xTaskCreatePinnedToCore(TaskFunction_t task_fn, const char *name,
                                   uint32_t stack_depth, void *params,
                                   unsigned int priority, TaskHandle_t *handle,
                                   int core_id)
{
    assert(strcmp(name, "buttonPress") == 0);
    assert(stack_depth == 4096 && params == NULL && priority == 1 && core_id == 1);
    if (task_create_fail) return 0;
    captured_task_fn = task_fn;
    *handle = (TaskHandle_t)1;
    return pdPASS;
}
esp_err_t core2foraws_common_task_stack_watermark(const char *tag,
                                                  TaskHandle_t task,
                                                  size_t *watermark_bytes)
{
    assert(tag != NULL && task == NULL && watermark_bytes != NULL);
    *watermark_bytes = 2048;
    watermark_calls++;
    return ESP_OK;
}
esp_err_t core2foraws_display_touch_data_get(esp_lcd_touch_point_data_t *data,
                                             uint8_t *touch_cnt,
                                             uint8_t max_points)
{
    assert(data != NULL && touch_cnt != NULL && max_points == 2);
    touch_reads++;
    *touch_cnt = mock_touch_cnt;
    for (uint8_t i = 0; i < mock_touch_cnt && i < max_points; i++)
    {
        data[i] = mock_points[i];
    }
    return ESP_OK;
}

static void run_poll_step(TickType_t ticks)
{
    current_ticks = ticks;
    poll_iterations_remaining = 1;
    if (setjmp(poll_exit_jmp) == 0)
    {
        captured_task_fn(NULL);
    }
}

static void on_button_event(enum core2foraws_button_btns button, press_event_t event)
{
    assert(button >= BUTTON_LEFT && button <= BUTTON_RIGHT);
    if (event == PRESS) press_events[button]++;
    else if (event == RELEASE) release_events[button]++;
    else if (event == LONGPRESS) longpress_events[button]++;
}

int main(void)
{
    /* Uninitialized register/unregister must return ESP_ERR_INVALID_STATE */
    assert(core2foraws_button_register_callback(BUTTON_LEFT, PRESS, on_button_event) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_button_unregister_callback(BUTTON_LEFT, PRESS) == ESP_ERR_INVALID_STATE);

    /* Invalid arguments rejected before initialization check */
    assert(core2foraws_button_register_callback(BUTTON_LEFT, PRESS, NULL) == ESP_ERR_INVALID_ARG);
    assert(core2foraws_button_register_callback(BUTTON_LEFT, 0, on_button_event) == ESP_ERR_INVALID_ARG);
    assert(core2foraws_button_register_callback((enum core2foraws_button_btns)3, PRESS, on_button_event) == ESP_ERR_INVALID_ARG);

    /* Task creation failure leaves subsystem uninitialized and retryable */
    task_create_fail = true;
    assert(core2foraws_button_init() == ESP_FAIL);
    assert(core2foraws_button_register_callback(BUTTON_LEFT, PRESS, on_button_event) == ESP_ERR_INVALID_STATE);
    task_create_fail = false;
    assert(core2foraws_button_init() == ESP_OK);
    assert(core2foraws_button_init() == ESP_OK);

    /* Idle poll skip: with no callbacks registered and no buttons touched,
     * button_press_task logs stack watermark once and skips I2C touch reads. */
    run_poll_step(0);
    run_poll_step(2);
    assert(watermark_calls == 1 && touch_reads == 0);

    /* Mutex timeout on register/unregister returns ESP_ERR_TIMEOUT */
    lock_timeout = true;
    assert(core2foraws_button_register_callback(BUTTON_LEFT, PRESS, on_button_event) == ESP_ERR_TIMEOUT);
    assert(core2foraws_button_unregister_callback(BUTTON_LEFT, PRESS) == ESP_ERR_TIMEOUT);
    lock_timeout = false;

    /* Register callbacks for left and right buttons (multi-touch + debounce + longpress) */
    assert(core2foraws_button_register_callback(
               BUTTON_LEFT, PRESS | RELEASE | LONGPRESS, on_button_event) == ESP_OK);
    assert(core2foraws_button_register_callback(
               BUTTON_RIGHT, PRESS | RELEASE, on_button_event) == ESP_OK);

    /* Touch both left (x=50,y=260) and right (x=250,y=260) simultaneously */
    mock_touch_cnt = 2;
    mock_points[0].x = 50;  mock_points[0].y = 260;
    mock_points[1].x = 250; mock_points[1].y = 260;

    /* First sample starts debounce timer (BUTTON_DEBOUNCE_TICKS = 4 ticks at 100 Hz) */
    run_poll_step(10);
    assert(touch_reads == 1);
    assert(press_events[BUTTON_LEFT] == 0 && press_events[BUTTON_RIGHT] == 0);

    /* Before debounce elapses (tick 13 < 10 + 4), still not pressed */
    run_poll_step(13);
    assert(press_events[BUTTON_LEFT] == 0 && press_events[BUTTON_RIGHT] == 0);

    /* Debounce elapsed at tick 14: both PRESS callbacks fire */
    run_poll_step(14);
    assert(press_events[BUTTON_LEFT] == 1 && press_events[BUTTON_RIGHT] == 1);
    assert(longpress_events[BUTTON_LEFT] == 0);

    /* Hold past long press threshold (41 ticks from press at tick 14 -> tick 55) */
    run_poll_step(55);
    assert(longpress_events[BUTTON_LEFT] == 1);
    /* Holding further does not re-fire LONGPRESS */
    run_poll_step(60);
    assert(longpress_events[BUTTON_LEFT] == 1);

    /* Release both buttons and wait for debounce */
    mock_touch_cnt = 0;
    run_poll_step(62);
    assert(release_events[BUTTON_LEFT] == 0 && release_events[BUTTON_RIGHT] == 0);
    run_poll_step(66);
    assert(release_events[BUTTON_LEFT] == 1 && release_events[BUTTON_RIGHT] == 1);

    /* Unregister all callbacks and verify polling is skipped again once idle */
    assert(core2foraws_button_unregister_callback(
               BUTTON_LEFT, PRESS | RELEASE | LONGPRESS) == ESP_OK);
    assert(core2foraws_button_unregister_callback(
               BUTTON_RIGHT, PRESS | RELEASE) == ESP_OK);
    unsigned int reads_before = touch_reads;
    run_poll_step(70);
    assert(touch_reads == reads_before);
    assert(_button_mutex->depth == 0);

    puts("Button idle poll skip, debounce, multi-touch and long-press tests passed");
    return 0;
}
