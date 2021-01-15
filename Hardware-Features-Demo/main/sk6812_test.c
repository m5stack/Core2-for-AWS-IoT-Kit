#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "core2forAWS.h"
#include "sk6812_test.h"

static xSemaphoreHandle lock;
static uint8_t stop_show = true;

void sk6812Test() {
    lock = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(sk6812ShowTask, "sk6812ShowTask", 4096*2, NULL, 1, NULL, 1);
}

void sk6812ShowTask(void *arg) {
    uint8_t stop_show_stash;
    while (1) {
        Core2ForAWS_Sk6812_Clear();
        Core2ForAWS_Sk6812_Show();
        while (1) {
            xSemaphoreTake(lock, portMAX_DELAY);
            stop_show_stash = stop_show;
            xSemaphoreGive(lock);
            if (stop_show_stash == false) {
                break;
            }
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }

        for (uint8_t i = 0; i < 10; i++) {
            Core2ForAWS_Sk6812_SetColor(i, 0x00ff00);
            Core2ForAWS_Sk6812_Show();

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        for (uint8_t i = 0; i < 10; i++) {
            Core2ForAWS_Sk6812_SetColor(i, 0x000000);
            Core2ForAWS_Sk6812_Show();
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT, 0x00ff00);
        Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, 0xff0000);
        Core2ForAWS_Sk6812_Show();

        for (uint8_t i = 40; i > 0; i--) {
            Core2ForAWS_Sk6812_SetBrightness(i);
            Core2ForAWS_Sk6812_Show();
            vTaskDelay(25 / portTICK_PERIOD_MS);
        }

        Core2ForAWS_Sk6812_SetBrightness(20);
    }
    vTaskDelete(NULL);
}

void sk6812TaskSuspend() {
    xSemaphoreTake(lock, portMAX_DELAY);
    stop_show = true;
    xSemaphoreGive(lock);
}

void sk6812TaskResume() {
    xSemaphoreTake(lock, portMAX_DELAY);
    stop_show = false;    
    xSemaphoreGive(lock);
}
