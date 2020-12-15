/*******************************************************************************
  WINC Wireless Driver System Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_api.h

  Summary:
    WINC wireless driver system API header file.

  Description:
    This interface is intended to be called from the main system initialization
    and task functions.
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

#ifndef _WDRV_WINC_API_H
#define _WDRV_WINC_API_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "system/system_module.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
    extern "C" {
#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver System Routines
// *****************************************************************************
// *****************************************************************************

//*******************************************************************************
/*
  Function:
    SYS_MODULE_OBJ WDRV_WINC_Initialize
    (
        const SYS_MODULE_INDEX index,
        const SYS_MODULE_INIT *const init
    )

  Summary:
    System interface initialization of the WINC driver.

  Description:
    This is the function that initializes the WINC driver. It is called by
    the system.

  Precondition:
    None.

  Parameters:
    index   - Index for the WINC driver instance to be initialized.
    init    - Pointer to initialization data, currently not used.

  Returns:
    Valid handle to a driver object - if successful
    SYS_MODULE_OBJ_INVALID - if initialization failed

  Remarks:
    The returned object must be passed as argument to WDRV_WINC_Reinitialize,
      WDRV_WINC_Deinitialize, WDRV_WINC_Tasks and WDRV_WINC_Status routines.

*/

SYS_MODULE_OBJ WDRV_WINC_Initialize
(
    const SYS_MODULE_INDEX index,
    const SYS_MODULE_INIT *const init
);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_Deinitialize(SYS_MODULE_OBJ object)

  Summary:
    WINC driver deinitialization function.

  Description:
    This is the function that deinitializes the WINC.
    It is called by the system.

  Precondition:
    WDRV_WINC_Initialize should have been called.

  Parameters:
    object  - Driver object handle, returned from WDRV_WINC_Initialize

  Returns:
    None.

  Remarks:
    None.

*/

void WDRV_WINC_Deinitialize(SYS_MODULE_OBJ object);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_Reinitialize
    (
        SYS_MODULE_OBJ object,
        const SYS_MODULE_INIT *const init
    )

  Summary:
    WINC driver reinitialization function.

  Description:
    This is the function that re-initializes the WINC.
    It is called by the system.

  Precondition:
    WDRV_WINC_Initialize should have been called.

  Parameters:
    object  - Driver object handle, returned from WDRV_WINC_Initialize
    init    - Pointer to initialization data, currently not used.

  Returns:
    None.

  Remarks:
    None.

*/

void WDRV_WINC_Reinitialize
(
    SYS_MODULE_OBJ object,
    const SYS_MODULE_INIT *const init
);

//*******************************************************************************
/*
  Function:
    SYS_STATUS WDRV_WINC_Status(SYS_MODULE_OBJ object)

  Summary:
    Provides the current status of the WINC driver module.

  Description:
    This function provides the current status of the WINC driver module.

  Precondition:
    WDRV_WINC_Initialize must have been called before calling this function.

  Parameters:
    object  - Driver object handle, returned from WDRV_WINC_Initialize

  Returns:
    SYS_STATUS_READY  - Indicates that any previous module operation for the
                          specified module has completed
    SYS_STATUS_BUSY   - Indicates that a previous module operation for the
                          specified module has not yet completed
    SYS_STATUS_ERROR  - Indicates that the specified module is in an error state

  Example:
    <code>
    SYS_MODULE_OBJ      object;     // Returned from WDRV_WINC_Initialize
    SYS_STATUS          status;

    status = WDRV_WINC_Status(object);
    if (SYS_STATUS_ERROR >= status)
    {
        // Handle error
    }
    </code>

  Remarks:
    Any value greater than SYS_STATUS_READY is also a normal running state
      in which the driver is ready to accept new operations.

    SYS_STATUS_BUSY - Indicates that the driver is busy with a previous
      system level operation and cannot start another

    SYS_STATUS_ERROR - Indicates that the driver is in an error state
     Any value less than SYS_STATUS_ERROR is also an error state.

    SYS_MODULE_DEINITIALIZED - Indicates that the driver has been
      deinitialized

    This operation can be used to determine when any of the driver's
      module level operations has completed.

    If the status operation returns SYS_STATUS_BUSY, then a previous
      operation has not yet completed. Once the status operation returns
      SYS_STATUS_READY, any previous operations have completed.

    The value of SYS_STATUS_ERROR is negative (-1). Any value less than
      that is also an error state.

    This function will NEVER block waiting for hardware.

    If the Status operation returns an error value, the error may be
      cleared by calling the reinitialize operation. If that fails, the
      deinitialize operation will need to be called, followed by the
      initialize operation to return to normal operations.

*/

SYS_STATUS WDRV_WINC_Status(SYS_MODULE_OBJ object);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_Tasks(SYS_MODULE_OBJ object)

  Summary:
    Maintains the WINC drivers state machine.

  Description:
    This function is used to maintain the driver's internal state machine.

  Precondition:
    WDRV_WINC_Initialize must have been called before calling this function.

  Parameters:
    object  - Driver object handle, returned from WDRV_WINC_Initialize

  Returns:
    None.

  Remarks:
    This function is normally not called directly by an application. It is
      called by the system's Tasks routine (SYS_Tasks).

*/

void WDRV_WINC_Tasks(SYS_MODULE_OBJ object);

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_ISR(void);

  Summary:
    WINC interrupt handler.

  Description:
    This function is called by the interrupt routines to signal an interrupt
      from the WINC.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Remarks:
    None.

*/

void WDRV_WINC_ISR(void);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _WDRV_WINC_API_H */
