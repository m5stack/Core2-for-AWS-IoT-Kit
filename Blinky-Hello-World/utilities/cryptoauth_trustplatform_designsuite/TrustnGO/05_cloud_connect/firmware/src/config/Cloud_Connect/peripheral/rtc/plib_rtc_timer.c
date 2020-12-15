/*******************************************************************************
  Real Time Counter (RTC) PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_rtc_timer.c

  Summary:
    RTC PLIB Implementation file

  Description:
    This file defines the interface to the RTC peripheral library. This
    library provides access to and control of the associated peripheral
    instance in timer mode.

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

#include "plib_rtc.h"
#include "device.h"
#include <stdlib.h>

RTC_OBJECT rtcObj;

static void RTC_CountReadSynchronization(void)
{
   /* Read-synchronization for COUNT register */
   RTC_REGS->MODE0.RTC_READREQ = RTC_READREQ_RREQ_Msk | RTC_READREQ_ADDR(0x10);
   while((RTC_REGS->MODE0.RTC_STATUS & RTC_STATUS_SYNCBUSY_Msk) == RTC_STATUS_SYNCBUSY_Msk)
   {
       /* Wait for Read-Synchronization */
   }
}

void RTC_Initialize(void)
{
   /* Writing to CTRL register will trigger write-synchronization */
   RTC_REGS->MODE0.RTC_CTRL = RTC_MODE0_CTRL_SWRST_Msk;
   while((RTC_REGS->MODE0.RTC_STATUS & RTC_STATUS_SYNCBUSY_Msk) == RTC_STATUS_SYNCBUSY_Msk)
   {
       /* Wait for Write-Synchronization */
   }

   /* Writing to CTRL register will trigger write-synchronization */
   RTC_REGS->MODE0.RTC_CTRL = RTC_MODE0_CTRL_MODE(0) | RTC_MODE0_CTRL_PRESCALER(0xA) | RTC_MODE0_CTRL_MATCHCLR_Msk;
   while((RTC_REGS->MODE0.RTC_STATUS & RTC_STATUS_SYNCBUSY_Msk) == RTC_STATUS_SYNCBUSY_Msk)
   {
       /* Wait for Write-Synchronization */
   }

   /* Writing to COMP register will trigger write-synchronization */
   RTC_REGS->MODE0.RTC_COMP = 0x0;
   while((RTC_REGS->MODE0.RTC_STATUS & RTC_STATUS_SYNCBUSY_Msk) == RTC_STATUS_SYNCBUSY_Msk)
   {
       /* Wait for Write-Synchronization */
   }


}



void RTC_Timer32Start ( void )
{
   /* Writing to CTRL register will trigger write-synchronization */
   RTC_REGS->MODE0.RTC_CTRL |= RTC_MODE0_CTRL_ENABLE_Msk;
   while((RTC_REGS->MODE0.RTC_STATUS & RTC_STATUS_SYNCBUSY_Msk) == RTC_STATUS_SYNCBUSY_Msk)
   {
       /* Wait for Write-synchronization */
   }
}

void RTC_Timer32Stop ( void )
{
   /* Writing to CTRL register will trigger write-synchronization */
   RTC_REGS->MODE0.RTC_CTRL &= ~(RTC_MODE0_CTRL_ENABLE_Msk);
   while((RTC_REGS->MODE0.RTC_STATUS & RTC_STATUS_SYNCBUSY_Msk) == RTC_STATUS_SYNCBUSY_Msk)
   {
       /* Wait for Write-Synchronization */
   }
}

void RTC_Timer32CounterSet ( uint32_t count )
{
   /* Writing to COUNT register will trigger write-synchronization */
   RTC_REGS->MODE0.RTC_COUNT = count;
   while((RTC_REGS->MODE0.RTC_STATUS & RTC_STATUS_SYNCBUSY_Msk) == RTC_STATUS_SYNCBUSY_Msk)
   {
       /* Wait for Write-Synchronization */
   }
}

void RTC_Timer32CompareSet ( uint32_t compareValue )
{
   /* Writing to COMP register will trigger write-synchronization */
   RTC_REGS->MODE0.RTC_COMP = compareValue;
   while((RTC_REGS->MODE0.RTC_STATUS & RTC_STATUS_SYNCBUSY_Msk) == RTC_STATUS_SYNCBUSY_Msk)
   {
       /* Wait for Write-Synchronization */
   }
}

uint32_t RTC_Timer32CounterGet ( void )
{
   /* Enable read-synchronization for COUNT register to avoid CPU stall */
   RTC_CountReadSynchronization();
   return(RTC_REGS->MODE0.RTC_COUNT);
}

uint32_t RTC_Timer32PeriodGet ( void )
{
   /* Get 32Bit Compare Value */
   return (RTC_MODE0_COUNT_COUNT_Msk);
}

uint32_t RTC_Timer32FrequencyGet ( void )
{
   /* Return Frequency of RTC Clock */
   return RTC_COUNTER_CLOCK_FREQUENCY;
}

void RTC_Timer32InterruptEnable(RTC_TIMER32_INT_MASK interrupt)
{
   RTC_REGS->MODE0.RTC_INTENSET = interrupt;
}

void RTC_Timer32InterruptDisable(RTC_TIMER32_INT_MASK interrupt)
{
   RTC_REGS->MODE0.RTC_INTENCLR = interrupt;
}

void RTC_Timer32CallbackRegister ( RTC_TIMER32_CALLBACK callback, uintptr_t context )
{
   rtcObj.timer32BitCallback = callback;
   rtcObj.context            = context;
}

void RTC_InterruptHandler(void)
{
   rtcObj.timer32intCause = (RTC_TIMER32_INT_MASK) RTC_REGS->MODE0.RTC_INTFLAG;
   RTC_REGS->MODE0.RTC_INTFLAG = RTC_MODE0_INTFLAG_Msk;

   /* Invoke registered Callback function */
   if(rtcObj.timer32BitCallback != NULL)
   {
       rtcObj.timer32BitCallback( rtcObj.timer32intCause, rtcObj.context );
   }
}
