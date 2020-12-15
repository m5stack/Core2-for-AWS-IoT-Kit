/**
 *
 * \file
 *
 * \brief Serial Bridge.
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

 /** \mainpage
 * \section intro Introduction
 * This example demonstrates how to implement a EDBG UART to WINC3400 SPI
 * bridge.<br>
 * It uses the following hardware:
 * - the SAM Xplained Pro.
 * - the WINC3400 on EXT1.
 *
 * \section files Main Files
 * - serial_bridge.c : Initialize the WINC3400 perform bridge operation.
 *
 * \section usage Usage
 * -# Build the program and download it into the board.
 * -# Run download_all.bat script in firmware_update_project
 *
 * \section compinfo Compilation Information
 * This software was written for the GNU GCC compiler using Atmel Studio 6.2
 * Other compilers are not guaranteed to work.
 *
 * \section contactinfo Contact Information
 * For further information, visit
 * <A href="http://www.microchip.com">Microchip</A>.\n
 */

#include <asf.h>
#include "common/include/nm_common.h"
#include "bus_wrapper/include/nm_bus_wrapper.h"
#include "driver/source/nmbus.h"
#include "driver/include/m2m_wifi.h"
#include "buffered_uart_ultra.h"
#include <string.h>
#include "driver/source/nmasic.h"

tstrWifiInitParam gstrWifiParam;

struct nm_command {
	uint16_t cmd;
	uint16_t size;
	uint32_t addr;
	uint32_t val;
} __attribute__ ((packed));

struct serial_bridge_frame {
	uint8_t op_code;
	uint8_t cmd[sizeof(struct nm_command)];
} __attribute__ ((packed));


#define PACKET_QUEUE_BUFFER_SIZE 2048
struct serial_bridge_frame_buffer {
	uint8_t buffer[PACKET_QUEUE_BUFFER_SIZE];
	uint32_t size;
};

static struct serial_bridge_frame_buffer frame_buffer;

static volatile uint32 ms_systicks;

void SysTick_Handler(void)
{
	ms_systicks++;
}

uint32 get_systicks(void)
{
	return ms_systicks;
}

static void serial_bridge_frame_buffer_pop(struct serial_bridge_frame_buffer *frame, int size)
{
	if (size < 0 || size > frame->size) {
		size = frame->size;
	}
	frame->size -= size;
	if (frame->size) {
		memmove(frame->buffer, &frame->buffer[size], frame->size);
	}
}

static int serial_bridge_frame_receive(struct serial_bridge_frame_buffer *frame, uint16_t len)
{
	while (frame->size < len) {
		frame->size += buffered_uart_rx(&frame->buffer[frame->size], 255);
	}
	return 0;
}

/**
 * \brief Read out the first 13bytes and copy into cmd buffer after checking the frame.
 *  frame buffer pointer is pop'ed by this function.
 */
static int serial_bridge_frame_get_nm_command(struct serial_bridge_frame *frame, struct nm_command *cmd)
{
	uint8 checksum = 0;
	int i;


	for (i = 0; i < sizeof(struct nm_command); i++) {
		checksum ^= frame->cmd[i];
	}
	if (checksum == 0) {
		union {
			uint32_t i;
			char c[4];
		}
		bint = {0x01020304};

		if (bint.c[0] == 1) {
			/* Big endian. */
			cmd->cmd  = ((uint32)((frame->cmd[0]) << 8)  |\
			             (uint32)((frame->cmd[1]) << 0)  );
			cmd->size = ((uint32)((frame->cmd[2]) << 8)  |\
						 (uint32)((frame->cmd[3]) << 0)  );
			cmd->addr = ((uint32)((frame->cmd[4]) << 24) |\
			             (uint32)((frame->cmd[5]) << 16) |\
						 (uint32)((frame->cmd[6]) << 8)  |\
						 (uint32)((frame->cmd[7]) << 0)  );
			cmd->val  = ((uint32)((frame->cmd[8]) << 24) |\
			             (uint32)((frame->cmd[9]) << 16) |\
						 (uint32)((frame->cmd[10])<< 8)  |\
						 (uint32)((frame->cmd[11])<< 0)  );
		} else {
			/* Little endian. */
			memcpy(cmd, frame->cmd, sizeof(struct nm_command));
		}
	}
	serial_bridge_frame_buffer_pop(frame, sizeof(struct serial_bridge_frame));
	return checksum;
}

