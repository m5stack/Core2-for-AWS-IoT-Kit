/**
 * @file core2forAWS.h
 * @brief Functions to initialize and access Core2 for AWS IoT EduKit hardware features.
 */

#pragma once
#include "axp192.h"

#if CONFIG_SOFTWARE_ILI9342C_SUPPORT
#include "freertos/FreeRTOS.h"
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

#if CONFIG_SOFTWARE_EXPPORTS_SUPPORT
#include "driver/gpio.h"
#include "driver/uart.h"

/**
 * @brief The ADC pin on expansion port B.
 *
 * This maps to GPIO 36 and is capable of converting analog
 * signals to digital.
 * Read more about using [ADCs with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html).
 */
/* @[declare_port_b_adc_pin] */
#define PORT_B_ADC_PIN GPIO_NUM_36
/* @[declare_port_b_adc_pin] */

/**
 * @brief The DAC pin on expansion port B.
 *
 * This maps to GPIO 26 and is capable of converting digital signals
 * to analog.
 * Read more about using [DACs with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/dac.html).
 */
/* @[declare_port_b_dac_pin] */
#define PORT_B_DAC_PIN GPIO_NUM_26
/* @[declare_port_b_dac_pin] */

/**
 * @brief The UART transmission pin on expansion port C.
 *
 * This maps to GPIO 14 and can be used as the transmission channel for
 * UART communication.
 * Read more about [UART communications with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html).
 */
/* @[declare_port_c_uart_tx_pin] */
#define PORT_C_UART_TX_PIN GPIO_NUM_14
/* @[declare_port_c_uart_tx_pin] */

/**
 * @brief The UART receiver pin on expansion port C.
 *
 * This maps to GPIO 13 and can be used as the receiver channel for
 * UART communication.
 * Read more about [UART communications with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html).
 */
/* @[declare_port_c_uart_rx_pin] */
#define PORT_C_UART_RX_PIN GPIO_NUM_13
/* @[declare_port_c_uart_rx_pin] */

/**
 * @brief The default UART controller used for expansion port C.
 *
 * This maps to UART controller UART2.
 * Read more about [UART communications with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html).
 */
/* @[declare_port_c_uart_num] */
#define PORT_C_UART_NUM UART_NUM_2
/* @[declare_port_c_uart_num] */

/**
 * @brief The default UART RX ring buffer size.
 *
 * This is the default size of the UART interface's receiver ring
 * buffer.
 * Read more about [UART communications with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html).
 */
/* @[declare_uart_rx_buf_size] */
#define UART_RX_BUF_SIZE 2048
/* @[declare_uart_rx_buf_size] */

/**
 * @brief Modes supported by the BSP for the GPIO pins.
 *
 * These are the modes supported for the GPIO pins by the BSP.
 * Read more about [UART communications with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html).
 */
/* @[declare_pin_mode_t] */
typedef enum {
    NONE,   /**< @brief Reset GPIO to default state. */
    OUTPUT, /**< @brief Set GPIO to output mode. */
    INPUT,  /**< @brief Set GPIO to input mode. */
    ADC,    /**< @brief Enable ADC mode. Only available on GPIO 36 */
    DAC,    /**< @brief Enable DAC mode. Only available on GPIO 26 */
    UART    /**< @brief Enable UART RX/TX mode. UART TX only available
                        on GPIO 14 and UART RX is only available on GPIO 13.
                        Only supports full-duplex UART so setting one pin to
                        UART mode will also set the other pin to UART mode.*/
} pin_mode_t;
/* @[declare_pin_mode_t] */
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
void Core2ForAWS_Init(void);
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
void Core2ForAWS_Button_Init(void);
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
float Core2ForAWS_PMU_GetBatVolt(void);
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
float Core2ForAWS_PMU_GetBatCurrent(void);
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
void Core2ForAWS_Display_Init(void);
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
void Core2ForAWS_Sk6812_Init(void);
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
void Core2ForAWS_Sk6812_Show(void);
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
void Core2ForAWS_Sk6812_Clear(void);
/* @[declare_core2foraws_sk6812_clear] */
#endif

