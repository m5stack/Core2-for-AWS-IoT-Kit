/**
 * @file atecc608.h
 * @brief Functions for the ATECC608 Trust&GO secure element.
 */

#pragma once

#include "stdio.h"

/** @brief I2C port the ATECC608 uses to communicate with the ESP32-D0WD main MCU */
/* @[declare_atecc608_i2c_port] */
#define ATECC608_I2C_PORT I2C_NUM_1
/* @[declare_atecc608_i2c_port] */

/**
 * @brief Initializes the ATECC608 Trust&GO on the I2C bus.
 * 
 * @note Must initialize the ATECC608 before performing 
 * any operations on the secure element.
 * @note The Core2ForAWS_Init() calls this function
 * when the hardware feature is enabled.
 * 
 * @return Status of initialization.
 * 0 is success. View the list of possible return status codes [here](https://github.com/espressif/esp-cryptoauthlib/blob/603225f3a4d0607d4e3c5483dc251639e956f9d3/cryptoauthlib/lib/atca_status.h).
 */
/* @[declare_atecc608_init] */
ATCA_STATUS Atecc608_Init();
/* @[declare_atecc608_init] */

/**
 * @brief Retrieves the unique serial number from the ATECC608.
 * 
 * The serial number of the ATECC608 is stored as a uint8
 * with the length defined in the macro ATCA_SERIAL_NUM_SIZE.
 * It might take a little time to wake the secure element and
 * retrieve the serial number.
 * 
 * **Example:**
 * 
 * Create a variable named clientId with enough dynamically 
 * allocated memory to store the ATECC608 serial number.
 * @code{c}
 *  char * clientId = malloc(ATCA_SERIAL_NUM_SIZE * 2 + 1); // ATCA_SERIAL_NUM_SIZE returns the 8-bit serial number size. 
 *                                                          // Need double for 16-bit and one more for the null terminator
 *                                                          // for the end of the char.
 *  ATCA_STATUS ret = Atecc608_GetSerialString(clientId);
 * @endcode
 * 
 * @param[out] sn The unique serial number of the chip.
 * 
 * @return Status of serial number retrieval operation.
 * 0 is success. View the list of possible return status codes [here](https://github.com/espressif/esp-cryptoauthlib/blob/603225f3a4d0607d4e3c5483dc251639e956f9d3/cryptoauthlib/lib/atca_status.h).
 */
/* @[declare_atecc608_getserialstring] */
ATCA_STATUS Atecc608_GetSerialString(char * sn);
/* @[declare_atecc608_getserialstring] */