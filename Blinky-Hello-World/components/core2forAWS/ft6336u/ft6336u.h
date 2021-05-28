/**
 * @file ft6336u.h
 * @brief Functions for the FT6336U capacitive touch panel controller.
 */

#pragma once

/**
 * @brief Initializes the FT6336U over I2C.
 * 
 * @note The Core2ForAWS_Init() calls this function
 * when the hardware feature is enabled.
 * @note It creates a FreeRTOS task with the task name `FT6336Task` and installs
 * an ISR on the interrupt pin FT6336U_INTR_PIN.
 *
 * The most recent button press state is stored within the library and can
 * be queried using the functions provided by this library.
 *
 * The FreeRTOS task remains suspended and only resumes when the screen is
 * pressed (which is informed by the FT6336U using the interrupt on the
 * FT6336U_INTR_PIN). If multiple press events are stashed by the hardware, the
 * FreeRTOS task keeps retrieving them one by one at an interval of 20 ticks.
 * Once all the events are retrieved, the task suspends itself.
 */
/* @[declare_ft6336_init] */
void FT6336U_Init();
/* @[declare_ft6336_init] */

/**
 * @brief Retrieves the most recent touch data from the FT6336U.
 * 
 * **Example:**
 * 
 * Print the touch coordinates and if the screen is currently being pressed.
 * @code{c}
 *  uint16_t x, y;
 *  bool press;
 * 
 *  FT6336U_GetTouch(&x, &y, &press);
 * 
 *  printf( "X: %d, Y: %d, Pressed: %d", x, y, press);
 * 
 * @endcode
 * 
 * @param[in] x The X-coordinate of the touch input.
 * @param[in] y The Y-coordinate of the touch input.
 * @param[in] press_down The pressed state of the touch screen.
 */
/* @[declare_ft6336_gettouch] */
void FT6336U_GetTouch(uint16_t* x, uint16_t* y, bool* press_down);
/* @[declare_ft6336_gettouch] */

/**
 * @brief Retrieves the pressed state of the touch screen.
 * 
 * @return The screen pressed state.
 */
/* @[declare_ft6336_waspressed] */
bool FT6336U_WasPressed();
/* @[declare_ft6336_waspressed] */

/**
 * @brief Retrieves the X-coordinate of the screen press.
 * 
 * @return The X-coordinate of the screen press.
 */
/* @[declare_ft6336_getpressposx] */
uint16_t FT6336U_GetPressPosX();
/* @[declare_ft6336_getpressposx] */

/**
 * @brief Retrieves the Y-coordinate of the screen press.
 * 
 * @return The Y-coordinate of the screen press.
 */
/* @[declare_ft6336_getpressposy] */
uint16_t FT6336U_GetPressPosY();
/* @[declare_ft6336_getpressposy] */