#if CONFIG_SOFTWARE_SDCARD_SUPPORT

/**
 * @brief Initializes and mounts the SD-Card.
 *
 * @note The SD Card must be mounted before use. The SD card
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
 * The example code below, mounts the SD card and then writes a
 * file named `hello.txt` on the path `/sdcard` with the contents
 * "Hello <<YOUR SD CARD'S PRODUCT NAME>>!":
 *
 * **Example:**
 * @code{c}
 *  #define MOUNT_POINT "/sdcard"
 *  sdmmc_card_t* card;
 *  esp_err_t err;
 *
 *  xSemaphoreTake(spi_mutex, portMAX_DELAY);
 *
 *  spi_poll();
 *
 *  err = Core2ForAWS_Sdcard_Mount(MOUNT_POINT, &card);
 *
 *  FILE* f = fopen(MOUNT_POINT"/hello.txt", "w+")
 *  fprintf(f, "Hello %s!\n", card->cid.name);
 *  fclose(f);
 *
 *  xSemaphoreGive(spi_mutex);
 * @endcode
 *
 * @param[in] mount_path The path where partition should be registered.
 * @param[out] out_card The pointer to the card information structure. Hold this handle and use it to unmount the card later if needed.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_sdcard_mount] */
esp_err_t Core2ForAWS_SDcard_Mount(const char *mount_path, sdmmc_card_t **out_card);
/* @[declare_core2foraws_sdcard_mount] */

/**
 * @brief Removes the FAT partition and unmounts the SD-Card.
 *
 * @note The SD Card must be mounted before use. The SD card
 * and the screen uses the same SPI bus. In order to avoid
 * conflicts with the screen, you must take the spi_mutex
 * semaphore, then call spi_poll() before accessing the SD card.
 * Once done, give/free the semaphore so the display can take it.
 * The display task will then continue to update the display
 * controller on the SPI bus.
 *
 * To learn more about using the SD card, visit Espressif's virtual
 * [file system component](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/storage/vfs.html)
 * docs for usage.
 *
 * The example code below, mounts the SD card, writes a file named
 * `hello.txt` on the path `/sdcard` with the contents "Hello
 * <<YOUR SD CARD'S PRODUCT NAME>>!", and then unmounts the card:
 *
 * **Example:**
 * @code{c}
 *  #define MOUNT_POINT "/sdcard"
 *  sdmmc_card_t* card;
 *  esp_err_t err;
 *
 *  xSemaphoreTake(spi_mutex, portMAX_DELAY);
 *
 *  spi_poll();
 *
 *  err = Core2ForAWS_Sdcard_Mount(MOUNT_POINT, &card);
 *
 *  FILE* f = fopen(MOUNT_POINT"/hello.txt", "w+")
 *  fprintf(f, "Hello %s!\n", card->cid.name);
 *  fclose(f);
 *
 *  err = Core2ForAWS_Sdcard_Unmount(MOUNT_POINT, &card);
 *  xSemaphoreGive(spi_mutex);
 * @endcode
 *
 * @param[in] mount_path The path where partition should be registered.
 * @param[out] out_card The card information structure.
 *
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_sdcard_unmount] */
esp_err_t Core2ForAWS_SDcard_Unmount(const char *mount_path, sdmmc_card_t *out_card);
/* @[declare_core2foraws_sdcard_unmount] */
#endif

