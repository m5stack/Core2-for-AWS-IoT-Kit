#pragma once

#if CONFIG_SOFTWARE_SDCARD_SUPPORT
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#endif

#if CONFIG_SOFTWARE_ILI9342C_SUPPORT
#include "freertos/semphr.h"
#include "lvgl/lvgl.h"
#include "disp_driver.h"
#include "disp_spi.h"
extern SemaphoreHandle_t xGuiSemaphore;
#endif

#if CONFIG_SOFTWARE_FT6336U_SUPPORT
#include "ft6336u.h"
#endif

#if CONFIG_SOFTWARE_SK6812_SUPPORT
#include "sk6812.h"
#define SK6812_SIDE_LEFT 0
#define SK6812_SIDE_RIGHT 1
#endif

#if CONFIG_SOFTWARE_BUTTON_SUPPORT
#include "button.h"
#endif

#define SPI_HOST_USE HSPI_HOST
#define SPI_DMA_CHAN 2

void Core2ForAWS_Init();

#if CONFIG_SOFTWARE_BUTTON_SUPPORT
extern Button_t* button_left;
extern Button_t* button_middle;
extern Button_t* button_right;

void Core2ForAWS_Button_Init();
#endif

void Core2ForAWS_Speaker_Enable(uint8_t state);

void Core2ForAWS_PMU_Init(uint16_t ldo2_volt, uint16_t ldo3_volt, uint16_t dc2_volt, uint16_t dc3_volt);

float Core2ForAWS_PMU_GetBatVolt();

float Core2ForAWS_PMU_GetBatCurrent();

#if CONFIG_SOFTWARE_ILI9342C_SUPPORT
void Core2ForAWS_Display_Init();

void Core2ForAWS_Display_SetBrightness(uint8_t brightness);
#endif

void Core2ForAWS_LED_Enable(uint8_t enable);

void Core2ForAWS_Motor_SetStrength(uint8_t strength);

#if CONFIG_SOFTWARE_SK6812_SUPPORT
void Core2ForAWS_Sk6812_Init();

void Core2ForAWS_Sk6812_SetColor(uint16_t pos, uint32_t color);

void Core2ForAWS_Sk6812_SetSideColor(uint8_t side, uint32_t color);

void Core2ForAWS_Sk6812_SetBrightness(uint8_t brightness);

void Core2ForAWS_Sk6812_Show();

void Core2ForAWS_Sk6812_Clear();
#endif

#if CONFIG_SOFTWARE_SDCARD_SUPPORT
esp_err_t Core2ForAWS_Sdcard_Init(const char* mount, sdmmc_card_t** out_card);
#endif