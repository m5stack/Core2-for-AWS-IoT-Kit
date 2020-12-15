/*******************************************************************************
  WINC Driver Power Save Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_powersave.h

  Summary:
    WINC wireless driver power save header file.

  Description:
    Provides an interface to control the power states of the WINC.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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
// DOM-IGNORE-END

#ifndef _WDRV_WINC_POWERSAVE_H
#define _WDRV_WINC_POWERSAVE_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_winc_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Power Save Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  Power Save Modes

  Summary:
    Defines the power save modes.

  Description:
    These mode describe possible power states.

  Remarks:
    None.
*/

typedef enum
{
    /* Invalid mode. */
    WDRV_WINC_PS_MODE_INVALID = /*DOM-IGNORE-BEGIN*/ -1 /*DOM-IGNORE-END*/,

    /* Power save is off. */
    WDRV_WINC_PS_MODE_OFF = /*DOM-IGNORE-BEGIN*/ M2M_NO_PS /*DOM-IGNORE-END*/,

#ifdef WDRV_WINC_DEVICE_WINC1500
    /* Automatic control of power save. */
    WDRV_WINC_PS_MODE_AUTO_HIGH_POWER = /*DOM-IGNORE-BEGIN*/ M2M_PS_AUTOMATIC /*DOM-IGNORE-END*/,

    /* Automatic control of power save. */
    WDRV_WINC_PS_MODE_AUTO_MED_POWER = /*DOM-IGNORE-BEGIN*/ M2M_PS_H_AUTOMATIC /*DOM-IGNORE-END*/,
#endif
    /* Automatic control of power save. */
    WDRV_WINC_PS_MODE_AUTO_LOW_POWER = /*DOM-IGNORE-BEGIN*/ M2M_PS_DEEP_AUTOMATIC /*DOM-IGNORE-END*/,

#ifdef WDRV_WINC_DEVICE_WINC1500
    /* Manual control of power save. */
    WDRV_WINC_PS_MODE_MANUAL = /*DOM-IGNORE-BEGIN*/ M2M_PS_MANUAL /*DOM-IGNORE-END*/
#endif
} WDRV_WINC_PS_MODE;

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Power Save Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_PowerSaveSetMode
    (
        DRV_HANDLE handle,
        WDRV_WINC_PS_MODE mode
    )

  Summary:
    Sets the power save mode.

  Description:
    Selects the current power save mode the WINC should be using.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle - Client handle obtained by a call to WDRV_WINC_Open.
    mode   - Desired new power save mode.

  Returns:
    WDRV_WINC_STATUS_OK             - The power save mode has been accepted.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_PowerSaveSetMode
(
    DRV_HANDLE handle,
    WDRV_WINC_PS_MODE mode
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_PS_MODE WDRV_WINC_PowerSaveGetMode(DRV_HANDLE handle)

  Summary:
    Returns the current power save mode.

  Description:
    Returns to the caller the current power save mode in use by the WINC.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle - Client handle obtained by a call to WDRV_WINC_Open.

  Returns:
    WDRV_WINC_PS_MODE_INVALID - An error occurred.
    Other WDRV_WINC_PS_MODE value - Current power save mode.

  Remarks:
    None.

*/

WDRV_WINC_PS_MODE WDRV_WINC_PowerSaveGetMode(DRV_HANDLE handle);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_PowerSaveSetBeaconInterval
    (
        DRV_HANDLE handle,
        uint16_t numBeaconIntervals
    )

  Summary:
    Configures the beacon listening interval.

  Description:
    The beacon listening interval specifies how many beacon period will occur
      between listening actions when the WINC is in low power mode.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.

  Parameters:
    handle             - Client handle obtained by a call to WDRV_WINC_Open.
    numBeaconIntervals - Number of beacon intervals between listening operations.

  Returns:
    WDRV_WINC_STATUS_OK             - The interval has been accepted.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_PowerSaveSetBeaconInterval
(
    DRV_HANDLE handle,
    uint16_t numBeaconIntervals
);

#ifdef WDRV_WINC_DEVICE_WINC1500
//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_PowerSaveManualSleep
    (
        DRV_HANDLE handle,
        uint32_t sleepTime
    )

  Summary:
    Manual sleep power save operation.

  Description:
    If the current power save mode is WDRV_WINC_PS_MODE_MANUAL the WINC
      can be placed into low power mode by requesting sleep.

  Precondition:
    WDRV_WINC_Initialize should have been called.
    WDRV_WINC_Open should have been called to obtain a valid handle.
    Power save mode must be WDRV_WINC_PS_MODE_MANUAL

  Parameters:
    handle    - Client handle obtained by a call to WDRV_WINC_Open.
    sleepTime - Sleep period specified in milli-seconds.

  Returns:
    WDRV_WINC_STATUS_OK             - The request has been accepted.
    WDRV_WINC_STATUS_NOT_OPEN       - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG    - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR  - The request to the WINC was rejected.

  Remarks:
    None.

*/

WDRV_WINC_STATUS WDRV_WINC_PowerSaveManualSleep
(
    DRV_HANDLE handle,
    uint32_t sleepTime
);

#endif

#endif /* _WDRV_WINC_POWERSAVE_H */