#if CONFIG_SOFTWARE_EXPPORTS_SUPPORT
/**
 * @brief Sets the pin mode of the specified pin.
 *
 * This function sets the mode of the pin. The available modes
 * are defined by pin_mode_t. All pins support digital input/ouput
 * of high/low. Only specified pins support ADC, DAC, or UART. View
 * the hardware schematic or [pinmap](https://docs.m5stack.com/en/core/core2_for_aws?id=pinmap)
 * for pin features.
 *
 * **ADC** sets the pin to analog single read mode. Single read mode is
 * suitable for low-frequency sampling operations. Must use
 * PORT_B_ADC_PIN (same as GPIO_NUM_36) since it is the only
 * expansion port pin that is ADC capable.
 *
 * **DAC** allows the pin to be set to specified voltages. Must use
 * PORT_B_DAC_PIN (same as GPIO_NUM_26) since it is the only
 * expansion port pin that is DAC capable.
 *
 * **UART** enables full-duplex Universal Asynchronous Receiver/
 * Transmitter (UART) serial communications, utilizing both
 * PORT_C_UART_TX_PIN and PORT_C_UART_RX_PIN for transmiting and
 * receiving data. This function should only be called once for UART
 * since setting one pin to UART will automatically set the other. Must
 * use PORT_C_UART_TX_PIN (same as GPIO_NUM_14) as the UART transmit
 * pin since it is the only expansion port pin that is UART TX
 * capable. Must use PORT_C_UART_RX_PIN (same as GPIO_NUM_13) as the
 * UART receive pin since it is the only expansion port pin that
 * is UART RX capable.
 *
 * The example code below sets the ADC pin (GPIO 36) to ADC mode after
 * initializing the Core2 for AWS IoT EduKit and prints out the
 * result of attempting to set the pin.
 *
 * **Example:**
 * @code{c}
 *  #include <stdio.h>
 *  #include "esp_log.h"
 *
 *  #include "core2forAWS.h"
 *
 *  static const char *TAG = "SET_PINMODE_DEMO";
 *
 *  void app_main(void){
 *      Core2ForAWS_Init();
 *      esp_err_t err = Core2ForAWS_Port_PinMode(GPIO_NUM_36, ADC);
 *      ESP_LOGI(TAG, "Status of setting the pin: %d", err);
 *  }
 * @endcode
 *
 * @param[in] pin The GPIO pin to set the mode of.
 * @param[in] mode The mode to set the GPIO pin.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 *
 */
/* @[declare_core2foraws_port_pinmode] */
esp_err_t Core2ForAWS_Port_PinMode(gpio_num_t pin, pin_mode_t mode);
/* @[declare_core2foraws_port_pinmode] */

/**
 * @brief Reads the value from a specified digital pin that's set as an
 * input — either 1 (high) or 0 (low).
 *
 * @note pin_mode_t for a GPIO pin must be set to INPUT before using
 * Core2ForAWS_Port_Read for that pin.
 * @note Reads high when connected to Vcc.
 *
 * This function reads if the specified pin is high (1) or low (0). In
 * order for the pin to read high, the connected peripheral must use the
 * available ground and Vcc pins and not an external power source.
 *
 * The example code sets GPIO 26 (same as PORT_B_DAC_PIN) as an input,
 * creates a FreeRTOS task which periodically (1s) reads from GPIO 26
 * to see if a compatible peripheral is connected. The task prints the
 * returned value to the serial monitor.
 *
 * **Example:**
 * @code{c}
 *  #include <stdio.h>
 *  #include "freertos/FreeRTOS.h"
 *  #include "freertos/task.h"
 *  #include "esp_log.h"
 *
 *  #include "core2forAWS.h"
 *
 *  static const char *TAG = "INPUT_READ_DEMO";
 *
 *  void pin_read_task(){
 *      for(;;){
 *          ESP_LOGI(TAG, "Sensor plugged in (0=no / 1=yes): %d", Core2ForAWS_Port_Read(GPIO_NUM_26));
 *          vTaskDelay(pdMS_TO_TICKS(1000));
 *      }
 *  }
 *
 *  void app_main(void){
 *      Core2ForAWS_Init();
 *      Core2ForAWS_Port_PinMode(GPIO_NUM_26, INPUT);
 *      xTaskCreatePinnedToCore(pin_read_task, "read_pin", 1024*4, NULL, 1, NULL, 1);
 *  }
 * @endcode
 *
 * @param[in] pin The GPIO pin to read.
 * @return 1 for high, 0 for low.
 */
