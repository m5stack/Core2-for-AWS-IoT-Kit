/*******************************************************************************
  PORTS Service

  Company:
    Microchip Technology Inc.

  File Name:
    sys_ports.h

  Summary:
    PORTS Service Header File

  Description:
    This library provides an interface to control and interact with PORTS
    System Service.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#ifndef SYS_PORTS_H
#define SYS_PORTS_H

#include "device.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Data types and constants
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
/* Sys Port

  Summary:
    Identifies the available Port Channels.

  Description:
    This enumeration identifies the available Port Channels.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all ports are available on all devices.  Refer to the specific
    device data sheet to determine which ports are supported.
*/

typedef enum
{
    SYS_PORT_0 = PORT_BASE_ADDRESS + 0 * (0x80),
} SYS_PORT;


// *****************************************************************************
/* Sys Port Pins

  Summary:
    Identifies the available port pins.

  Description:
    This enumeration identifies the available port pins.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all pins are available on all devices.  Refer to the specific
    device data sheet to determine which pins are supported.
*/

typedef enum
{
    SYS_PORT_PIN_PA00 = 0,

    SYS_PORT_PIN_PA01 = 1,

    SYS_PORT_PIN_PA02 = 2,

    SYS_PORT_PIN_PA03 = 3,

    SYS_PORT_PIN_PA04 = 4,

    SYS_PORT_PIN_PA05 = 5,

    SYS_PORT_PIN_PA06 = 6,

    SYS_PORT_PIN_PA07 = 7,

    SYS_PORT_PIN_PA08 = 8,

    SYS_PORT_PIN_PA09 = 9,

    SYS_PORT_PIN_PA10 = 10,

    SYS_PORT_PIN_PA11 = 11,

    SYS_PORT_PIN_PA14 = 14,

    SYS_PORT_PIN_PA15 = 15,

    SYS_PORT_PIN_PA16 = 16,

    SYS_PORT_PIN_PA17 = 17,

    SYS_PORT_PIN_PA18 = 18,

    SYS_PORT_PIN_PA19 = 19,

    SYS_PORT_PIN_PA22 = 22,

    SYS_PORT_PIN_PA23 = 23,

    SYS_PORT_PIN_PA24 = 24,

    SYS_PORT_PIN_PA25 = 25,

    SYS_PORT_PIN_PA27 = 27,

    SYS_PORT_PIN_PA28 = 28,

    SYS_PORT_PIN_PA30 = 30,

    SYS_PORT_PIN_PA31 = 31,

    /* This element should not be used in any of the PORTS APIs.
       It will be used by other modules or application to denote that none of the PORT Pin is used */
    SYS_PORT_PIN_NONE = -1
} SYS_PORT_PIN;


// *****************************************************************************
// *****************************************************************************
// Section: SYS PORT Functions which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void SYS_PORT_PinWrite(SYS_PORT_PIN pin, bool value)

  Summary:
    Writes to the selected pin.

  Description:
    This function writes/drives the "value" on the selected I/O line/pin.

  Precondition:
    Port Initialization must have been done using appropriate Initialize API call.

  Parameters:
    pin       - One of the IO pins from the enum SYS_PORT_PIN
    value     - value to be written on the selected pin:
                true  = set pin to high (1).
                false = clear pin to low (0).

  Returns:
    None.

  Example:
    <code>
    SYS_PORT_PinWrite(SYS_PORT_PIN_PB3, true);
    </code>

  Remarks:
    None.
*/
static inline void SYS_PORT_PinWrite(SYS_PORT_PIN pin, bool value);

// *****************************************************************************
/* Function:
    bool SYS_PORT_PinRead(SYS_PORT_PIN pin)

  Summary:
    Read the selected pin value.

  Description:
    This function reads the selected pin value.
    it reads the value regardless of pin configuration, whether uniquely as an
    input, or driven by the PIO Controller, or driven by peripheral.

  Precondition:
    Reading the I/O line levels requires the clock of the PIO Controller to be
    enabled, otherwise this API reads the levels present on the I/O line at the
    time the clock was disabled.

  Parameters:
    pin - One of the IO pins from the enum SYS_PORT_PIN

  Returns:
    Returns the read value of the selected I/O pin.

  Example:
    <code>

    bool value;
    value = SYS_PORT_PinRead(SYS_PORT_PIN_PB3);

    </code>

  Remarks:
       To read the latched value on this pin, SYS_PORT_PinLatchRead API should be used.
*/
static inline bool SYS_PORT_PinRead(SYS_PORT_PIN pin);