/**
 * \brief Handles the UART frame from nm_sync_cmd() in nmuart.c
 */
static int serial_bridge_process_sync_cmd(struct serial_bridge_frame *frame)
{
	serial_bridge_frame_buffer_pop(frame, 1);
	buffered_uart_putchar(0x5B);
	//buffered_uart_putchar(0x5C);
	return 0;
}

static int serial_bridge_process_reboot_cmd(struct serial_bridge_frame *frame)
{
	NVIC_SystemReset();
	return 0;
}

/**
 * \brief Handles the UART frame from nm_read_reg_with_ret() in nmuart.c
 */
 static int serial_bridge_process_read_reg_with_ret(struct serial_bridge_frame *frame)
{
	struct nm_command cmd;
	uint32_t val;

	if (serial_bridge_frame_get_nm_command(frame, &cmd) != 0) {
		buffered_uart_putchar(0x5A);
		return -1;
	}
	buffered_uart_putchar(0xAC);

	val = nm_read_reg(cmd.addr);

	buffered_uart_putchar((uint8_t)(val >> 24));
	buffered_uart_putchar((uint8_t)(val >> 16));
	buffered_uart_putchar((uint8_t)(val >> 8));
	buffered_uart_putchar((uint8_t)(val >> 0));
	return 0;
}

/**
 * \brief Handles the UART frame from nm_write_reg() in nmuart.c
 */
static int serial_bridge_process_write_reg(struct serial_bridge_frame *frame)
{
	struct nm_command cmd;

	if (serial_bridge_frame_get_nm_command(frame, &cmd) != 0) {
		buffered_uart_putchar(0x5A);
		return -1;
	}
	buffered_uart_putchar(0xAC);

	nm_write_reg(cmd.addr, cmd.val);
	return 0;
}

/**
 * \brief Handles the UART frame from nm_read_block() in nmuart.c
 */
static int serial_bridge_process_read_block(struct serial_bridge_frame *frame)
{
	struct nm_command cmd;
	uint8_t reg_buffer[PACKET_QUEUE_BUFFER_SIZE];

	if (serial_bridge_frame_get_nm_command(frame, &cmd) != 0) {
		buffered_uart_putchar(0x5A);
		return -1;
	}
	buffered_uart_putchar(0xAC);

	nm_read_block(cmd.addr, reg_buffer, cmd.size);
	buffered_uart_tx(reg_buffer, cmd.size);
	return 0;
}

/**
 * \brief Handles frame from nm_uart_write_block() in nmuart.c
 */
static int serial_bridge_process_write_block(struct serial_bridge_frame *frame)
{
	struct nm_command cmd;
	sint8 r;

	if (serial_bridge_frame_get_nm_command(frame, &cmd) != 0) {
		buffered_uart_putchar(0x5A);
		return -1;
	}
	buffered_uart_putchar(0xAC);

	serial_bridge_frame_receive(frame, cmd.size);
	r = nm_write_block(cmd.addr, (uint8_t*)frame, cmd.size);
	serial_bridge_frame_buffer_pop(frame, cmd.size);
	buffered_uart_putchar(r? 0x5A : 0xAC);

	return 0;
}

/**
 * \brief Handles the UART frame from nm_reconfigure() in nmuart.c
 */
static int serial_bridge_process_reconfigure(struct serial_bridge_frame *frame)
{
	struct nm_command cmd;

	if (serial_bridge_frame_get_nm_command(frame, &cmd) != 0) {
		buffered_uart_putchar(0x5A);
		return -1;
	}

	buffered_uart_putchar(0xAC);
	delay_ms(1);
	buffered_uart_term();
	buffered_uart_init(cmd.val);
	return 0;
}

