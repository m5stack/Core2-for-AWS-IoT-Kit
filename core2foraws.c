/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws.c
 * @brief Core2 for AWS IoT Kit general hardware driver APIs
 */

#include <esp_log.h>

#include "core2foraws.h"

static const char *_TAG = "CORE2FORAWS";

esp_err_t core2foraws_init( void )
{
  ESP_LOGI( _TAG, "\tInitializing" );

#ifndef CONFIG_SOFTWARE_BSP_SUPPORT
  ESP_LOGW( _TAG, "\tHardware features are disabled; nothing to initialize" );
  return ESP_OK;
#else
  esp_err_t err = ESP_FAIL;
  esp_err_t ret = ESP_OK;

  /* The internal I2C bus (GPIO21/GPIO22) is the foundation for the AXP192
   * PMU, BM8563 RTC, FT6336 touch controller, MPU6886 or BMI270 IMU, and ATECC608
   * secure element. It must come up before any of those peripherals. */
  err = core2foraws_i2c_init( CORE2FORAWS_I2C_INTERNAL );
  if( err != ESP_OK )
  {
    ESP_LOGE( _TAG, "\tError initializing internal I2C bus. Error 0x%x", err );
    return err;
  }

  /* The AXP192 PMU must be initialized second: it brings up the peripheral
   * rails (display logic/SD on LDO2, backlight on DCDC3), enables the 5V
   * boost bus that powers the SK6812 RGB LEDs, drives the speaker-amp
   * enable, and performs the LCD/touch reset pulse on AXP192 GPIO4. The
   * display and RGB LED drivers therefore depend on this step. */
  err = core2foraws_power_init();
  if( err != ESP_OK )
  {
    ESP_LOGE( _TAG, "\tError initializing power. Error 0x%x", err );
    return err;
  }

  /* Internal I2C sensors. All share the bus brought up above. They run while
   * the LCD and touch controllers boot after the reset released by
   * power_init, so their init time overlaps that start-up window. */
#ifdef CONFIG_SOFTWARE_MOTION_SUPPORT
  err = core2foraws_motion_init();
  if( err != ESP_OK )
    ESP_LOGE( _TAG, "\tError initializing motion sensor. Error 0x%x", err );
  ret |= err;
#endif

#ifdef CONFIG_SOFTWARE_RTC_SUPPORT
  err = core2foraws_rtc_init();
  if( err != ESP_OK )
    ESP_LOGE( _TAG, "\tError initializing real time clock. Error 0x%x", err );
  ret |= err;
#endif

  /* ATECC608 secure element. It shares the internal I2C bus and has
   * non-standard wake/sleep timing, so it is initialized after the other
   * fixed I2C sensors. */
#ifdef CONFIG_SOFTWARE_CRYPTO_SUPPORT
  err = core2foraws_crypto_init();
  if( err != ESP_OK )
    ESP_LOGE( _TAG,
              "\tError initializing secure element (crypto chip). Error 0x%x",
              err );
  ret |= err;
#endif

  /* Display (ILI9342C/E LCD + FT6336 touch). Depends on the AXP192 having
   * raised the LCD logic/backlight rails and released the LCD/touch reset
   * line (AXP192 GPIO4), and on the internal I2C bus for the touch
   * controller. It waits out whatever remains of the controllers' start-up
   * time. */
#ifdef CONFIG_SOFTWARE_DISPLAY_SUPPORT
  err = core2foraws_display_init();
  if( err != ESP_OK )
    ESP_LOGE( _TAG, "\tError initializing display. Error 0x%x", err );
  ret |= err;
#endif

  /* Virtual touch buttons. These read the FT6336 touch controller, so the
   * display/touch stack must already be initialized. */
#ifdef CONFIG_SOFTWARE_BUTTON_SUPPORT
  err = core2foraws_button_init();
  if( err != ESP_OK )
    ESP_LOGE( _TAG, "\tError initializing button. Error 0x%x", err );
  ret |= err;
#endif

  /* SK6812 RGB LED chain (add-on board, GPIO25). Driven from the 5V boost
   * bus enabled during power_init, so it must come after the PMU. */
#ifdef CONFIG_SOFTWARE_RGB_LED_SUPPORT
  err = core2foraws_rgb_led_init();
  if( err != ESP_OK )
    ESP_LOGE( _TAG, "\tError initializing rgb leds. Error 0x%x", err );
  ret |= err;
#endif

  /* Wi-Fi / BLE provisioning helper. This is independent of the board power
   * sequencing and on-board peripherals, so it is initialized last. */
#ifdef CONFIG_SOFTWARE_WIFI_SUPPORT
  err = core2foraws_wifi_init();
  if( err != ESP_OK )
    ESP_LOGE( _TAG,
              "\tError initializing Wi-Fi provisioning over BLE. Error 0x%x",
              err );
  ret |= err;
#endif

  return core2foraws_common_error( ret );
#endif
}