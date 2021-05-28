#include "stdio.h"
#include "stdbool.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "ft6336u.h"
#include "i2c_device.h"

#define FT6336U_I2C_ADDR 0x38
#define FT6336U_INTR_PIN 39

static uint16_t _x, _y;
static bool _pressed;
static I2CDevice_t ft6336u_i2c;
static xTaskHandle ft6336_task_handle;
static SemaphoreHandle_t thread_mutex;

static void IRAM_ATTR FT6336U_ISRHandler(void* arg);
static void FT6336U_UpdateTask(void *arg);

void FT6336U_Init() {
    ft6336u_i2c = i2c_malloc_device(I2C_NUM_1, 21, 22, 400000, FT6336U_I2C_ADDR);
    i2c_write_byte(ft6336u_i2c, 0xa4, 0x00);
    
    thread_mutex = xSemaphoreCreateMutex();

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (1ULL << FT6336U_INTR_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    xTaskCreatePinnedToCore(FT6336U_UpdateTask, "FT6336Task", 2 * 1024, NULL, 1, &ft6336_task_handle, 0);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(FT6336U_INTR_PIN, FT6336U_ISRHandler, &ft6336_task_handle);
}

static void IRAM_ATTR FT6336U_ISRHandler(void* arg) {
    xTaskHandle task_handle = *(xTaskHandle *)arg;
    xTaskResumeFromISR(task_handle);
}

static void FT6336U_UpdateTask(void *arg) {
    uint8_t buff[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    bool press_stash;
    for (;;) {
        i2c_read_bytes(ft6336u_i2c, 0x02, buff, 5);
        
        xSemaphoreTake(thread_mutex, portMAX_DELAY);
        _pressed = buff[0] ? true : false;
        _x = ((buff[1] & 0x0f) << 8) | buff[2];
        _y = ((buff[3] & 0x0f) << 8) | buff[4];
        press_stash = _pressed;
        xSemaphoreGive(thread_mutex);

        if (press_stash == false) {
            vTaskSuspend(NULL);
        } else {
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}

void FT6336U_GetTouch(uint16_t* x, uint16_t* y, bool* press_down) {
    xSemaphoreTake(thread_mutex, portMAX_DELAY);
    *x = _x;
    *y = _y;
    *press_down = _pressed;
    xSemaphoreGive(thread_mutex);
}

bool FT6336U_WasPressed() {
    bool state = 0;
    xSemaphoreTake(thread_mutex, portMAX_DELAY);
    state = _pressed;
    xSemaphoreGive(thread_mutex);
    return state;
}

uint16_t FT6336U_GetPressPosX() {
    uint16_t stash = 0;
    xSemaphoreTake(thread_mutex, portMAX_DELAY);
    stash = _x;
    xSemaphoreGive(thread_mutex);
    return stash;
}

uint16_t FT6336U_GetPressPosY() {
    uint16_t stash = 0;
    xSemaphoreTake(thread_mutex, portMAX_DELAY);
    stash = _y;
    xSemaphoreGive(thread_mutex);
    return stash;
}
