/**
 * @file core2forAWS.h
 * @brief Functions to initialize and access Core2 for AWS IoT EduKit hardware features.
 */

#pragma once
#include "axp192.h"

#if CONFIG_SOFTWARE_ILI9342C_SUPPORT
#include "freertos/semphr.h"
#include "lvgl/lvgl.h"
#include "disp_driver.h"
#include "disp_spi.h"

/**
 * @brief FreeRTOS semaphore to be used when performing any 
 * operation on the display.
 * 
 * To avoid conflicts with multiple threads attempting to
 * write to the display, take this FreeRTOS semaphore first,
 * use the [LVGL widget](https://docs.lvgl.io/7.11/widgets/index.html) 
 * API of choice, and then give the semaphore.
 * The FreeRTOS task, guiTask(), will write to the ILI9342C display 
 * controller to update the display.
 * 
 * **Example:**
 * 
 * Create a LVGL label widget, set the text of the label to "Hello World!", and 
 * align the label to the center of the screen.
 * @code{c}
 *  xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
 * 
 *  lv_obj_t * hello_label = lv_label_create(NULL, NULL);
 *  lv_label_set_text_static(hello_label, "Hello World!");
 *  lv_obj_align(hello_label, NULL, LV_ALIGN_CENTER, 0, 0);
 * 
 *  xSemaphoreGive(xGuiSemaphore);
 * @endcode
 * 
 */
/* @[declare_xguisemaphore] */
extern SemaphoreHandle_t xGuiSemaphore;
/* @[declare_xguisemaphore] */
#endif

#if CONFIG_SOFTWARE_FT6336U_SUPPORT
#include "ft6336u.h"
#endif

#if CONFIG_SOFTWARE_SK6812_SUPPORT
#include "sk6812.h"
/**
 * @brief LEDs on left side of the LED bar. For use with Core2ForAWS_Sk6812_SetSideColor().
 */
/* @[declare_sk6812_side_left] */
#define SK6812_SIDE_LEFT 0
/* @[declare_sk6812_side_left] */

/**
 * @brief LEDs on right side of the LED bar. For use with Core2ForAWS_Sk6812_SetSideColor().
 */
/* @[declare_sk6812_side_right] */
#define SK6812_SIDE_RIGHT 1
/* @[declare_sk6812_side_right] */
#endif

#if CONFIG_SOFTWARE_BUTTON_SUPPORT
#include "button.h"
#endif

#if CONFIG_SOFTWARE_MPU6886_SUPPORT
#include "mpu6886.h"
#endif

#if CONFIG_SOFTWARE_RTC_SUPPORT
#include "bm8563.h"
#endif

#if CONFIG_SOFTWARE_ATECC608_SUPPORT
#include "cryptoauthlib.h"
#include "atecc608.h"
#endif

#if CONFIG_SOFTWARE_SPEAKER_SUPPORT || CONFIG_SOFTWARE_MIC_SUPPORT
#include "driver/i2s.h"
#endif

#if CONFIG_SOFTWARE_MIC_SUPPORT
#include "microphone.h"
#endif

#if CONFIG_SOFTWARE_SPEAKER_SUPPORT
#include "speaker.h"
#endif

#if CONFIG_SOFTWARE_SDCARD_SUPPORT
#include "esp_freertos_hooks.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#endif

#if CONFIG_SOFTWARE_ILI9342C_SUPPORT || CONFIG_SOFTWARE_SDCARD_SUPPORT
/**
 * @brief The SPI2 peripheral that controls the SPI bus.
 * 
 * This is peripheral type used for initializing the SPI bus.
 * There are two peripherals connected to the SPI bus, the
 * ILI9342C display controller and the TF/SD card slot.
 * 
 */
/* @[declare_spi_host_use] */
#define SPI_HOST_USE HSPI_HOST
/* @[declare_spi_host_use] */

/**
 * @brief DMA channel for SPI bus
 * 
 * This is the channel used by the SPI bus for the ILI9342C 
 * display controller and TF/SD card slot. 
 */
/* @[declare_spi_dma_chan] */
#define SPI_DMA_CHAN 2
/* @[declare_spi_dma_chan] */
#endif

/**
 * @brief Initializes the power chip with default values, enables
 * battery charging, and initializes all enabled hardware features.
 * 
 * At minimum, this helper function initializes the AXP192 power
 * management unit (PMU) with the green LED and vibration motor
 * off, as well as initializing the following features (if enabled):
 * 1. Display — initializes the SPI bus, also powers the controller and 
 * backlight to ~50% brightness.
 * 2. The touch controller via the FT6336U.
 * 3. Virtual left, middle, right buttons using the touch controller.
 * 4. The SK6812 side LED bars.
 * 5. The 6-axis IMU via the MPU6886.
 * 6. The real-time clock via the BM8563
 * features.
 */
