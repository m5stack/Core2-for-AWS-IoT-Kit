/*
 * Copyright 2025 Espressif Systems (Shanghai) CO LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string.h>
#include "ecu_console_interface.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#if SOC_UART_SUPPORTED
#include <driver/uart.h>
#include "hal/uart_types.h"
#endif

#if SOC_USB_SERIAL_JTAG_SUPPORTED
#include "driver/usb_serial_jtag.h"
#endif

static const char *TAG = "ECU Console Interface";
const char *prompt = "Initializing Command line: >>";
static ecu_console_interface_t *console_interface = NULL;
#define ECU_CONSOLE_INTERFACE_TX_BUFFER_SIZE 2048
#define ECU_CONSOLE_INTERFACE_RX_BUFFER_SIZE 2048
#define ECU_CONSOLE_INTERFACE_TIMEOUT 1000

#if SOC_UART_SUPPORTED
#define ECU_UART_NUM UART_NUM_0
#define ECU_UART_INTR_ALLOC_FLAGS 0
#define ECU_UART_BAUD_RATE 115200
static esp_err_t uart_install()
{
    esp_err_t ret;

    // Install UART driver
    ret = uart_driver_install(ECU_UART_NUM, ECU_CONSOLE_INTERFACE_RX_BUFFER_SIZE, ECU_CONSOLE_INTERFACE_TX_BUFFER_SIZE, 0, NULL, ECU_UART_INTR_ALLOC_FLAGS);
    if (ret != ESP_OK) {
        return ret;
    }

    // Configure UART parameters (baud rate, data bits, stop bits, etc.)
    uart_config_t uart_config = {
        .baud_rate = ECU_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ret = uart_param_config(ECU_UART_NUM, &uart_config);
    if (ret != ESP_OK) {
        uart_driver_delete(ECU_UART_NUM);
        return ret;
    }

    // Set UART pins (use default pins if not specified)
    ret = uart_set_pin(ECU_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        uart_driver_delete(ECU_UART_NUM);
        return ret;
    }

    // Note: We flush RX buffer right before reading, not here, to ensure we clear any stale data
    // right before we expect to receive the "version" command

    return ESP_OK;
}

static esp_err_t uart_uninstall()
{
    return uart_driver_delete(ECU_UART_NUM);
}

static int uart_read(uint8_t *buf, size_t length, TickType_t ticks_to_wait)
{
    return uart_read_bytes(ECU_UART_NUM, buf, length, ticks_to_wait);
}

static int uart_write(const char *buf, size_t length, TickType_t ticks_to_wait)
{
    return uart_write_bytes(ECU_UART_NUM, buf, length); // Added ticks_to_wait
}
static esp_err_t ecu_uart_wait_tx_done(TickType_t ticks_to_wait)
{
    return uart_wait_tx_done(ECU_UART_NUM, ticks_to_wait);
}
#endif

ecu_console_interface_t ecu_console_interface_uart = {
#if SOC_UART_SUPPORTED
    .type = ECU_CONSOLE_INTERFACE_UART,
    .install = uart_install,
    .uninstall = uart_uninstall,
    .read_bytes = uart_read,
    .write_bytes = uart_write,
    .wait_tx_done = ecu_uart_wait_tx_done,
#else
    .type = ECU_CONSOLE_INTERFACE_NONE,
#endif
};

#if SOC_USB_SERIAL_JTAG_SUPPORTED
static esp_err_t usb_serial_install() {
    usb_serial_jtag_driver_config_t jtag_config = {
        .tx_buffer_size = ECU_CONSOLE_INTERFACE_TX_BUFFER_SIZE,
        .rx_buffer_size = ECU_CONSOLE_INTERFACE_RX_BUFFER_SIZE,
    };
    return usb_serial_jtag_driver_install(&jtag_config);
}

static int usb_serial_read(uint8_t *buf, size_t length, TickType_t ticks_to_wait)
{
    return usb_serial_jtag_read_bytes(buf, length, ticks_to_wait);
}

static int usb_serial_write(const char *buf, size_t length, TickType_t ticks_to_wait)
{
    return usb_serial_jtag_write_bytes(buf, length, ticks_to_wait);
}

static esp_err_t usb_serial_uninstall()
{
    return usb_serial_jtag_driver_uninstall();
}
static esp_err_t ecu_usb_serial_wait_tx_done(TickType_t ticks_to_wait)
{
    return usb_serial_jtag_wait_tx_done(ticks_to_wait);
}
#endif

ecu_console_interface_t ecu_console_interface_usb = {
#if SOC_USB_SERIAL_JTAG_SUPPORTED
    .type = ECU_CONSOLE_INTERFACE_USB,
    .install = usb_serial_install,
    .uninstall = usb_serial_uninstall,
    .read_bytes = usb_serial_read,
    .write_bytes = usb_serial_write,
    .wait_tx_done = ecu_usb_serial_wait_tx_done,
#else
    .type = ECU_CONSOLE_INTERFACE_NONE,
#endif
};

void print_console_interface(void)
{
    if (console_interface != NULL) {
        if (console_interface->type == ECU_CONSOLE_INTERFACE_UART) {
            ESP_LOGI(TAG, "Console is running on UART0");
        } else if (console_interface->type == ECU_CONSOLE_INTERFACE_USB) {
            ESP_LOGI(TAG, "Console is running on USB Serial JTAG");
        }
    } else {
        ESP_LOGI(TAG, "No console interface is configured");
    }
}

esp_err_t ecu_initialize_console_interface(void)
{
    esp_err_t esp_ret = ESP_FAIL;
    int ret = 0;
    int ret_usb = -1;
    int ret_uart = -1;
    char linebuf[8];
    char linebuf_usb[8];
    char linebuf_uart[8];
    bool usb_tried = false;
    bool uart_tried = false;

    ESP_LOGI(TAG, "Free heap: %ld bytes", esp_get_free_heap_size());

#if SOC_USB_SERIAL_JTAG_SUPPORTED
    console_interface = &ecu_console_interface_usb;
    esp_ret = console_interface->install();
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install USB Serial JTAG driver");
        return esp_ret;
    }
    bzero(linebuf, sizeof(linebuf));
    // Write prompt directly to the interface being tested, not via printf (which goes to console)
    console_interface->write_bytes(prompt, strlen(prompt), portMAX_DELAY);
    console_interface->wait_tx_done(pdMS_TO_TICKS(100));  // Wait for transmission to complete
    // Small delay to give host time to see prompt and respond
    vTaskDelay(pdMS_TO_TICKS(100));  // Increased delay to ensure host has time to send
    ret = console_interface->read_bytes((uint8_t *)linebuf, strlen("version"), pdMS_TO_TICKS(ECU_CONSOLE_INTERFACE_TIMEOUT));
    if (ret == strlen("version")) {
        if (memcmp(linebuf, "version", strlen("version")) == 0) {
            // Write version response directly to the interface
            console_interface->write_bytes(PROJECT_VER, strlen(PROJECT_VER), portMAX_DELAY);
            console_interface->write_bytes("\n", 1, portMAX_DELAY);
            ESP_LOGI(TAG, "USB Serial JTAG interface successfully initialized");
            return ESP_OK;
        }
    }
    // USB failed, save debug info but don't print yet (will print if UART also fails)
    ret_usb = ret;
    memcpy(linebuf_usb, linebuf, sizeof(linebuf));
    usb_tried = true;
    esp_ret = console_interface->uninstall();
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to uninstall USB Serial JTAG driver");
        return esp_ret;
    }
#endif

#if SOC_UART_SUPPORTED
    console_interface = &ecu_console_interface_uart;
    esp_ret = console_interface->install();
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver");
        // UART installation failed, can't try UART, so print USB debug info if USB was tried
        if (usb_tried) {
            ESP_LOGE(TAG, "USB Serial JTAG read_bytes returned: %d (expected: %d)", ret_usb, (int)strlen("version"));
            ESP_LOGE(TAG, "USB Serial JTAG received data (hex):");
            for (int i = 0; i < sizeof(linebuf_usb); i++) {
                ESP_LOGE(TAG, "  [%d] = 0x%02x ('%c')", i, (unsigned char)linebuf_usb[i],
                         (linebuf_usb[i] >= 32 && linebuf_usb[i] < 127) ? linebuf_usb[i] : '.');
            }
        }
        return esp_ret;
    }
    bzero(linebuf, sizeof(linebuf));

    // Flush RX buffer BEFORE sending prompt to clear any stale data
    uart_flush(ECU_UART_NUM);

    // Write prompt directly to the interface being tested, not via printf (which goes to console)
    console_interface->write_bytes(prompt, strlen(prompt), portMAX_DELAY);
    console_interface->wait_tx_done(pdMS_TO_TICKS(100));  // Wait for transmission to complete

    // Small delay to give host time to see prompt and respond
    vTaskDelay(pdMS_TO_TICKS(100));  // Increased delay to ensure host has time to send
    ret = console_interface->read_bytes((uint8_t *)linebuf, strlen("version"), pdMS_TO_TICKS(ECU_CONSOLE_INTERFACE_TIMEOUT));
    if (ret == strlen("version")) {
        if (memcmp(linebuf, "version", strlen("version")) == 0) {
            // Write version response directly to the interface
            console_interface->write_bytes(PROJECT_VER, strlen(PROJECT_VER), portMAX_DELAY);
            console_interface->write_bytes("\n", 1, portMAX_DELAY);
            ESP_LOGI(TAG, "UART interface successfully initialized");
            return ESP_OK;
        }
    }
    // UART failed, save debug info
    ret_uart = ret;
    memcpy(linebuf_uart, linebuf, sizeof(linebuf));
    uart_tried = true;
    esp_ret = console_interface->uninstall();
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to uninstall UART driver");
        return esp_ret;
    }
#endif

    // Both interfaces failed, print debug info for both
    ESP_LOGE(TAG, "Failed to initialize ECU console interface");
    if (usb_tried) {
        ESP_LOGE(TAG, "USB Serial JTAG read_bytes returned: %d (expected: %d)", ret_usb, (int)strlen("version"));
        ESP_LOGE(TAG, "USB Serial JTAG received data (hex):");
        for (int i = 0; i < sizeof(linebuf_usb); i++) {
            ESP_LOGE(TAG, "  [%d] = 0x%02x ('%c')", i, (unsigned char)linebuf_usb[i],
                     (linebuf_usb[i] >= 32 && linebuf_usb[i] < 127) ? linebuf_usb[i] : '.');
        }
    }
    if (uart_tried) {
        ESP_LOGE(TAG, "UART read_bytes returned: %d (expected: %d)", ret_uart, (int)strlen("version"));
        ESP_LOGE(TAG, "UART received data (hex):");
        for (int i = 0; i < sizeof(linebuf_uart); i++) {
            ESP_LOGE(TAG, "  [%d] = 0x%02x ('%c')", i, (unsigned char)linebuf_uart[i],
                     (linebuf_uart[i] >= 32 && linebuf_uart[i] < 127) ? linebuf_uart[i] : '.');
        }
    }
    return ESP_FAIL;
}

ecu_console_interface_t *get_console_interface(void)
{
    if (console_interface == NULL) {
        ESP_LOGE(TAG, "Console interface is not initialized");
        return NULL;
    }
    return console_interface;
}
