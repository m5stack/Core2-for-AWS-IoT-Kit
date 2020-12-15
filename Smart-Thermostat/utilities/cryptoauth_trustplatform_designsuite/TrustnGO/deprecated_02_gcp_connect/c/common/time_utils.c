/**
 * \file
 * \brief  Date/Time Utilities (RTC)
 *
 * \copyright (c) 2017 Microchip Technology Inc. and its subsidiaries.
 *            You may use this software and any derivatives exclusively with
 *            Microchip products.
 *
 * \page License
 * 
 * (c) 2017 Microchip Technology Inc. and its subsidiaries. You may use this
 * software and any derivatives exclusively with Microchip products.
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
 * 
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIPS TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 * 
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
 * TERMS.
 */

#include "time_utils.h"
#include "atmel_start.h"

/* Globals */
static bool g_time_set;


uint32_t time_utils_convert(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second)
{
    uint32_t ret = 0;

    //January and February are counted as months 13 and 14 of the previous year
    if(month <= 2)
    {
        month += 12;
        year -= 1;
    }
     
    //Convert years to days
    ret = (365 * year) + (year / 4) - (year / 100) + (year / 400);
    //Convert months to days
    ret += (30 * month) + (3 * (month + 1) / 5) + day;
    //Unix time starts on January 1st, 1970
    ret -= 719561;
    //Convert days to seconds
    ret *= 86400;
    //Add hours, minutes and seconds
    ret += (3600 * hour) + (60 * minute) + second;
     
    return ret;
}

uint32_t time_utils_get_utc(void)

{
	struct calendar_date_time date_time;
    if(g_time_set)
    {

	   calendar_get_date_time(&CALENDAR_0,&date_time);
        
        return time_utils_convert(date_time.date.year, date_time.date.month, date_time.date.day, date_time.time.hour,date_time.time.min,date_time.time.sec);
	}
}

void time_utils_set(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second)
{
		
	/* Set current time. */
	struct calendar_date date;

	date.year  = year;
	date.month = month;
	date.day   = day;
	struct calendar_time time;
	time.hour = hour;
	time.min  = minute;
	time.sec  = second;
		
	calendar_set_time(&CALENDAR_0, &time);
	calendar_set_date(&CALENDAR_0, &date);

    g_time_set = true;
}
