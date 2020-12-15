/**
 * \file
 *
 * \brief Application implement
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "atmel_start.h"
#include <hal_gpio.h>
#include <hal_delay.h>
#include "cryptoauthlib.h"
#include "aws_wifi_task.h"
#include "timer_interface.h"


extern volatile uint32_t g_timer_val;

struct timer_task TIMER_0_mqtt_timer;
static void TIMER_0_mqtt_timer_cb(const struct timer_task *const timer_task)
{
	g_timer_val += 100;
}

int main(void)
{
	atmel_start_init();
	__enable_irq();

	gpio_set_pin_direction(LED0, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(LED0, 1);
	timer_start(&TIMER_0);
	TIMER_0_mqtt_timer.interval = 100;
	TIMER_0_mqtt_timer.cb       = TIMER_0_mqtt_timer_cb;
	TIMER_0_mqtt_timer.mode     = TIMER_TASK_REPEAT;
	timer_add_task(&TIMER_0, &TIMER_0_mqtt_timer);
	
	aws_wifi_task(NULL);
	while(1);
}
