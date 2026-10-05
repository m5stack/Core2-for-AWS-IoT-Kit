/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file core2foraws_button.h
 * @brief Core2 for AWS IoT Kit virtual button hardware driver APIs
 */

#ifndef _CORE2FORAWS_BUTTON_H_
#define _CORE2FORAWS_BUTTON_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>

  /** @brief Enumerated list of available buttons
   *
   * These are the available virtual buttons mapped to the imprinted
   * circles on the touch screen.
   *
   * **Example:**
   *
   * Register a callback to handle button events.
   * @code{c}
   *  #include <stdint.h>
   *  #include <stdbool.h>
   *  #include <esp_log.h>
   *  #include "core2foraws.h"
   *
   *  static const char *TAG = "MAIN_BUTTON_DEMO";
   *
   *  void button_callback(enum core2foraws_button_btns button, press_event_t
   * event) { switch(event) { case PRESS: ESP_LOGI(TAG, "Button %d pressed",
   * button); break; case RELEASE: ESP_LOGI(TAG, "Button %d released", button);
   *              break;
   *          case LONGPRESS:
   *              ESP_LOGI(TAG, "Button %d long pressed", button);
   *              break;
   *      }
   *  }
   *
   *  void app_main ( void )
   *  {
   *      ESP_LOGI( TAG, "\tStarting..." );
   *      core2foraws_init();
   *
   *      // Register callback for all events on left button
   *      core2foraws_button_register_callback(BUTTON_LEFT, PRESS | RELEASE |
   * LONGPRESS, button_callback);
   *  }
   * @endcode
   */
  /* @[declare_core2foraws_button_btns] */
  enum core2foraws_button_btns
  {
    BUTTON_LEFT = 0,
    BUTTON_MIDDLE,
    BUTTON_RIGHT
  };
  /* @[declare_core2foraws_button_btns] */

  /**
   * @brief List of possible virtual button press events.
   */
  /* @[declare_button_press_event_t] */
  typedef enum
  {
    PRESS = ( 1 << 0 ),     /**< @brief Virtual button was pressed. */
    RELEASE = ( 1 << 1 ),   /**< @brief Virtual button was released. */
    LONGPRESS = ( 1 << 2 ), /**< @brief Virtual button was long pressed. */
  } press_event_t;
  /* @[declare_button_press_event_t] */

  /**
   * @brief Button event callback function type.
   *
   * @param button The button that triggered the event
   * @param event The type of event that occurred
   */
  typedef void ( *button_event_cb_t )( enum core2foraws_button_btns button,
                                       press_event_t event );

  /**
   * @brief Register a callback function for button events.
   *
   * This function allows you to register callback functions that will be
   * called when specific button events occur. You can register different
   * callbacks for different combinations of buttons and events.
   *
   * **Example:**
   *
   * Register callbacks for button press and release events.
   *
   * @code{c}
   *  #include <stdint.h>
   *  #include <stdbool.h>
   *  #include <esp_log.h>
   *  #include "core2foraws.h"
   *
   *  static const char *TAG = "MAIN_BUTTON_DEMO";
   *
   *  void button_callback(enum core2foraws_button_btns button, press_event_t
   * event) { switch(event) { case PRESS: ESP_LOGI(TAG, "Button %d pressed",
   * button); break; case RELEASE: ESP_LOGI(TAG, "Button %d released", button);
   *              break;
   *          case LONGPRESS:
   *              ESP_LOGI(TAG, "Button %d long pressed", button);
   *              break;
   *      }
   *  }
   *
   *  void app_main ( void )
   *  {
   *      ESP_LOGI( TAG, "\tStarting..." );
   *      core2foraws_init();
   *
   *      // Register callback for all events on all buttons
   *      core2foraws_button_register_callback(BUTTON_LEFT, PRESS | RELEASE |
   * LONGPRESS, button_callback);
   *      core2foraws_button_register_callback(BUTTON_MIDDLE, PRESS | RELEASE |
   * LONGPRESS, button_callback);
   *      core2foraws_button_register_callback(BUTTON_RIGHT, PRESS | RELEASE |
   * LONGPRESS, button_callback);
   *  }
   * @endcode
   *
   * @param[in] button The button to register the callback for
   * @param[in] events Bitmask of events to trigger the callback (PRESS |
   * RELEASE | LONGPRESS)
   * @param[in] callback The callback function to call when events occur
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
  *  - ESP_OK              : Success
  *  - ESP_ERR_INVALID_ARG : Invalid button, event mask, or callback
  *  - ESP_FAIL            : Failed to register callback
   */
  /* @[declare_core2foraws_button_register_callback] */
  esp_err_t
  core2foraws_button_register_callback( enum core2foraws_button_btns button,
                                        press_event_t events,
                                        button_event_cb_t callback );
  /* @[declare_core2foraws_button_register_callback] */

  /**
   * @brief Unregister a callback function for button events.
   *
   * @param[in] button The button to unregister the callback for
   * @param[in] events Bitmask of events to unregister (PRESS | RELEASE |
   * LONGPRESS)
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
  *  - ESP_OK              : Success
  *  - ESP_ERR_INVALID_ARG : Invalid button or event mask
  *  - ESP_FAIL            : Failed to unregister callback
   */
  /* @[declare_core2foraws_button_unregister_callback] */
  esp_err_t
  core2foraws_button_unregister_callback( enum core2foraws_button_btns button,
                                          press_event_t events );
  /* @[declare_core2foraws_button_unregister_callback] */

  /**
   * @brief Initializes the virtual buttons using the ft6336u touch
   * controller.
   *
  * @note The core2foraws_init() calls this function when the hardware
  * feature is enabled. Repeating this function after successful
  * initialization returns ESP_OK without creating another task or mutex.
   *
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK    : Success
   *  - ESP_FAIL  : Failed to initialize over I2C bus
   */
  /* @[declare_core2foraws_button_init] */
  esp_err_t core2foraws_button_init( void );
  /* @[declare_core2foraws_button_init] */

#ifdef __cplusplus
}
#endif
#endif