/* @[declare_core2foraws_init] */
void Core2ForAWS_Init();
/* @[declare_core2foraws_init] */

#if CONFIG_SOFTWARE_BUTTON_SUPPORT

/** @brief Left virtual touch button 
 * 
 * This is a virtual touch button which contains the definitions of 
 * a mapped region of the screen that corresponds to the button location 
 * and touch events from the FT6336U capacitive touch controller. When 
 * the screen is touched in the region that represents the left button, 
 * it will update the state of the button. You can use the APIs in the 
 * button library with this button as the input parameter to query the
 * button's state.
 * 
 * **Example:**
 * 
 * Print "left button pressed" when the on-screen left button is pressed.
 * @code{c}
 *  if (Button_WasPressed(button_left)) {
 *      printf("left button pressed");
 *  }
 * @endcode
*/
/* @[declare_button_left] */
extern Button_t* button_left;
/* @[declare_button_left] */

/** @brief Middle virtual touch button 
 * 
 * This is a virtual touch button which contains the definitions of 
 * a mapped region of the screen that corresponds to the button location 
 * and touch events from the FT6336U capacitive touch controller. When 
 * the screen is touched in the region that represents the middle button, 
 * it will update the state of the button. You can use the APIs in the 
 * button library with this button as the input parameter to query the
 * button's state.
 * 
 * **Example:**
 * 
 * Print "middle button pressed" when the on-screen middle button is pressed.
 * @code{c}
 *  if (Button_WasPressed(button_middle)) {
 *      printf("middle button pressed");
 *  }
 * @endcode
*/
/* @[declare_button_middle] */
extern Button_t* button_middle;
/* @[declare_button_middle] */

/** @brief Right virtual touch button 
 * 
 * This is a virtual touch button which contains the definitions of 
 * a mapped region of the screen that corresponds to the button location 
 * and touch events from the FT6336U capacitive touch controller. When 
 * the screen is touched in the region that represents the right button, 
 * it will update the state of the button. You can use the APIs in the 
 * button library with this button as the input parameter to query the
 * button's state.
 * 
 * **Example:**
 * 
 * Print "right button pressed" when the on-screen right button is 
 * pressed.
 * 
 * @code{c}
 *  if (Button_WasPressed(button_right)) {
 *      printf("right button pressed");
 *  }
 * @endcode
*/
/* @[declare_button_right] */
extern Button_t* button_right;
/* @[declare_button_right] */

/**
 * @brief Initializes the @ref button_left, @ref button_middle, @ref 
 * button_right virtual buttons using the ft6336u touch controller.
 * 
 * @note The Core2ForAWS_Init() calls this function
 * when the hardware feature is enabled.
 */
/* @[declare_core2foraws_button_init] */
void Core2ForAWS_Button_Init();
/* @[declare_core2foraws_button_init] */
#endif

/**
 * @brief Enables or disables the NS4168 speaker amplifier.
 * 
 * @note The speaker cannot be enabled at the same time
 * as the microphone since they both share a common pin (GPIO0).
 * Attempting to enable and use both at the same time will 
 * cause the device to hard fault.
 * 
 * @param[in] state Desired state of the speaker.
 * 1 to enable, 0 to disable.
 */
/* @[declare_core2foraws_speaker_enable] */
void Core2ForAWS_Speaker_Enable(uint8_t state);
/* @[declare_core2foraws_speaker_enable] */

/**
 * @brief Initializes the AXP192 Power Management Unit (PMU).
 * 
 * Used to directly initialize the AXP192 LDO2, LDO3, DC2, and DC3 
 * pins.
 * 
 * @note The Core2ForAWS_Init() calls this function with parameters 
 * to initialize the vibration motor and display backlight. If the
 * ILI9342u display hardware feature is enabled, it sets the 
 * brightness to ~55% brightness. The vibration motor is always
 * initialized in the off state.
 * 
 * @param[in] ldo2_volt low-dropout voltage of the display.
 * @param[in] ldo3_volt low-dropout voltage of the vibration
 * motor.
 * @param[in] dc2_volt voltage of the vibration motor.
 * @param[in] dc3_volt voltage of the display.
 */
/* @[declare_core2foraws_pmu_init] */
void Core2ForAWS_PMU_Init(uint16_t ldo2_volt, uint16_t ldo3_volt, uint16_t dc2_volt, uint16_t dc3_volt);
/* @[declare_core2foraws_pmu_init] */

/**
 * @brief Gets the battery voltage with the AXP192.
 * 
 * **EXAMPLE**
 * Get the current battery voltage in the variable named _battery_voltage_
 * and print the voltage to the serial output.
 * 
 * @code{c}
 *  float battery_voltage = Core2ForAWS_PMU_GetBatVolt();
 *  printf("Battery Voltage: %0.2f", battery_voltage);
 * @endcode
 * 
 * @return The voltage of the battery.
 */
