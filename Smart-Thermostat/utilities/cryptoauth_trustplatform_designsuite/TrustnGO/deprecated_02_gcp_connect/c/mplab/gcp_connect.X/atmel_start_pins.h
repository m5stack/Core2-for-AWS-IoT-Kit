/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef ATMEL_START_PINS_H_INCLUDED
#define ATMEL_START_PINS_H_INCLUDED

#include <hal_gpio.h>

// SAMD21 has 8 pin functions

#define GPIO_PIN_FUNCTION_A 0
#define GPIO_PIN_FUNCTION_B 1
#define GPIO_PIN_FUNCTION_C 2
#define GPIO_PIN_FUNCTION_D 3
#define GPIO_PIN_FUNCTION_E 4
#define GPIO_PIN_FUNCTION_F 5
#define GPIO_PIN_FUNCTION_G 6
#define GPIO_PIN_FUNCTION_H 7


/*The PIN used by the USB dongle to control the reset of the winc module*/
#define RESET_PIN1 GPIO(GPIO_PORTA, 3)

/*For older versions of USB dongle board uses the pin 4 for resetting the winc module*/
#define RESET_PIN2 GPIO(GPIO_PORTA, 4)

#define PA05 GPIO(GPIO_PORTA, 5)
#define PA08 GPIO(GPIO_PORTA, 8)
#define PA09 GPIO(GPIO_PORTA, 9)
#define CE_PIN GPIO(GPIO_PORTA, 10)
#define PA16 GPIO(GPIO_PORTA, 16)
#define CS_PIN GPIO(GPIO_PORTA, 17)
#define PA18 GPIO(GPIO_PORTA, 18)
#define PA19 GPIO(GPIO_PORTA, 19)
#define PA22 GPIO(GPIO_PORTA, 22)
#define PA23 GPIO(GPIO_PORTA, 23)

#define LED0 GPIO(GPIO_PORTA, 2)
#define CONF_WINC_EXT_INT_PIN PA05

#endif // ATMEL_START_PINS_H_INCLUDED
