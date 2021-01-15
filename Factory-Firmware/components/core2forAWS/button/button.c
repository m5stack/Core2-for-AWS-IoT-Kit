#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "ft6336u.h"
#include "button.h"

Button_t* button_ahead = NULL;
static SemaphoreHandle_t button_lock = NULL;
static void Button_UpdateTask(void *arg);

void Button_Init() {
    button_lock = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(Button_UpdateTask, "Button", 2 * 1024, NULL, 1, NULL, 0);
}

Button_t* Button_Attach(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    xSemaphoreTake(button_lock, portMAX_DELAY);
    Button_t *button = (Button_t *)malloc(sizeof(Button_t) * 1);
    button->x = x;
    button->y = y;
    button->w = w;
    button->h = h;
    button->last_value = 0;
    button->last_press_time = 0;
    button->long_press_time = 0;
    button->next = NULL;
    button->state = 0;
    if (button_ahead == NULL) {
        button_ahead = button;
    } else {
        Button_t* button_last = button_ahead;
        while (button_last->next != NULL) {
            button_last = button_last->next;
        }
        button_last->next = button;
    }
    xSemaphoreGive(button_lock);
    return button;
}

uint8_t Button_WasPressed(Button_t* button) {
    xSemaphoreTake(button_lock, portMAX_DELAY);
    uint8_t result = (button->state & PRESS) > 0;
    button->state &= ~PRESS;
    xSemaphoreGive(button_lock);
    return result;
}

uint8_t Button_WasReleased(Button_t* button) {
    xSemaphoreTake(button_lock, portMAX_DELAY);
    uint8_t result = (button->state & RELEASE) > 0;
    button->state &= ~RELEASE;
    xSemaphoreGive(button_lock);
    return result;
}

uint8_t Button_WasLongPress(Button_t* button, uint32_t long_press_time) {
    xSemaphoreTake(button_lock, portMAX_DELAY);
    button->long_press_time = long_press_time;
    uint8_t result = (button->state & LONGPRESS) > 0;
    button->state &= ~LONGPRESS;
    xSemaphoreGive(button_lock);
    return result;
}

uint8_t Button_IsPress(Button_t* button) {
    xSemaphoreTake(button_lock, portMAX_DELAY);
    uint8_t result = (button->value == 1);
    xSemaphoreGive(button_lock);
    return result;
}

uint8_t Button_IsRelease(Button_t* button) {
    xSemaphoreTake(button_lock, portMAX_DELAY);
    uint8_t result = (button->value == 0);
    xSemaphoreGive(button_lock);
    return result;
}

void Button_Update(Button_t* button, uint8_t press, uint16_t x, uint16_t y) {
    uint8_t value = press & !((x < button->x) || (x > (button->x + button->w)) || (y < button->y) || (y > (button->y + button->h)));
    uint32_t now_ticks = xTaskGetTickCount();
    if (value != button->last_value) {
        if (value == 1) {
            button->state |= PRESS;
            button->last_press_time = now_ticks;
        } else {
            if (button->long_press_time && (now_ticks - button->last_press_time > button->long_press_time)) {
                button->state |= LONGPRESS;
            } else {
                button->state |= RELEASE;
            }
        }
        button->last_value = value;
    }
    button->last_value = value;
    button->value = value;
}

static void Button_UpdateTask(void *arg) {
    Button_t* button;
    uint16_t x, y;
    bool press;

    for (;;) {
        FT6336U_GetTouch(&x, &y, &press);
        xSemaphoreTake(button_lock, portMAX_DELAY);
        button = button_ahead;
        while (button != NULL) {
            Button_Update(button, press, x, y);
            button = button->next;
        }
        xSemaphoreGive(button_lock);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}