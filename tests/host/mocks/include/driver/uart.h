/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#define UART_NUM_2 2
#define UART_PIN_NO_CHANGE (-1)
enum { UART_DATA_8_BITS, UART_PARITY_DISABLE, UART_STOP_BITS_1, UART_HW_FLOWCTRL_DISABLE };
typedef struct { uint32_t baud_rate; int data_bits, parity, stop_bits, flow_ctrl, rx_flow_ctrl_thresh; } uart_config_t;
esp_err_t uart_driver_install(int port, int rx_size, int tx_size, int queue_size, void *queue, int flags);
esp_err_t uart_driver_delete(int port);
esp_err_t uart_set_pin(int port, int tx, int rx, int rts, int cts);
esp_err_t uart_param_config(int port, const uart_config_t *config);
esp_err_t uart_get_buffered_data_len(int port, size_t *length);
int uart_read_bytes(int port, void *buffer, size_t size, TickType_t timeout);
int uart_write_bytes(int port, const void *buffer, size_t size);
esp_err_t uart_wait_tx_done(int port, TickType_t timeout);
esp_err_t uart_flush_input(int port);