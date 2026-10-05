/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_board.h
 * @brief Core2 for AWS IoT Kit hardware revision detection APIs
 */

#ifndef _CORE2FORAWS_BOARD_H_
#define _CORE2FORAWS_BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_err.h>

/**
 * @brief Kit model, identified by the IMU on the M5GO bottom.
 */
/* @[declare_core2foraws_board_model_t] */
typedef enum
{
    BOARD_MODEL_UNKNOWN = 0,      /**< @brief No supported IMU answered; the bottom may be missing. */
    BOARD_MODEL_CORE2FORAWS,      /**< @brief Core2 for AWS (K010-AWS): M5GO Bottom for AWS with MPU6886. */
    BOARD_MODEL_CORE2FORAWS_V1_3, /**< @brief Core2 for AWS v1.3 (K010-AWS-V13): M5GO Bottom2 v1.3 with BMI270. */
} core2foraws_board_model_t;
/* @[declare_core2foraws_board_model_t] */

/**
 * @brief 6-axis IMU fitted on the M5GO bottom.
 */
/* @[declare_core2foraws_board_imu_t] */
typedef enum
{
    BOARD_IMU_NONE = 0,  /**< @brief No supported IMU answered at 0x68. */
    BOARD_IMU_MPU6886,   /**< @brief TDK InvenSense MPU6886 (WHO_AM_I 0x19). */
    BOARD_IMU_BMI270,    /**< @brief Bosch BMI270 (CHIP_ID 0x24). */
} core2foraws_board_imu_t;
/* @[declare_core2foraws_board_imu_t] */

/**
 * @brief PDM microphone fitted on the M5GO bottom.
 *
 * Microphones have no readable identity, so this follows the kit model's
 * published schematic.
 */
/* @[declare_core2foraws_board_mic_t] */
typedef enum
{
    BOARD_MIC_UNKNOWN = 0,   /**< @brief Kit model unknown. */
    BOARD_MIC_SPM1423,       /**< @brief Knowles SPM1423HM4H-B (Core2 for AWS). */
    BOARD_MIC_LMD4737T261,   /**< @brief LinkMems LMD4737T261 (Core2 for AWS v1.3). */
} core2foraws_board_mic_t;
/* @[declare_core2foraws_board_mic_t] */

/**
 * @brief LCD controller in the Core2 display module.
 *
 * Either kit model can ship with either controller.
 */
/* @[declare_core2foraws_board_lcd_t] */
typedef enum
{
    BOARD_LCD_UNKNOWN = 0,  /**< @brief Display driver not built or not initialized yet. */
    BOARD_LCD_ILI9342C,     /**< @brief ILITEK ILI9342C. */
    BOARD_LCD_ILI9342E,     /**< @brief ILITEK ILI9342E (units built since 2026-08-07). */
} core2foraws_board_lcd_t;
/* @[declare_core2foraws_board_lcd_t] */

/**
 * @brief Detected hardware revision and the parts that differ between
 * revisions. Name strings are static and never `NULL`.
 */
/* @[declare_core2foraws_board_info_t] */
typedef struct
{
    core2foraws_board_model_t model;      /**< @brief Kit model. */
    core2foraws_board_imu_t imu;          /**< @brief IMU part. */
    core2foraws_board_mic_t microphone;   /**< @brief Microphone part. */
    core2foraws_board_lcd_t lcd;          /**< @brief LCD controller. */
    const char *model_name;               /**< @brief For example "Core2 for AWS v1.3". */
    const char *imu_name;                 /**< @brief For example "BMI270". */
    const char *microphone_name;          /**< @brief For example "LMD4737T261". */
    const char *lcd_name;                 /**< @brief For example "ILI9342E". */
} core2foraws_board_info_t;
/* @[declare_core2foraws_board_info_t] */

/**
 * @brief Gets the detected hardware revision.
 *
 * The first call identifies the IMU at I2C address 0x68 on the internal bus
 * (MPU6886 WHO_AM_I or BMI270 CHIP_ID) and caches the result. The LCD
 * controller is read back by the display driver during
 * core2foraws_display_init(), so it is @ref BOARD_LCD_UNKNOWN until then or
 * when the display is not built. Thread-safe.
 *
 * **Example:**
 *
 * Log the kit model and its fitted parts.
 * @code{c}
 *  #include <esp_log.h>
 *  #include "core2foraws.h"
 *
 *  void app_main( void )
 *  {
 *      core2foraws_init();
 *
 *      core2foraws_board_info_t board;
 *      if( core2foraws_board_info_get( &board ) == ESP_OK )
 *      {
 *          ESP_LOGI( "APP", "%s: %s IMU, %s mic, %s LCD", board.model_name,
 *                    board.imu_name, board.microphone_name, board.lcd_name );
 *      }
 *  }
 * @endcode
 *
 * @param[out] info Receives the board information.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
 *  - ESP_OK              : Success; check `model` for @ref BOARD_MODEL_UNKNOWN
 *  - ESP_ERR_INVALID_ARG : @p info is `NULL`
 *  - Other               : Internal I2C bus could not be used; the result is
 *                          not cached and the next call retries
 */
/* @[declare_core2foraws_board_info_get] */
esp_err_t core2foraws_board_info_get( core2foraws_board_info_t *info );
/* @[declare_core2foraws_board_info_get] */

#ifdef __cplusplus
}
#endif
#endif