/* @[declare_core2foraws_port_read] */
bool Core2ForAWS_Port_Read(gpio_num_t pin);
/* @[declare_core2foraws_port_read] */

/**
 * @brief Writes the specified level (0 or 1) to the specified digital
 * pin.
 *
 * @note pin_mode_t for a GPIO pin must be set to OUTPUT before using
 * Core2ForAWS_Port_Write for that pin.
 *
 * This function sets the specified pin to either high(1) or low(0)
 * digital level.
 *
 * The example code sets the GPIO 36 (same as PORT_B_ADC_PIN) pin mode
 * to OUTPUT, creates a FreeRTOS task which periodically (1s) toggles
 * GPIO 36 pin between high and low and prints the output level to
 * the serial monitor.
 *
 * **Example:**
 * @code{c}
 *  #include <stdio.h>
 *  #include "freertos/FreeRTOS.h"
 *  #include "freertos/task.h"
 *  #include "esp_log.h"
 *
 *  #include "core2forAWS.h"
 *
 *  static const char *TAG = "DIGITAL_WRITE_DEMO";
 *
 *  void pin_write_task(){
 *      bool output = 0;
 *      for(;;){
 *          Core2ForAWS_Port_Write(GPIO_NUM_36, output);
 *          ESP_LOGI(TAG, "Output on GPIO %d: %s", GPIO_NUM_36, output ? "HIGH" : "LOW");
 *
 *          output = !output;
 *
 *          vTaskDelay(pdMS_TO_TICKS(1000));
 *      }
 *  }
 *
 *  void app_main(void){
 *      Core2ForAWS_Init();
 *      Core2ForAWS_Port_PinMode(GPIO_NUM_36, OUTPUT);
 *      xTaskCreatePinnedToCore(pin_write_task, "write_pin", 1024*4, NULL, 1, NULL, 1);
 *  }
 * @endcode
 *
 * @param[in] pin The GPIO pin to set the digital output level of.
 * @param[in] level The digital out level of the pin (low = 0, high = 1).
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_port_write] */
esp_err_t Core2ForAWS_Port_Write(gpio_num_t pin, bool level);
/* @[declare_core2foraws_port_write] */

/**
 * @brief Read the raw ADC value from GPIO36.
 *
 * @note Uses the etched eFuse VRef calibration.
 * @note pin_mode_t for PORT_B_ADC_PIN must be set to ADC before using
 * Core2ForAWS_Port_B_ADC_ReadRaw.
 *
 * This function reads the raw ADC value from Port B's
 * Analog-to-Digital-Converter (ADC) on GPIO36. GPIO36 is the only pin on
 * the external ports with access to the ADC.
 * Read more about using [ADCs with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html).
 *
 * The example code sets the ADC pin mode for the PORT_B_ADC_PIN, creates 
 * a FreeRTOS task which periodically (1s) reads the raw ADC value of the 
 * [M5Stack EARTH](https://shop.m5stack.com/products/earth-sensor-unit) 
 * moisture sensor connected on Port B. It prints out the value to the 
 * serial output.
 *
 * **Example:**
 * @code{c}
 *  #include <stdio.h>
 *  #include "freertos/FreeRTOS.h"
 *  #include "freertos/task.h"
 *  #include "esp_log.h"
 *
 *  #include "core2forAWS.h"
 *
 *  static const char *TAG = "RAW_ADC_READ_DEMO";
 *
 *  void read_moisture_task(){
 *      for(;;){
 *          ESP_LOGI(TAG, "Moisture ADC raw: %d", Core2ForAWS_Port_B_ADC_ReadRaw()); *
 *          vTaskDelay(pdMS_TO_TICKS(1000));
 *      }
 *  }
 *
 *  void app_main(void){
 *      Core2ForAWS_Init();
 *      Core2ForAWS_Port_PinMode(PORT_B_ADC_PIN, ADC);
 *      xTaskCreatePinnedToCore(read_moisture_task, "moisture_raw", 4096*2, NULL, 1, NULL, 1);
 *  }
 *
 * @return the raw ADC reading.
 *
 * @endcode
 */
