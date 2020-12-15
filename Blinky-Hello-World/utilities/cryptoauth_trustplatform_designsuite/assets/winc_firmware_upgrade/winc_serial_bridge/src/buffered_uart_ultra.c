/**
 *
 * \file
 *
 * \brief SAM D21 UART API Implementations.
 *
 * Copyright (c) 2019 Microchip Technology Inc. and its subsidiaries.
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
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
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

#include "asf.h"
#include <stdio.h>
#include <string.h>
#include "buffered_uart_ultra.h"

static uint8_t serial_rx_buf[SERIAL_RX_BUF_SIZE_HOST];

static uint8_t serial_rx_buf_head;

static uint8_t serial_rx_buf_tail;

static struct usart_module host_uart_module;

void buffered_uart_init(uint32_t baudrate)
{
	serial_rx_buf_head = 0;
	serial_rx_buf_tail = 0;

	/* Configure the UART console. */
	struct usart_config host_uart_config;
	usart_get_config_defaults(&host_uart_config);
	host_uart_config.mux_setting = HOST_SERCOM_MUX_SETTING;
	host_uart_config.pinmux_pad0 = HOST_SERCOM_PINMUX_PAD0;
	host_uart_config.pinmux_pad1 = HOST_SERCOM_PINMUX_PAD1;
	host_uart_config.pinmux_pad2 = HOST_SERCOM_PINMUX_PAD2;
	host_uart_config.pinmux_pad3 = HOST_SERCOM_PINMUX_PAD3;
	host_uart_config.baudrate    = baudrate;
	stdio_serial_init(&host_uart_module, USART_HOST, &host_uart_config);
	usart_enable(&host_uart_module);

	USART_HOST_RX_ISR_ENABLE();
}

void buffered_uart_term(void)
{
	usart_disable(&host_uart_module);
}

void buffered_uart_putchar(uint8_t ch)
{
	USART_HOST->USART.DATA.reg = ch;
	while (!(USART_HOST->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_TXC));
}

uint8_t buffered_uart_tx(uint8_t *data, uint32_t length)
{
	uint8_t status = 0;
	uint32_t written = 0;

	do {
		while (length != written) {
			buffered_uart_putchar(*data);
			data++;
			written++;
		}
	} while (status != STATUS_OK);
	return length;
}

uint8_t buffered_uart_rx(uint8_t *data, uint8_t max_len)
{
	uint8_t copy_len = 0;
	uint8_t data_len = 0;

	if (serial_rx_buf_tail != serial_rx_buf_head) {
		cpu_irq_disable();

		if (serial_rx_buf_tail >= serial_rx_buf_head) {
			copy_len = serial_rx_buf_tail - serial_rx_buf_head;
		} else {
			copy_len = serial_rx_buf_tail + (SERIAL_RX_BUF_SIZE_HOST - serial_rx_buf_head);
		}

		if (copy_len > max_len) {
			copy_len = max_len;
		}

		if (copy_len) {
			if (SERIAL_RX_BUF_SIZE_HOST < (copy_len + serial_rx_buf_head)) {
				data_len = SERIAL_RX_BUF_SIZE_HOST - serial_rx_buf_head;
				memcpy(data, &serial_rx_buf[serial_rx_buf_head], data_len);
				data += data_len;
				copy_len -= data_len;
				serial_rx_buf_head = 0;
			}
			memcpy(data, &serial_rx_buf[serial_rx_buf_head], copy_len);
			data_len += copy_len;
			serial_rx_buf_head += copy_len;
		}
		cpu_irq_enable();
	}
	return data_len;
}

int buffered_uart_getchar_nowait(void)
{
	int c = -1;
	cpu_irq_disable();
	if (serial_rx_buf_tail != serial_rx_buf_head) {
		c = serial_rx_buf[serial_rx_buf_head];
		serial_rx_buf_head++;
		if (serial_rx_buf_head == SERIAL_RX_BUF_SIZE_HOST) {
			serial_rx_buf_head = 0;
		}
	}
	cpu_irq_enable();
	return c;
}

uint8_t buffered_uart_getchar(void)
{
	int c;
	do {
		c = buffered_uart_getchar_nowait();
	} while (c < 0);
	return (uint8_t)c;
}

void USART_HOST_ISR_VECT(void)
{
	while (USART_HOST->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_RXC) {
		serial_rx_buf[serial_rx_buf_tail] = USART_HOST->USART.DATA.reg;

		if ((SERIAL_RX_BUF_SIZE_HOST - 1) == serial_rx_buf_tail) {
			serial_rx_buf_tail = 0x00;
		} else {
			serial_rx_buf_tail++;
		}
	}
}
