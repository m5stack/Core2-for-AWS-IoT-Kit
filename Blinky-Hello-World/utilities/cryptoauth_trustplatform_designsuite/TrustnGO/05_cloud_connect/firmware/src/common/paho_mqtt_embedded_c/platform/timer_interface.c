/**
 *
 * \file
 *
 * \brief Platform timer interface
 *
 * Copyright (c) 2016-2017 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */


#include "timer_interface.h"

volatile uint32_t g_timer_val;

/**
 * \brief Get current value of RTT timer
 *
 * \param[out] time          Set time values in seconds and microseconds
 
 * \return  Whether the function was successful
 *            0  - The function was successful
 *            -1 - The function was not successful
 */
int get_time_of_day(struct timeval *time)
{
	uint32_t tick_ms = 0;

	if (time == NULL)
    {
        return -1;
    }
        
	tick_ms = g_timer_val;
	time->tv_sec =  (tick_ms / 1000);
	time->tv_usec = ((tick_ms % 1000) * 1000);

	return 0;
}

/**
 * \brief Initialize a timer
 *
 * \param[out] timer       The timer to be initialized
 */
void TimerInit(Timer *timer)
{
    if (timer == NULL)
    {
        return;
    }
    
	timer->end_time.tv_sec  = 0;
	timer->end_time.tv_usec = 0;
}

/**
 * \brief Check if a timer is expired
 *
 * \param[in] timer        The timer to be checked for expiration
 *
 * \return  Whether the timer has expired
 *            True  - The timer has expired
 *            False - The timer has not expired
 */
char TimerIsExpired(Timer *timer)
{
	struct timeval time_now;
	struct timeval time_result;

    if (timer == NULL)
    {
        return 1;
    }

	get_time_of_day(&time_now);
    
	timer_subtract(&timer->end_time, &time_now, &time_result);

	return (time_result.tv_sec < 0 || (time_result.tv_sec == 0 && time_result.tv_usec <= 0));
}

/**
 * \brief Create a timer this is set to expire in a specified number of 
 *        milliseconds
 *
 * \param[out] timer       The timer set to expire in the specified number of
 *                         milliseconds
 * \param[in] timeout_ms   The timer expiration (in milliseconds)
 */
void TimerCountdownMS(Timer *timer, unsigned int timeout_ms)
{
	struct timeval time_now;
    struct timeval time_interval = {timeout_ms / 1000, (int)((timeout_ms % 1000) * 1000)};

    if (timer == NULL)
    {
        return;
    }

	get_time_of_day(&time_now);
    
	timer_add(&time_now, &time_interval, &timer->end_time);
}

/**
 * \brief Create a timer this is set to expire in a specified number of seconds
 *
 * \param[out] timer       The timer set to expire in the specified number of
 *                         seconds
 * \param[in] timeout_ms   The timer expiration (in seconds)
 */
void TimerCountdown(Timer *timer, unsigned int timeout)
{
	struct timeval time_now;
	struct timeval time_interval = {timeout, 0};

    if (timer == NULL)
    {
        return;
    }

	get_time_of_day(&time_now);

	timer_add(&time_now, &time_interval, &timer->end_time);
}

/**
 * \brief Check the time remaining on a given timer
 *
 * \param[out] timer       The timer to be set to checked
 *
 * \return  The number of milliseconds left on the countdown timer
 */
int TimerLeftMS(Timer *timer)
{
	int result_ms = 0;
	struct timeval time_now;
    struct timeval time_result;

    if (timer == NULL)
    {
        return 0;
    }

	get_time_of_day(&time_now);
    
	timer_subtract(&timer->end_time, &time_now, &time_result);
	if(time_result.tv_sec >= 0)
    {
		result_ms = (int)((time_result.tv_sec * 1000) + (time_result.tv_usec / 1000));
	}

	return result_ms;
}
