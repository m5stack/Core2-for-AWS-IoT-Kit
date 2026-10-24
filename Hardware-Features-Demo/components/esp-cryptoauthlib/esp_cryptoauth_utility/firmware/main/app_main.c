/*
 *  Key generation application
 *  Espressif MIT License
 *  Copyright 2021 Espressif Systems (Shanghai) CO LTD
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  Mbedtls License
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
#include <string.h>

#include "esp_log.h"
#include <esp_console.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "commands.h"

#include "ecu_console_interface.h"

static const char *TAG = "CMD Handler";

#define ECU_CMD_BUFFER_SIZE 1024

static void scli_loop() {
    int i, cmd_ret;
    uint8_t linebuf[ECU_CMD_BUFFER_SIZE];
    esp_err_t ret;
    while (true) {
        ecu_console_interface_t *console_interface = get_console_interface();
        ret = console_interface->write_bytes("\n>>\n", 4, portMAX_DELAY);
        if (ret < 0) {
            ESP_LOGE(TAG, "Failed to write to USB Serial JTAG");
            continue;
        }
        bzero(linebuf, sizeof(linebuf));
        i = 0;
        do {
            ret = console_interface->read_bytes((uint8_t *)&linebuf[i], 1, portMAX_DELAY);
            if (ret > 0) {
                if (linebuf[i] == '\r') {
                    ret = console_interface->write_bytes("\r\n", 2, portMAX_DELAY);
                    if (ret < 0) {
                        ESP_LOGE(TAG, "Failed to write to USB Serial JTAG");
                        break;
                    }
                } else {
                    ret = console_interface->write_bytes((char *)&linebuf[i], 1, portMAX_DELAY);
                    if (ret < 0) {
                        ESP_LOGE(TAG, "Failed to write to USB Serial JTAG");
                        break;
                    }
                }
                i++;
            }
        } while ((i < ECU_CMD_BUFFER_SIZE - 1) && linebuf[i - 1] != '\r');
        ret = console_interface->wait_tx_done(portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to wait for TX done");
            continue;
        }
        linebuf[strlen((char *)linebuf) - 1] = '\0';

        esp_console_run((char *)linebuf, &cmd_ret);
    }
}

static void scli_task(void *arg) {
    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = ECU_CMD_BUFFER_SIZE,
    };
    esp_console_init(&console_config);
    esp_console_register_help_command();
    if (register_command_handler() == ESP_OK) {
        esp_err_t ret = ecu_initialize_console_interface();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize ECU console interface");
            return;
        }
        scli_loop();
    } else {
        ESP_LOGE(TAG, "Failed to register all commands");
    }
    ESP_LOGI(TAG, "Stopping the CLI");
    vTaskDelete(NULL);
}

void app_main()
{
    BaseType_t cli_task = xTaskCreate(scli_task, "scli_task", 8 * 1024, NULL, configMAX_PRIORITIES - 5, NULL);
    if (cli_task != pdPASS) {
        ESP_LOGE(TAG, "Couldn't create scli thread");
    }
}