struct serial_bridge_frame_lookup {
	uint8_t header[4];
	uint32_t header_size;
	uint32_t min_size;
	int (*handler)(struct serial_bridge_frame *);
};

struct serial_bridge_frame_lookup serial_bridge_lookup_table[] = {
	{{0x12      }, 1, 1,                                  serial_bridge_process_sync_cmd         },
	{{0x13      }, 1, 1,                                  serial_bridge_process_reboot_cmd       },
	{{0xa5, 0x00}, 2, sizeof(struct serial_bridge_frame), serial_bridge_process_read_reg_with_ret},
	{{0xa5, 0x01}, 2, sizeof(struct serial_bridge_frame), serial_bridge_process_write_reg        },
	{{0xa5, 0x02}, 2, sizeof(struct serial_bridge_frame), serial_bridge_process_read_block       },
	{{0xa5, 0x03}, 2, sizeof(struct serial_bridge_frame), serial_bridge_process_write_block      },
	{{0xa5, 0x05}, 2, sizeof(struct serial_bridge_frame), serial_bridge_process_reconfigure      },
};

#define SERIAL_BRIDGE_LOOKUP_TABLE_ENTRIES sizeof(serial_bridge_lookup_table) / sizeof(struct serial_bridge_frame_lookup)


static void serial_bridge_process_frame(void)
{
	uint32_t i;

	for (i = 0; i < SERIAL_BRIDGE_LOOKUP_TABLE_ENTRIES; i++) {
		if (frame_buffer.size >= serial_bridge_lookup_table[i].min_size
		&&  !memcmp(serial_bridge_lookup_table[i].header, frame_buffer.buffer, serial_bridge_lookup_table[i].header_size)) {
			if (serial_bridge_lookup_table[i].handler((struct serial_bridge_frame*)(frame_buffer.buffer)) == 0) {
				return;
			} else {
				break;
			}
		}
	}

	if (frame_buffer.buffer[0] == 0xFF) {
		serial_bridge_frame_buffer_pop(&frame_buffer, -1);
	} else if (frame_buffer.buffer[0] != 0x12 && frame_buffer.buffer[0] != 0xA5) {
		buffered_uart_putchar(0xEA); //??
		serial_bridge_frame_buffer_pop(&frame_buffer, -1);
	}
}

sint8 enter_wifi_firmware_download(void)
{
	int n;

	if (cpu_start() != M2M_SUCCESS) {
		puts("Failed to put the WiFi Chip in download mode!\n");
		return M2M_ERR_INIT;
	}

	frame_buffer.size = 0;
	memset(frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));

	serial_bridge_process_sync_cmd(&frame_buffer);

	while (1) {
		n = buffered_uart_rx(&frame_buffer.buffer[frame_buffer.size], 255);
		if (n) {
		    frame_buffer.size += n;
			serial_bridge_process_frame();
		}
	}
	return 0;
}

/**
 * \brief Main application function.
 *
 * Application entry point.
 *
 * \return program return value.
 */
int main (void)
{
    sint8 ret = M2M_SUCCESS;

	/* Initialize the system. */
    system_init();

    /* Initialize the UART console. */
    buffered_uart_init(115200);

	/* Initialize WINC IOs. */
    ret = nm_bsp_init();
    if(ret != M2M_SUCCESS) {
	    M2M_ERR("\r\nFailed to initialize BSP.");
	    goto HALT;
    }

	/* Initialize WINC driver. */
	ret = m2m_wifi_init(&gstrWifiParam);
	if(ret != M2M_SUCCESS) {
	    M2M_ERR("\r\nFailed to initialize WINC driver.");
	    goto HALT;
    }

    if(SysTick_Config(system_cpu_clock_get_hz() / 1000))
    puts("main: SysTick configuration error!");

	/* Enter WiFi firmware download mode. */
    enter_wifi_firmware_download();

    HALT:
    while(1) {
	    continue;
    }
}