// *****************************************************************************
/* Function:
    bool SYS_PORT_PinLatchRead ( SYS_PORT_PIN pin )

  Summary:
    Read the value driven on the selected pin.

  Description:
    This function reads the data driven on the selected I/O line/pin.
    Whatever data is written/driven on I/O line by using any of the PORTS
    APIs, will be read by this API.

  Precondition:
    None.

  Parameters:
    pin - One of the IO pins from the enum SYS_PORT_PIN

  Returns:
    Returns the value driven on the selected I/O pin.

  Example:
    <code>

    bool value;
    value = SYS_PORT_PinLatchRead(SYS_PORT_PIN_PB3);

    </code>

  Remarks:
    To read actual pin value, SYS_PORT_PinRead API should be used.
*/
static inline bool SYS_PORT_PinLatchRead(SYS_PORT_PIN pin);

// *****************************************************************************
/* Function:
    void SYS_PORT_PinToggle(SYS_PORT_PIN pin)

  Summary:
    Toggles the selected pin.

  Description:
    This function toggles/inverts the value on the selected I/O line/pin.

  Precondition:
    Port Initialization must have been done using appropriate Initialize API call.

  Parameters:
    pin - One of the IO pins from the enum SYS_PORT_PIN

  Returns:
    None.

  Example:
    <code>

    SYS_PORT_PinToggle(SYS_PORT_PIN_PB3);

    </code>

  Remarks:
    None.
*/
static inline void SYS_PORT_PinToggle(SYS_PORT_PIN pin);

// *****************************************************************************
/* Function:
    void SYS_PORT_PinSet(SYS_PORT_PIN pin)

  Summary:
    Sets the selected pin.

  Description:
    This function drives '1' on the selected I/O line/pin.

  Precondition:
    None.

  Parameters:
    pin - One of the IO pins from the enum SYS_PORT_PIN

  Returns:
    None.

  Example:
    <code>

    SYS_PORT_PinSet(SYS_PORT_PIN_PB3);

    </code>

  Remarks:
    None.
*/
static inline void SYS_PORT_PinSet(SYS_PORT_PIN pin);

// *****************************************************************************
/* Function:
    void SYS_PORT_PinClear(SYS_PORT_PIN pin)

  Summary:
    Clears the selected pin.

  Description:
    This function drives '0' on the selected I/O line/pin.

  Precondition:
    None.

  Parameters:
    pin - One of the IO pins from the enum SYS_PORT_PIN

  Returns:
    None.

  Example:
    <code>

    SYS_PORT_PinClear(SYS_PORT_PIN_PB3);

    </code>

  Remarks:
    None.
*/
static inline void SYS_PORT_PinClear(SYS_PORT_PIN pin);

// *****************************************************************************
/* Function:
    void SYS_PORT_PinInputEnable(SYS_PORT_PIN pin)

  Summary:
    Enables selected IO pin as input.

  Description:
    This function enables selected IO pin as input.

  Precondition:
    None.

  Parameters:
    pin - One of the IO pins from the enum SYS_PORT_PIN

  Returns:
    None.

  Example:
    <code>

    SYS_PORT_PinInputEnable(SYS_PORT_PIN_PB3);

    </code>

  Remarks:
    None.
*/
static inline void SYS_PORT_PinInputEnable(SYS_PORT_PIN pin);

// *****************************************************************************
/* Function:
    void SYS_PORT_PinOutputEnable(SYS_PORT_PIN pin)

  Summary:
    Enables selected IO pin as output.

  Description:
    This function enables selected IO pin as output.

  Precondition:
    None.

  Parameters:
    pin - One of the IO pins from the enum SYS_PORT_PIN

  Returns:
    None.

  Example:
    <code>

    SYS_PORT_PinOutputEnable(SYS_PORT_PIN_PB3);

    </code>

  Remarks:
    None.
*/
static inline void SYS_PORT_PinOutputEnable(SYS_PORT_PIN pin);


#include "sys_ports_mapping.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // SYS_PORTS_H