/* @[declare_core2foraws_port_b_adc_readraw] */
uint32_t Core2ForAWS_Port_B_ADC_ReadRaw(void);
/* @[declare_core2foraws_port_b_adc_readraw] */

/**
 * @brief Read the calibrated ADC voltage from GPIO36.
 *
 * @note Uses the etched eFuse VRef calibration.
 * @note pin_mode_t for PORT_B_ADC_PIN must be set to ADC before using
 * Core2ForAWS_Port_B_ADC_ReadMilliVolts.
 *
 * This function reads the raw ADC value from Port B's
 * Analog-to-Digital-Converter (ADC) on GPIO36 and converts it to
 * millivolts using the VRef calibration. GPIO36 is the only pin on
 * the external ports with access to the ADC.
 * Read more about using [ADCs with the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html).
 *
 * The example code sets the ADC pin mode for the PORT_B_ADC_PIN, creates 
 * a FreeRTOS task which periodically (1s) reads the ADC voltage of the 
 * [M5Stack EARTH](https://shop.m5stack.com/products/earth-sensor-unit) 
 * moisture sensor connected on Port B. It prints out the value to the 
 * serial output.
 *
 * **Example:**
 * @code{c}
 *  #include <stdio.h>
 *  #include "freertos/FreeRTOS.h"
 *  #include "freertos/task.h"
 *  #include "esp_log.h"
 *
 *  #include "core2forAWS.h"
 *
 *  static const char *TAG = "ADC_VOLTAGE_READ_DEMO";
 *
 *  void read_moisture_task(){
 *      for(;;){
 *          ESP_LOGI(TAG, "Moisture ADC converted to voltage: %d", Core2ForAWS_Port_B_ADC_ReadMilliVolts());
 *          vTaskDelay(pdMS_TO_TICKS(1000));
 *      }
 *  }
 *
 *  void app_main(void){
 *      Core2ForAWS_Init();
 *      Core2ForAWS_Port_PinMode(PORT_B_ADC_PIN, ADC);
 *      xTaskCreatePinnedToCore(read_moisture_task, "moisture_voltage", 1024*4, NULL, 1, NULL, 1);
 *  }
 * @endcode
 *
 * @return the voltage reading from the ADC in millivolts.
 */
/* @[declare_core2foraws_port_b_adc_readmillivolts] */
uint32_t Core2ForAWS_Port_B_ADC_ReadMilliVolts(void);
/* @[declare_core2foraws_port_b_adc_readmillivolts] */

/**
 * @brief Outputs the specified voltage (millivolts) to the DAC.
 *
 * @note pin_mode_t for PORT_B_DAC_PIN must be set to DAC before using
 * Core2ForAWS_Port_B_DAC_WriteMilliVolts.
 *
 * This function accepts a range from 0 to a maximum of VDD3P3_RTC
 * (~3300mV).
 *
 * The example code sets the PORT_B_DAC_PIN (GPIO 26) pin mode
 * to DAC and outputs 1,500mV.
 *
 * **Example:**
 * @code{c}
 *  #include <stdio.h>
 *  #include "freertos/FreeRTOS.h"
 *  #include "freertos/task.h"
 *  #include "esp_log.h"
 *
 *  #include "core2forAWS.h"
 *
 *  void app_main(void){
 *      Core2ForAWS_Init();
 *      Core2ForAWS_Port_PinMode(PORT_B_DAC_PIN, DAC);
 *      Core2ForAWS_Port_B_DAC_WriteMilliVolts(1500);
 *  }
 * @endcode
 *
 * @param[in] mvolts The voltage level to set the DAC pin in millivolts.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_port_b_dac_writemillivolts] */
