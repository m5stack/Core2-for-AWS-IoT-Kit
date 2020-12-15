/*******************************************************************************
  Real Time Counter (RTC) PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_rtc.h

  Summary:
    RTC PLIB Header file

  Description:
    This file defines the interface to the RTC peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

*******************************************************************************/
// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

#ifndef PLIB_RTC_H
#define PLIB_RTC_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
extern "C" {
#endif
// DOM-IGNORE-END

/* Frequency of Counter Clock for RTC */
#define RTC_COUNTER_CLOCK_FREQUENCY        (1024 / (1 << (0xA)))

typedef enum
{
    RTC_TIMER32_INT_MASK_COMPARE_MATCH = 0x0001,
    RTC_TIMER32_INT_MASK_COUNTER_OVERFLOW = 0x0080,
} RTC_TIMER32_INT_MASK;

typedef void (*RTC_TIMER32_CALLBACK)( RTC_TIMER32_INT_MASK intCause, uintptr_t context );


typedef struct
{
    /* Timer 32Bit */
    RTC_TIMER32_CALLBACK timer32BitCallback;
    RTC_TIMER32_INT_MASK timer32intCause;
    uintptr_t context;
} RTC_OBJECT;

void RTC_Initialize(void);

void RTC_Timer32Start ( void );
void RTC_Timer32Stop ( void );
void RTC_Timer32CounterSet ( uint32_t count );
uint32_t RTC_Timer32CounterGet ( void );
uint32_t RTC_Timer32FrequencyGet ( void );
void RTC_Timer32CompareSet ( uint32_t compareValue );
uint32_t RTC_Timer32PeriodGet ( void );
void RTC_Timer32InterruptEnable(RTC_TIMER32_INT_MASK interrupt);
void RTC_Timer32InterruptDisable(RTC_TIMER32_INT_MASK interrupt);

void RTC_Timer32CallbackRegister ( RTC_TIMER32_CALLBACK callback, uintptr_t context );

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
}
#endif
// DOM-IGNORE-END

#endif /* PLIB_RTC_H */
