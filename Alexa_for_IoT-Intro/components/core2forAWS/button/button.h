/**
 * @file button.h
 * @brief Functions for the virtual buttons using the FT633U touch controller.
 */

#pragma once

#include "stdio.h"

/**
 * @brief List of possible virtual button press events.
 */
/* @[declare_button_pressevent] */
typedef enum {
    PRESS = (1 << 0),       /**< @brief Virtual button was pressed. */
    RELEASE = (1 << 1),     /**< @brief Virtual button was released. */
    LONGPRESS = (1 << 2),   /**< @brief Virtual button was long pressed. */
} PressEvent;
/* @[declare_button_pressevent] */

/**
 * @brief The member defintions of the virtual button data.
 * 
 * This structure can be used to define virtual buttons bounded by rectangular 
 * areas of the touch screen.
 */
/* @[declare_button_button_t] */
typedef struct _Button_t  {
    uint16_t x;                 /**< @brief Virtual button touch starting point in the X-coordinate plane. */
    uint16_t y;                 /**< @brief Virtual button touch starting point in the Y-coordinate plane */
    uint16_t w;                 /**< @brief Virtual button touched width from the starting point in the X-coordinate plane. */
    uint16_t h;                 /**< @brief Virtual button touched height from the starting point in the y-coordinate plane. */

    uint8_t value;              /**< @brief Current virtual button touched state */
    uint8_t last_value;         /**< @brief Previous virtual button touched state */
    uint32_t last_press_time;   /**< @brief FreeRTOS ticks when virtual button was last touched */
    uint32_t long_press_time;   /**< @brief Number of FreeRTOS ticks to elapse to consider holding the touch button a long press */
    uint8_t state;              /**< @brief The button press event */
    struct _Button_t* next;     /**< @brief Pointer to the next virtual button */
} Button_t;
/* @[declare_button_button_t] */

/**
 * @brief Initializes the virtual buttons using the FT6336U touch controller.
 * 
 * @note The Core2ForAWS_Init() calls this function
 * when the hardware feature is enabled.
 */
/* @[declare_button_init] */
void Button_Init();
/* @[declare_button_init] */

/**
 * @brief Create a virtual button based on locations and size of the 
 * button area. The Core2 for AWS IoT EduKit has 3 buttons on the 
 * face of the device that these virtual buttons are mapped to. 
 * 
 * @param[in] x Virtual button touch starting point in the X-coordinate plane.
 * @param[in] y Virtual button touch starting point in the Y-coordinate plane.
 * @param[in] w Virtual button touched width from the starting point in the X-coordinate plane.
 * @param[in] h Virtual button touched height from the starting point in the Y-coordinate plane.
 * 
 * @return An instance of the virtual button struct.
 */
/* @[declare_button_attach] */
Button_t* Button_Attach(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
/* @[declare_button_attach] */

/**
 * @brief See if the specified button was pressed. 
 * 
 * @param[out] button The button to check if it was pressed. Updates the value of the state definition.
 * 
 * @return A boolean if the button specified was pressed.
 * 0x00 for not pressed, 0xff for pressed.
 */
/* @[declare_button_waspressed] */
uint8_t Button_WasPressed(Button_t* button);
/* @[declare_button_waspressed] */

/**
 * @brief See if the specified button press was released. 
 * 
 * @param[out] button The button to check if it was released. Updates the value of the state definition.
 * 
 * @return A boolean if the button specified was released.
 * 0x00 for not released, 0xff for released.
 */
/* @[declare_button_wasreleased] */
uint8_t Button_WasReleased(Button_t* button);
/* @[declare_button_wasreleased] */

/**
 * @brief See if the specified button is currently being pressed. 
 * 
 * @param[out] button The button to check if it is currently being pressed. Updates the value of the state definition.
 * 
 * @return A boolean if the button specified is currently being pressed.
 * 0x00 for currently not being pressed, 0xff for is currently being pressed.
 */
/* @[declare_button_ispress] */
uint8_t Button_IsPress(Button_t* button);
/* @[declare_button_ispress] */

/**
 * @brief See if the specified button press is currently released. 
 * 
 * @param[out] button The button to check if it is currently released.
 * 
 * @return A boolean if the button specified is currently released.
 * 0x00 for currently not released, 0xff for is currently released.
 */
/* @[declare_button_isrelease] */
uint8_t Button_IsRelease(Button_t* button);
/* @[declare_button_isrelease] */

/**
 * @brief See if the specified button press was a long press. 
 * 
 * @param[out] button The button to check if it was a long press button event. Updates the value of the state definition.
 * @param[in] long_press_time The time duration to consider a button press a long press.
 * 
 * @return A boolean if the button specified was long pressed.
 * 0x00 for not a long button press, 0xff for was a long button press.
 */
/* @[declare_button_waslongpress] */
uint8_t Button_WasLongPress(Button_t* button, uint32_t long_press_time);
/* @[declare_button_waslongpress] */