/* @[declare_core2foraws_pmu_getbatvolt] */
float Core2ForAWS_PMU_GetBatVolt();
/* @[declare_core2foraws_pmu_getbatvolt] */

/**
 * @brief Gets the battery current draw with the AXP192.
 * 
 * **EXAMPLE**
 * Get the current battery current in the variable named _battery_current_
 * and print the current to the serial output.
 * 
 * @code{c}
 *  float battery_current = Core2ForAWS_PMU_GetBatCurrent();
 *  printf("Battery Current: %0.3f", battery_current);
 * @endcode
 * 
 * @return The current draw of the battery.
 */
/* @[declare_core2foraws_pmu_getbatcurrent] */
float Core2ForAWS_PMU_GetBatCurrent();
/* @[declare_core2foraws_pmu_getbatcurrent] */

#if CONFIG_SOFTWARE_ILI9342C_SUPPORT

/**
 * @brief Initializes the display.
 * 
 * You must use this to initialize the display before
 * attempting to write to the display. It initializes
 * the ILI9342C on the SPI bus and the LVGL driver.
 * 
 * @note Creates a FreeRTOS task with the task name `gui`.
 * @note The Core2ForAWS_Init() calls this function
 * when the hardware feature is enabled.
 */
/* @[declare_core2foraws_display_init] */
void Core2ForAWS_Display_Init();
/* @[declare_core2foraws_display_init] */

/**
 * @brief Sets the brightness of the display.
 * 
 * This function sets the brightness of the display
 * using the AXP192 power management unit. It does
 * not write to the display controller, so it does not
 * require taking the xGuiSemaphore mutex prior to
 * use.
 * 
 * **Example:**
 * 
 * Set the display brightness to maximum.
 * @code{c}
 *  Core2ForAWS_Display_SetBrightness(100);
 * @endcode
 * 
 * @param[in] brightness the desired brightness of the 
 * display. Accepts a value from 0 to 100.
 */
/* @[declare_core2foraws_display_setbrightness] */
void Core2ForAWS_Display_SetBrightness(uint8_t brightness);
/* @[declare_core2foraws_display_setbrightness] */
#endif

/**
 * @brief Turns on or off green LED.
 * 
 * @param[in] enable The desired state of the LED.
 * 1 to turn on, 0 to turn off.
 */
/* @[declare_core2foraws_led_enable] */
void Core2ForAWS_LED_Enable(uint8_t enable);
/* @[declare_core2foraws_led_enable] */

/**
 * @brief Sets the speed of the vibration motor.
 * 
 * @param[in] strength The desired speed of the motor
 * or strength of the vibration. 0 to turn it off, 255
 * for maximum speed.
 */
/* @[declare_core2foraws_motor_setstrength] */
void Core2ForAWS_Motor_SetStrength(uint8_t strength);
/* @[declare_core2foraws_motor_setstrength] */

#if CONFIG_SOFTWARE_SK6812_SUPPORT
/**
 * @brief Initializes the SK6812 LED bars.
 * 
 * You must use this to initialize the LED bars 
 * before attempting to use either of the LED bars.
 * 
 * @note The Core2ForAWS_Init() calls this function
 * when the hardware feature is enabled.
 */
/* @[declare_core2foraws_sk6812_init] */
void Core2ForAWS_Sk6812_Init();
/* @[declare_core2foraws_sk6812_init] */

/**
 * @brief Sets the color of a single LED in the LED bars.
 * 
 * @note Requires following with Core2ForAWS_Sk6812_Show() to write the 
 * set color to the LEDs.
 * 
 * **Example:**
 * 
 * Set the color of each of the LEDs to white one at a time every
 * second.
 * @code{c}
 *  for (uint8_t i = 0; i < 10; i++) {
 *      Core2ForAWS_Sk6812_SetColor(i, 0xffffff);
 *      Core2ForAWS_Sk6812_Show();
 *      vTaskDelay(pdMS_TO_TICKS(1000));
 *  }
 * @endcode
 * 
 * @param[in] pos LED to set.
 * Accepts a value from 0 to 9. 
 * 
 * @param[in] color Color of the LED.
 * Accepts hexadecimal (web colors). 0x000000 is black 
 * and 0xffffff is white.
 * 
 * @note You must use `Core2ForAWS_Sk6812_Show()` after 
 * this function for the setting to take effect. 
 */
/* @[declare_core2foraws_sk6812_setcolor] */
void Core2ForAWS_Sk6812_SetColor(uint16_t pos, uint32_t color);
/* @[declare_core2foraws_sk6812_setcolor] */

