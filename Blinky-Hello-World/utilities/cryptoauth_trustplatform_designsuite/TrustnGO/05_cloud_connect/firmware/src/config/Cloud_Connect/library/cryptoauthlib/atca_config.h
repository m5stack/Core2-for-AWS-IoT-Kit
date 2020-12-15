/* Auto-generated config file atca_config.h */
#ifndef ATCA_CONFIG_H
#define ATCA_CONFIG_H

/* MPLAB Harmony Common Include */
#include "definitions.h"


#ifndef ATCA_HAL_I2C
#define ATCA_HAL_I2C
#endif

/** Include Device Support Options */
#define ATCA_ATECC608A_SUPPORT
#define ATCA_ATECC608_SUPPORT


/* Polling Configuration Options  */
#ifndef ATCA_POLLING_INIT_TIME_MSEC
#define ATCA_POLLING_INIT_TIME_MSEC       1
#endif
#ifndef ATCA_POLLING_FREQUENCY_TIME_MSEC
#define ATCA_POLLING_FREQUENCY_TIME_MSEC  2
#endif
#ifndef ATCA_POLLING_MAX_TIME_MSEC
#define ATCA_POLLING_MAX_TIME_MSEC        2500
#endif

/** Define if the library is not to use malloc/free */
#ifndef ATCA_NO_HEAP
#define ATCA_NO_HEAP
#endif

#define atca_delay_ms   hal_delay_ms
#define atca_delay_us   hal_delay_us

/* \brief How long to wait after an initial wake failure for the POST to
 *         complete.
 * If Power-on self test (POST) is enabled, the self test will run on waking
 * from sleep or during power-on, which delays the wake reply.
 */
#ifndef ATCA_POST_DELAY_MSEC
#define ATCA_POST_DELAY_MSEC 25
#endif


/* Define generic interfaces to the processor libraries */

#define PLIB_I2C_ERROR          SERCOM_I2C_ERROR
#define PLIB_I2C_ERROR_NONE     SERCOM_I2C_ERROR_NONE
#define PLIB_I2C_TRANSFER_SETUP SERCOM_I2C_TRANSFER_SETUP

typedef bool (* atca_i2c_plib_read)( uint16_t, uint8_t *, uint32_t );
typedef bool (* atca_i2c_plib_write)( uint16_t, uint8_t *, uint32_t );
typedef bool (* atca_i2c_plib_is_busy)( void );
typedef PLIB_I2C_ERROR (* atca_i2c_error_get)( void );
typedef bool (* atca_i2c_plib_transfer_setup)(PLIB_I2C_TRANSFER_SETUP* setup, uint32_t srcClkFreq);

typedef struct atca_plib_api
{
    atca_i2c_plib_read              read;
    atca_i2c_plib_write             write;
    atca_i2c_plib_is_busy           is_busy;
    atca_i2c_error_get              error_get;
    atca_i2c_plib_transfer_setup    transfer_setup;
} atca_plib_i2c_api_t;

extern atca_plib_i2c_api_t sercom2_plib_i2c_api;
extern atca_plib_i2c_api_t sercom2_plib_i2c_api;

/** Define certificate templates to be supported. */
#define ATCA_TNGTLS_SUPPORT
#define ATCA_TNG_LEGACY_SUPPORT



#endif // ATCA_CONFIG_H
