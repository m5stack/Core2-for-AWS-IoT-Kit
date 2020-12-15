/*******************************************************************************
  WINC Driver Debugging Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_debug.h

  Summary:
    WINC wireless driver debug header file.

  Description:
    Provides methods to send formatted debugging information.
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

#ifndef _WDRV_WINC_DEBUG_H
#define _WDRV_WINC_DEBUG_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "configuration.h"
#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
    extern "C" {
#endif
// DOM-IGNORE-END

// *****************************************************************************
/*  Debug Callback

  Summary:
    Defines the debug callback.

  Description:
    The function callback provides a printf-like prototype.

  Remarks:
    None.
*/

typedef void (*WDRV_WINC_DEBUG_PRINT_CALLBACK)(const char*, ...);

// *****************************************************************************
/*  Debug Levels

  Summary:
    Defines for debugging verbosity level.

  Description:
    These defines can be assigned to WDRV_WINC_DEBUG_LEVEL to affect the level of
      verbosity provided by the debug output channel.

  Remarks:
    None.
*/

#define WDRV_WINC_DEBUG_TYPE_NONE       0
#define WDRV_WINC_DEBUG_TYPE_ERROR      1
#define WDRV_WINC_DEBUG_TYPE_INFORM     2
#define WDRV_WINC_DEBUG_TYPE_TRACE      3
#define WDRV_WINC_DEBUG_TYPE_VERBOSE    4

// *****************************************************************************
/*  Debug Verbosity

  Summary:
    Defines the chosen level of debugging verbosity supported.

  Description:
    This define set the debugging output verbosity level.

  Remarks:
    None.
*/

#ifndef WDRV_WINC_DEBUG_LEVEL
#define WDRV_WINC_DEBUG_LEVEL   WDRV_WINC_DEBUG_TYPE_TRACE
#endif

// *****************************************************************************
/*  Debugging Macros

  Summary:
    Macros to define debugging output.

  Description:
    Macros are provided for each level of verbosity.

  Remarks:
    None.
*/

#define WDRV_DBG_VERBOSE_PRINT(...)
#define WDRV_DBG_TRACE_PRINT(...)
#define WDRV_DBG_INFORM_PRINT(...)
#define WDRV_DBG_ERROR_PRINT(...)

#if (WDRV_WINC_DEBUG_LEVEL >= WDRV_WINC_DEBUG_TYPE_ERROR)
#undef WDRV_DBG_ERROR_PRINT
#define WDRV_DBG_ERROR_PRINT(...) do { if (pfWINCDebugPrintCb) { pfWINCDebugPrintCb(__VA_ARGS__); } } while (0)
#if (WDRV_WINC_DEBUG_LEVEL >= WDRV_WINC_DEBUG_TYPE_INFORM)
#undef WDRV_DBG_INFORM_PRINT
#define WDRV_DBG_INFORM_PRINT(...) do { if (pfWINCDebugPrintCb) { pfWINCDebugPrintCb(__VA_ARGS__); } } while (0)
#if (WDRV_WINC_DEBUG_LEVEL >= WDRV_WINC_DEBUG_TYPE_TRACE)
#undef WDRV_DBG_TRACE_PRINT
#define WDRV_DBG_TRACE_PRINT(...) do { if (pfWINCDebugPrintCb) { pfWINCDebugPrintCb(__VA_ARGS__); } } while (0)
#if (WDRV_WINC_DEBUG_LEVEL >= WDRV_WINC_DEBUG_TYPE_VERBOSE)
#undef WDRV_DBG_VERBOSE_PRINT
#define WDRV_DBG_VERBOSE_PRINT(...) do { if (pfWINCDebugPrintCb) { pfWINCDebugPrintCb(__VA_ARGS__); } } while (0)
#endif /* WDRV_WINC_DEBUG_TYPE_VERBOSE */
#endif /* WDRV_WINC_DEBUG_TYPE_TRACE */
#endif /* WDRV_WINC_DEBUG_TYPE_INFORM */
#endif /* WDRV_WINC_DEBUG_TYPE_ERROR */

// Reference debug output channel printf-like function.
extern WDRV_WINC_DEBUG_PRINT_CALLBACK pfWINCDebugPrintCb;

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _WDRV_WINC_DEBUG_H */