/**
 * @brief Sets the color of all the LEDs on one side of the LED bars.
 * 
 * @note Requires following with Core2ForAWS_Sk6812_Show() to write the 
 * set color to the LEDs.
 * 
 * **Example:**
 * 
 * Set the left LED bar to green and the right LED bar to red.
 * @code{c}
 *  Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_LEFT, 0x00ff00);
 *  Core2ForAWS_Sk6812_SetSideColor(SK6812_SIDE_RIGHT, 0xff0000);
 *  Core2ForAWS_Sk6812_Show();
 * @endcode
 * 
 * @param[in] side Side of LEDs to set.
 * Accepts `SK6812_SIDE_LEFT` or `SK6812_SIDE_RIGHT`. 
 * 
 * @param[in] color Color of LED to set.
 * Accepts hexadecimal (web colors). 0x000000 is black 
 * and 0xffffff is white.
 * 
 * @note You must use `Core2ForAWS_Sk6812_Show()` after 
 * this function for the setting to take effect. 
 */
/* @[declare_core2foraws_sk6812_setsidecolor] */
void Core2ForAWS_Sk6812_SetSideColor(uint8_t side, uint32_t color);
/* @[declare_core2foraws_sk6812_setsidecolor] */

/**
 * @brief Sets the brightness of the LED bars.
 * 
 * @note You must use `Core2ForAWS_Sk6812_Show()` after 
 * this function for the setting to take effect. 
 * 
 * **Example:**
 * 
 * Set the LEDs to full brightness.
 * @code{c}
 *  Core2ForAWS_Sk6812_SetBrightness(100);
 *  Core2ForAWS_Sk6812_Show();
 * @endcode
 * 
 * @param[in] brightness Brightness of the LED bars.
 * 0 is off, 100 is full brightness. 
 */
/* @[declare_core2foraws_sk6812_setbrightness] */
void Core2ForAWS_Sk6812_SetBrightness(uint8_t brightness);
/* @[declare_core2foraws_sk6812_setbrightness] */

/**
 * @brief Updates the LEDs in the LED bars with any new
 * values set using Core2ForAWS_Sk6812_SetColor,
 * Core2ForAWS_Sk6812_SetSideColor or Core2ForAWS_Sk6812_SetBrightness.
 * 
 * This function must be executed after setting LED bar 
 * values. This saves execution time by updating LEDs once
 * after making multiple changes instead of updating with
 * every change.
 */
/* @[declare_core2foraws_sk6812_show] */
void Core2ForAWS_Sk6812_Show();
/* @[declare_core2foraws_sk6812_show] */

/**
 * @brief Turns off the LEDs in the LED bars and removes any set color.
 * 
 * @note You must use `Core2ForAWS_Sk6812_Show()` after 
 * this function for the setting to take effect. 
 * 
 * **Example:**
 * 
 * Set the LEDs to full brightness.
 * @code{c}
 *  Core2ForAWS_Sk6812_Clear();
 *  Core2ForAWS_Sk6812_Show();
 * @endcode
 */
/* @[declare_core2foraws_sk6812_clear] */
void Core2ForAWS_Sk6812_Clear();
/* @[declare_core2foraws_sk6812_clear] */
#endif

#if CONFIG_SOFTWARE_SDCARD_SUPPORT

/**
 * @brief Initializes the SD-Card.
 * 
 * @note The SD Card must be initialized before use. The SD card 
 * and the screen uses the same SPI bus. In order to avoid 
 * conflicts with the screen, you must take the spi_mutex 
 * semaphore, then call spi_poll() before accessing the SD card. 
 * Once done, give/free the semaphore so the display can take it.
 * The display task will then continue to update the display
 * controller on the SPI bus.
 * 
 * Reading/writing to an inserted SD card can use the standard C 
 * functions fopen or fprintf to work with Espressif's virtual 
 * file drivers. Visit Espressif's virtual [file system component](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/storage/vfs.html) 
 * docs for usage.
 * 
 * The example code below, writes a file named `hello.txt` on the 
 * path `/sdcard` with the contents "Hello <<YOUR SD CARD'S 
 * PRODUCT NAME>>!":
 * 
 * **Example:**
 * @code{c}
 *  #define MOUNT_POINT "/sdcard"
 *  sdmmc_card_t* card;
 *  esp_err_t ret;
 *
 *  ret = Core2ForAWS_Sdcard_Init(MOUNT_POINT, &card);
 * 
 *  xSemaphoreTake(spi_mutex, portMAX_DELAY);
 *  spi_poll();
 *  FILE* f = fopen(MOUNT_POINT"/hello.txt", "w+")
 *  fprintf(f, "Hello %s!\n", card->cid.name);
 *  fclose(f);
 *  xSemaphoreGive(spi_mutex);
 * @endcode
 * 
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_sdcard_init] */
esp_err_t Core2ForAWS_Sdcard_Init(const char* mount, sdmmc_card_t** out_card);
/* @[declare_core2foraws_sdcard_init] */
#endif