esp_err_t Core2ForAWS_Port_B_DAC_WriteMilliVolts(uint16_t mvolts);
/* @[declare_core2foraws_port_b_dac_writemillivolts] */

/**
 * @brief Configures the UART interface at the specified baud rate.
 *
 * This is a helper function to simplify building applications
 * with UART. It preconfigures the UART communications with word length
 * of 8 bits, no parity bit, 1 bit for the number of stop bits, and
 * hardware flow control disabled. It sets the baud rate to the user
 * defined value and starts reading from the UART channel 2 and storing
 * data into the ring buffer of length UART_RX_BUF_SIZE.
 *
 * Usage of the UART convenience methods provided in this BSP aims to
 * simplify development at the expense of compatibility and performance.
 * The configuration above may not be suitable for your application or
 * attached peripheral. For more information about UART communications
 * on the Core2 for AWS IoT EduKit using the ESP32 and how to create
 * your own configuration, visit Espressif's official [documentation](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/peripherals/uart.html).
 *
 * The example below sets the PORT_C_UART_TX_PIN (GPIO 14) pin mode
 * to UART transmit, which will also set PORT_C_UART_RX_PIN (GPIO 13)
 * to UART receiver, and sets the UART baud rate to 115200. It then
 * starts two FreeRTOS tasks — one that transmits "Hello from AWS IoT
 * EduKit" every two seconds and the other receives the message. The
 * receiver task outputs the number of bytes read from the buffer, the
 * number of bytes that remains unread in the ring buffer, and the
 * message.
 *
 * To receive the messages transmitted on the same device, run a
 * female-female jumper wire from Port C's TX pin to PORT C's RX pin.
 *
 * **Example:**
 * @code{c}
 *  #include "freertos/FreeRTOS.h"
 *  #include "freertos/task.h"
 *  #include "driver/uart.h"
 *  #include "driver/gpio.h"
 *  #include "esp_log.h"
 *
 *  #include "core2forAWS.h"
 *
 *  static const char *TAG = "UART_DEMO";
 *
 *  static void uart_tx_task(void *arg){
 *      while (1) {
 *          const char *message = "Hello from AWS IoT EduKit";
 *          size_t message_len = strlen(message) + 1;
 *          Core2ForAWS_Port_C_UART_Send(message, message_len);
 *          vTaskDelay(pdMS_TO_TICKS(2000));
 *      }
 *  }
 *
 *  static void uart_rx_task(void *arg){
 *      int rxBytes;
 *      uint8_t *data = heap_caps_malloc(UART_RX_BUF_SIZE, MALLOC_CAP_SPIRAM); // Allocate space for message in external RAM
 *      while (1) {
 *          rxBytes = Core2ForAWS_Port_C_UART_Receive(data);
 *          if (rxBytes > 0) {
 *              ESP_LOGI(TAG, "Read %d bytes from UART. Received: '%s'", rxBytes, data);
 *          }
 *          vTaskDelay(pdMS_TO_TICKS(100)); // Read more frequently than transmit to ensure the messages are not erased from buffer.
 *      }
 *      free(data); // Free memory from external RAM
 *  }
 *
 *  void app_main(void){
 *      Core2ForAWS_Init();
 *      esp_err_t err = Core2ForAWS_Port_PinMode(PORT_C_UART_TX_PIN, UART);
 *      if (err == ESP_OK){
 *          Core2ForAWS_Port_C_UART_Begin(115200);
 *          xTaskCreate(uart_rx_task, "uart_rx", 1024*2, NULL, configMAX_PRIORITIES, NULL);
 *          xTaskCreate(uart_tx_task, "uart_tx", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
 *      }
 *  }
 * @endcode
 *
 * @param[in] baud The baud rate (bits per second) of the UART2 channel.
 * @return [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/system/esp_err.html#macros). 0 or `ESP_OK` if successful.
 */
