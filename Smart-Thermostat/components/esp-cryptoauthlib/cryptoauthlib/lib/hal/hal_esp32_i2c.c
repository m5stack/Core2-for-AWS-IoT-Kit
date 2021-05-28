/*
 * Copyright 2018 Espressif Systems (Shanghai) PTE LTD
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <driver/i2c.h>
#include "hal/atca_hal.h"
#include "esp_err.h"
#include "esp_log.h"
#include "i2c_device.h"

#define SDA_PIN                            21 
#define SCL_PIN                            22
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL                    ESP_LOG_INFO
#endif

#define MAX_I2C_BUSES 2  //ESP32 has 2 I2C bus

I2CDevice_t i2c_device_bus = NULL;

static const char *TAG = "HAL_I2C";

void hal_i2c_change_baud(ATCAIface iface, uint32_t speed) {
    i2c_device_change_freq(i2c_device_bus, speed);
}

ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg)
{
    esp_err_t rc;
    int bus = cfg->atcai2c.bus;
    i2c_device_bus = i2c_malloc_device(bus, SDA_PIN, SCL_PIN, cfg->atcai2c.baud, cfg->atcai2c.slave_address >> 1);

    if (i2c_device_bus == NULL) {
        return ATCA_COMM_FAIL;
    } else {
        return ATCA_SUCCESS;
    }
}

ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t *txdata, int txlength)
{
    ATCAIfaceCfg *cfg = iface->mIfaceCFG;
    esp_err_t rc;

    txdata[0] = 0x03;              //Word Address value, Command Token as per datasheet of ATECC508A
    txlength++;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    (void)i2c_master_start(cmd);
    (void)i2c_master_write_byte(cmd, cfg->atcai2c.slave_address | I2C_MASTER_WRITE, ACK_CHECK_EN);
    (void)i2c_master_write(cmd, txdata, txlength, ACK_CHECK_EN);
    (void)i2c_master_stop(cmd);

    rc = i2c_master_cmd_begin(cfg->atcai2c.bus, cmd, 10);

    (void)i2c_cmd_link_delete(cmd);

    if (ESP_OK != rc) {
        return ATCA_COMM_FAIL;
    } else {
        return ATCA_SUCCESS;
    }
}

ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t *rxdata, uint16_t *rxlength)
{
    ATCAIfaceCfg *cfg = iface->mIfaceCFG;
    esp_err_t rc;
    i2c_cmd_handle_t cmd;
    int high = 0;
    int low = 0;

    cmd = i2c_cmd_link_create();
    (void)i2c_master_start(cmd);
    (void)i2c_master_write_byte(cmd, cfg->atcai2c.slave_address | I2C_MASTER_READ, ACK_CHECK_EN);
    (void)i2c_master_read_byte(cmd, rxdata, ACK_VAL);

    rc = i2c_master_cmd_begin(cfg->atcai2c.bus, cmd, 10);

    (void)i2c_cmd_link_delete(cmd);

    if (ESP_OK != rc)
    {
        return ATCA_COMM_FAIL;
    }

    *rxlength = rxdata[0];

    if (*rxlength > 1)
    {
        cmd = i2c_cmd_link_create();
        if (*rxlength > 2)
        {
            (void)i2c_master_read(cmd, &rxdata[1], (*rxlength) - 2, ACK_VAL);
        }
        (void)i2c_master_read_byte(cmd, rxdata + (*rxlength) - 1, NACK_VAL);
        (void)i2c_master_stop(cmd);
        rc = i2c_master_cmd_begin(cfg->atcai2c.bus, cmd, 10);
        (void)i2c_cmd_link_delete(cmd);
    }
    else
    {
        cmd = i2c_cmd_link_create();
        (void)i2c_master_stop(cmd);
        rc = i2c_master_cmd_begin(cfg->atcai2c.bus, cmd, 10);
        (void)i2c_cmd_link_delete(cmd);
    }

//    ESP_LOG_BUFFER_HEX(TAG, rxdata, *rxlength);

    if (ESP_OK != rc)
    {
        return ATCA_COMM_FAIL;
    }
    else
    {
        return ATCA_SUCCESS;
    }
}

ATCA_STATUS hal_i2c_release(void *hal_data)
{
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_wake(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);

//    uint32_t bdrt = cfg->atcai2c.baud;
    uint16_t rxlen;
    uint8_t data[4] = { 0 };
    const uint8_t expected[4] = { 0x04, 0x11, 0x33, 0x43 };
    i2c_apply_bus(i2c_device_bus);
//    if (bdrt != 100000) {
//        hal_i2c_change_baud(iface, 100000);
//    }

    // 0x00 as wake up pulse
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    (void)i2c_master_start(cmd);
    (void)i2c_master_write_byte(cmd, I2C_MASTER_WRITE, ACK_CHECK_DIS);
    (void)i2c_master_stop(cmd);
    (void)i2c_master_cmd_begin(cfg->atcai2c.bus, cmd, 10);
    (void)i2c_cmd_link_delete(cmd);

    atca_delay_ms(10);   // wait tWHI + tWLO which is configured based on device type and configuration structure

    rxlen = 4;

    hal_i2c_receive(iface, data, &rxlen);
    if (memcmp(data, expected, 4) == 0)
    {
        return ATCA_SUCCESS;
    }
    return ATCA_COMM_FAIL;
}

ATCA_STATUS hal_i2c_idle(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    uint8_t idle_data = 0x02;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    (void)i2c_master_start(cmd);
    (void)i2c_master_write_byte(cmd, cfg->atcai2c.slave_address | I2C_MASTER_WRITE, ACK_CHECK_EN);
    (void)i2c_master_write(cmd, &idle_data, 1, ACK_CHECK_DIS);
    (void)i2c_master_stop(cmd);
    (void)i2c_master_cmd_begin(cfg->atcai2c.bus, cmd, 10);
    (void)i2c_cmd_link_delete(cmd);
    i2c_free_bus(i2c_device_bus);
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_sleep(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    uint8_t sleep_data = 0x01;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    (void)i2c_master_start(cmd);
    (void)i2c_master_write_byte(cmd, cfg->atcai2c.slave_address | I2C_MASTER_WRITE, ACK_CHECK_EN);
    (void)i2c_master_write(cmd, &sleep_data, 1, ACK_CHECK_DIS);
    (void)i2c_master_stop(cmd);
    (void)i2c_master_cmd_begin(cfg->atcai2c.bus, cmd, 10);
    (void)i2c_cmd_link_delete(cmd);

    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_discover_buses(int i2c_buses[], int max_buses)
{
    return ATCA_UNIMPLEMENTED;
}

ATCA_STATUS hal_i2c_discover_devices(int bus_num, ATCAIfaceCfg *cfg, int *found)
{
    return ATCA_UNIMPLEMENTED;
}