/* @[declare_core2foraws_port_c_uart_begin] */
esp_err_t Core2ForAWS_Port_C_UART_Begin(uint32_t baud);
/* @[declare_core2foraws_port_c_uart_begin] */

/**
 * @brief Sends a specified length of a user defined message over UART2.
 *
 * This is a helper function to simplify transmitting data over UART
 * using the UART channel 2. This function does not return until all
 * bytes are either transferred from the ring buffer to the hardware
 * FIFO buffer, or already sent.
 *
 * For more information about UART communications on the Core2 for AWS
 * IoT EduKit using the ESP32, visit Espressif's official
 * [documentation](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/peripherals/uart.html).
 *
 * The example below sets the PORT_C_UART_TX_PIN (GPIO 14) pin mode
 * to UART transmit, which will also set PORT_C_UART_RX_PIN (GPIO 13)
 * to UART receiver, and sets the UART baud rate to 115200. It then
 * starts two FreeRTOS tasks — one that transmits "Hello from AWS IoT
 * EduKit" every two seconds and the other receives the message. The
 * receiver task outputs the number of bytes read from the buffer, the
 * number of bytes that remains unread in the ring buffer, and the
 * message.
 *
 * To receive the messages transmitted on the same device, run a
 * female-female jumper wire from Port C's TX pin to PORT C's RX pin.
 *
 * **Example:**
 * @code{c}
 *  #include "freertos/FreeRTOS.h"
 *  #include "freertos/task.h"
 *  #include "driver/uart.h"
 *  #include "driver/gpio.h"
 *  #include "esp_log.h"
 *
 *  #include "core2forAWS.h"
 *
 *  static const char *TAG = "UART_DEMO";
 *
 *  static void uart_tx_task(void *arg){
 *      while (1) {
 *          const char *message = "Hello from AWS IoT EduKit";
 *          size_t message_len = strlen(message) + 1;
 *          Core2ForAWS_Port_C_UART_Send(message, message_len);
 *          vTaskDelay(pdMS_TO_TICKS(2000));
 *      }
 *  }
 *
 *  static void uart_rx_task(void *arg){
 *      int rxBytes;
 *      uint8_t *data = heap_caps_malloc(UART_RX_BUF_SIZE, MALLOC_CAP_SPIRAM); // Allocate space for message in external RAM
 *      while (1) {
 *          rxBytes = Core2ForAWS_Port_C_UART_Receive(data);
 *          if (rxBytes > 0) {
 *              ESP_LOGI(TAG, "Read %d bytes from UART. Received: '%s'", rxBytes, data);
 *          }
 *          vTaskDelay(pdMS_TO_TICKS(100)); // Read more frequently than transmit to ensure the messages are not erased from buffer.
 *      }
 *      free(data); // Free memory from external RAM
 *  }
 *
 *  void app_main(void){
 *      Core2ForAWS_Init();
 *      esp_err_t err = Core2ForAWS_Port_PinMode(PORT_C_UART_TX_PIN, UART);
 *      if (err == ESP_OK){
 *          Core2ForAWS_Port_C_UART_Begin(115200);
 *          xTaskCreate(uart_rx_task, "uart_rx", 1024*2, NULL, configMAX_PRIORITIES, NULL);
 *          xTaskCreate(uart_tx_task, "uart_tx", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
 *      }
 *  }
 * @endcode
 *
 * @param[in] message A pointer to the message to be transmitted over
 * UART2.
 * @param[in] len The size of the message that you want to transmit.
 * @return The 0 or greater number of bytes pushed to the TX
 * FIFO. A return value -1 represents an error in one of the provided
 * parameters.
 */
/* @[declare_core2foraws_port_c_uart_send] */
int Core2ForAWS_Port_C_UART_Send(const char *message, size_t len);
/* @[declare_core2foraws_port_c_uart_send] */

/**
 * @brief Reads messages in the UART2 receiver ring buffer.
 *
 * This is a helper function to simplify development with UART. This
 * function is used to read messages stored in the receiver hardware
 * FIFO buffer, which was configured to a maximum of UART_RX_BUF_SIZE.
 *
 * This convenience methods provided in this BSP aims to
 * simplify development at the expense of compatibility and performance.
 * The usage of this function requires regular polling for any new
 * messages. For some applications, it might offer significant
 * performance benefits to set up an interrupt for handling and
 * processing new data. For more information about UART communications
 * on the Core2 for AWS IoT EduKit using the ESP32 and how to create
 * your own configuration, visit Espressif's official [documentation](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/peripherals/uart.html).
 *
 * The example below sets the PORT_C_UART_TX_PIN (GPIO 14) pin mode
 * to UART transmit, which will also set PORT_C_UART_RX_PIN (GPIO 13)
 * to UART receiver, and sets the UART baud rate to 115200. It then
 * starts two FreeRTOS tasks — one that transmits "Hello from AWS IoT
 * EduKit" every two seconds and the other receives the message. The
 * receiver task outputs the number of bytes read from the buffer, the
 * number of bytes that remains unread in the ring buffer, and the
 * message.
 *
 * To receive the messages transmitted on the same device, run a
 * female-female jumper wire from Port C's TX pin to PORT C's RX pin.
 *
 * **Example:**
 * @code{c}
 *  #include "freertos/FreeRTOS.h"
 *  #include "freertos/task.h"
 *  #include "driver/uart.h"
 *  #include "driver/gpio.h"
 *  #include "esp_log.h"
 *
 *  #include "core2forAWS.h"
 *
 *  static const char *TAG = "UART_DEMO";
 *
 *  static void uart_tx_task(void *arg){
 *      while (1) {
 *          const char *message = "Hello from AWS IoT EduKit";
 *          size_t message_len = strlen(message) + 1;
 *          Core2ForAWS_Port_C_UART_Send(message, message_len);
 *          vTaskDelay(pdMS_TO_TICKS(2000));
 *      }
 *  }
 *
 *  static void uart_rx_task(void *arg){
 *      int rxBytes;
 *      uint8_t *data = heap_caps_malloc(UART_RX_BUF_SIZE, MALLOC_CAP_SPIRAM); // Allocate space for message in external RAM
 *      while (1) {
 *          rxBytes = Core2ForAWS_Port_C_UART_Receive(data);
 *          if (rxBytes > 0) {
 *              ESP_LOGI(TAG, "Read %d bytes from UART. Received: '%s'", rxBytes, data);
 *          }
 *          vTaskDelay(pdMS_TO_TICKS(100)); // Read more frequently than transmit to ensure the messages are not erased from buffer.
 *      }
 *      free(data); // Free memory from external RAM
 *  }
 *
 *  void app_main(void){
 *      Core2ForAWS_Init();
 *      esp_err_t err = Core2ForAWS_Port_PinMode(PORT_C_UART_TX_PIN, UART);
 *      if (err == ESP_OK){
 *          Core2ForAWS_Port_C_UART_Begin(115200);
 *          xTaskCreate(uart_rx_task, "uart_rx", 1024*2, NULL, configMAX_PRIORITIES, NULL);
 *          xTaskCreate(uart_tx_task, "uart_tx", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
 *      }
 *  }
 * @endcode
 *
 * @param[out] message_buffer The buffer which will contain the message
 * read from the receiver hardware FIFO buffer. At maximum, it will be
 * of size UART_RX_BUF_SIZE.
 * @return The number of bytes read from the receiver FIFO buffer.
 */
/* @[declare_core2foraws_port_c_uart_receive] */
int Core2ForAWS_Port_C_UART_Receive(uint8_t *message_buffer);
/* @[declare_core2foraws_port_c_uart_receive] */
#endif