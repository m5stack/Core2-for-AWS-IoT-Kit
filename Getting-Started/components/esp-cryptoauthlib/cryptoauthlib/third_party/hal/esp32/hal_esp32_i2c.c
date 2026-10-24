/*
 * Copyright 2018-2025 Espressif Systems (Shanghai) PTE LTD
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
#include <soc/soc_caps.h>

#include "esp_err.h"
#include "esp_log.h"
#include "cryptoauthlib.h"
#include "esp_idf_version.h"
#include "esp_rom_sys.h"

#if defined(CONFIG_ATCA_I2C_USE_LEGACY_DRIVER)
#include <driver/i2c.h>
#else
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0)
#include <driver/i2c_master.h>
#else
#include <driver/i2c.h>
#endif
#endif

static uint8_t I2C_SDA_PIN = CONFIG_ATCA_I2C_SDA_PIN;
static uint8_t I2C_SCL_PIN = CONFIG_ATCA_I2C_SCL_PIN;

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL                    ESP_LOG_INFO
#endif

#define MAX_I2C_BUSES SOC_I2C_NUM

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)

#if SOC_HP_I2C_NUM >= 2
#define I2C_PORT_2 I2C_NUM_1
#elif SOC_LP_I2C_NUM >= 1
#define I2C_PORT_2 LP_I2C_NUM_0
#endif // SOC_HP_I2C_NUM >= 2

#else // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)

#if SOC_I2C_NUM >= 2
#define I2C_PORT_2 I2C_NUM_1
#endif // SOC_I2C_NUM >= 2

#endif // !ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)

void hal_esp32_i2c_set_pin_config(uint8_t sda_pin, uint8_t scl_pin)
{
    I2C_SDA_PIN = sda_pin;
    I2C_SCL_PIN = scl_pin;
}

const char* TAG = "HAL_I2C";

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 2, 0) || defined(CONFIG_ATCA_I2C_USE_LEGACY_DRIVER)

// ============================================================================
// LEGACY I2C DRIVER IMPLEMENTATION (ESP-IDF < 5.2 or CONFIG_ATCA_I2C_USE_LEGACY_DRIVER)
// ============================================================================

#define ACK_CHECK_EN                       0x1  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0  /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0  /*!< I2C ack value */
#define NACK_VAL                           0x1  /*!< I2C nack value */

typedef struct atcaI2Cmaster {
    int id;
    i2c_config_t conf;
    int ref_ct;
} ATCAI2CMaster_t;

ATCAI2CMaster_t i2c_hal_data[MAX_I2C_BUSES];

ATCA_STATUS status;

/** \brief method to change the bus speed of I2C
 * \param[in] iface  interface on which to change bus speed
 * \param[in] speed  baud rate (typically 100000 or 400000)
 */
ATCA_STATUS hal_i2c_change_baud(ATCAIface iface, uint32_t speed)
{
    esp_err_t rc;
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int bus = cfg->atcai2c.bus;

    i2c_hal_data[bus].conf.master.clk_speed = speed;

    rc = i2c_param_config(i2c_hal_data[bus].id, &i2c_hal_data[bus].conf);
    if (rc == ESP_OK) {
        return ATCA_SUCCESS;
    } else {
        return ATCA_COMM_FAIL;
    }
}

/** \brief initialize an I2C interface using given config
 * \param[in] hal - opaque ptr to HAL data
 * \param[in] cfg - interface configuration
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_init(ATCAIface iface, ATCAIfaceCfg *cfg)
{
    esp_err_t rc = ESP_FAIL;
    int bus = cfg->atcai2c.bus;

    if (bus >= 0 && bus < MAX_I2C_BUSES) {
        if (0 == i2c_hal_data[bus].ref_ct) {
            i2c_hal_data[bus].ref_ct = 1;
            i2c_hal_data[bus].conf.mode = I2C_MODE_MASTER;
            i2c_hal_data[bus].conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
            i2c_hal_data[bus].conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
            i2c_hal_data[bus].conf.master.clk_speed = 100000; //cfg->atcai2c.baud;

            switch (bus) {
            case 0:
                i2c_hal_data[bus].id = I2C_NUM_0;
                break;
            case 1:
#if SOC_I2C_NUM >= 2
                i2c_hal_data[bus].id = I2C_PORT_2;
#endif
                break;
            default:
                break;
            }
            i2c_hal_data[bus].conf.sda_io_num = I2C_SDA_PIN;
            i2c_hal_data[bus].conf.scl_io_num = I2C_SCL_PIN;

            rc = i2c_param_config(i2c_hal_data[bus].id, &i2c_hal_data[bus].conf);
            rc = i2c_driver_install(i2c_hal_data[bus].id, I2C_MODE_MASTER, 0, 0, 0);
        } else {
            i2c_hal_data[bus].ref_ct++;
        }

        iface->hal_data = &i2c_hal_data[bus];
    }

    if (ESP_OK == rc) {
        return ATCA_SUCCESS;
    } else {
        return ATCA_COMM_FAIL;
    }
}

/** \brief HAL implementation of I2C post init
 * \param[in] iface  instance
 * \return ATCA_SUCCESS
 */
ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
    return ATCA_SUCCESS;
}

/** \brief HAL implementation of I2C send
 * \param[in] iface         instance
 * \param[in] word_address  device transaction type
 * \param[in] txdata        pointer to space to bytes to send
 * \param[in] txlength      number of bytes to send
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t word_address, uint8_t *txdata, int txlength)
{
    ATCAIfaceCfg *cfg = iface->mIfaceCFG;
    esp_err_t rc;
    uint8_t device_address = 0xFFu;

    if (!cfg) {
        return ATCA_BAD_PARAM;
    }

#ifdef ATCA_ENABLE_DEPRECATED
    device_address = ATCA_IFACECFG_VALUE(cfg, atcai2c.slave_address);
#else
    device_address = ATCA_IFACECFG_VALUE(cfg, atcai2c.address);
#endif

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    (void)i2c_master_start(cmd);
    (void)i2c_master_write_byte(cmd, device_address | I2C_MASTER_WRITE, ACK_CHECK_EN);
    (void)i2c_master_write_byte(cmd, word_address, ACK_CHECK_EN);

    if (NULL != txdata && 0u < txlength) {
        (void)i2c_master_write(cmd, txdata, txlength, ACK_CHECK_EN);
    }
    (void)i2c_master_stop(cmd);
    rc = i2c_master_cmd_begin(cfg->atcai2c.bus, cmd, 10);
    (void)i2c_cmd_link_delete(cmd);

    if (ESP_OK != rc) {
        return ATCA_COMM_FAIL;
    } else {
        return ATCA_SUCCESS;
    }
}

/** \brief HAL implementation of I2C receive function
 * \param[in]    iface          Device to interact with.
 * \param[in]    address        Device address
 * \param[out]   rxdata         Data received will be returned here.
 * \param[in,out] rxlength      As input, the size of the rxdata buffer.
 *                              As output, the number of bytes received.
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t address, uint8_t *rxdata, uint16_t *rxlength)
{
    ATCAIfaceCfg *cfg = iface->mIfaceCFG;
    esp_err_t rc;
    i2c_cmd_handle_t cmd;
    ATCA_STATUS status = ATCA_COMM_FAIL;

    if ((NULL == cfg) || (NULL == rxlength) || (NULL == rxdata)) {
        return ATCA_TRACE(ATCA_BAD_PARAM, "NULL pointer encountered");
    }

    cmd = i2c_cmd_link_create();
    (void)i2c_master_start(cmd);
    (void)i2c_master_write_byte(cmd, address | I2C_MASTER_READ, ACK_CHECK_EN);
    if (*rxlength > 1) {
        (void)i2c_master_read(cmd, rxdata, *rxlength - 1, ACK_VAL);
    }
    (void)i2c_master_read_byte(cmd, rxdata + (size_t)*rxlength - 1, NACK_VAL);
    (void)i2c_master_stop(cmd);
    rc = i2c_master_cmd_begin(cfg->atcai2c.bus, cmd, 10);
    (void)i2c_cmd_link_delete(cmd);

    if (ESP_OK == rc) {
        status = ATCA_SUCCESS;
    }

    return status;
}

/** \brief manages reference count on given bus and releases resource if no more references exist
 * \param[in] hal_data - opaque pointer to hal data structure - known only to the HAL implementation
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_release(void *hal_data)
{
    ATCAI2CMaster_t *hal = (ATCAI2CMaster_t*)hal_data;

    if (hal && --(hal->ref_ct) <= 0) {
        i2c_driver_delete(hal->id);
    }
    return ATCA_SUCCESS;
}

/** \brief Perform control operations for the kit protocol
 * \param[in]     iface          Interface to interact with.
 * \param[in]     option         Control parameter identifier
 * \param[in]     param          Optional pointer to parameter value
 * \param[in]     paramlen       Length of the parameter
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_control(ATCAIface iface, uint8_t option, void* param, size_t paramlen)
{
    (void)param;
    (void)paramlen;

    if (iface && iface->mIfaceCFG) {
        if (ATCA_HAL_CHANGE_BAUD == option) {
            return hal_i2c_change_baud(iface, *(uint32_t*)param);
        } else {
            return ATCA_UNIMPLEMENTED;
        }
    }
    return ATCA_BAD_PARAM;
}

#else

// ============================================================================
// NEW I2C DRIVER IMPLEMENTATION (ESP-IDF >= 5.2 and !CONFIG_ATCA_I2C_USE_LEGACY_DRIVER)
// ============================================================================

typedef struct atcaI2Cmaster {
    int port_num;
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
    uint32_t speed;
    uint8_t device_address;
    int ref_ct;
    bool initialized;
} ATCAI2CMaster_t;

ATCAI2CMaster_t i2c_hal_data[MAX_I2C_BUSES];

/** \brief method to change the bus speed of I2C
 * \param[in] iface  interface on which to change bus speed
 * \param[in] speed  baud rate (typically 100000 or 400000)
 */
ATCA_STATUS hal_i2c_change_baud(ATCAIface iface, uint32_t speed)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int bus = cfg->atcai2c.bus;

    if (bus >= MAX_I2C_BUSES || !i2c_hal_data[bus].bus_handle) {
        return ATCA_COMM_FAIL;
    }

    // Store the new speed for future device registrations
    i2c_hal_data[bus].speed = speed;

    // If device is already registered, we need to re-register with new speed
    if (i2c_hal_data[bus].dev_handle) {
        esp_err_t rc;

        // Remove old device
        rc = i2c_master_bus_rm_device(i2c_hal_data[bus].dev_handle);
        if (rc != ESP_OK) {
            return ATCA_COMM_FAIL;
        }

        // Re-add device with new speed
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = i2c_hal_data[bus].device_address,
            .scl_speed_hz = speed,
            .scl_wait_us = 0,  // Use default timing
        };

        rc = i2c_master_bus_add_device(i2c_hal_data[bus].bus_handle, &dev_cfg, &i2c_hal_data[bus].dev_handle);
        if (rc != ESP_OK) {
            return ATCA_COMM_FAIL;
        }
    }

    return ATCA_SUCCESS;
}

/** \brief initialize an I2C interface using given config
 * \param[in] hal - opaque ptr to HAL data
 * \param[in] cfg - interface configuration
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_init(ATCAIface iface, ATCAIfaceCfg *cfg)
{
    esp_err_t rc = ESP_FAIL;
    int bus = cfg->atcai2c.bus;

    if (bus >= 0 && bus < MAX_I2C_BUSES) {
        if (0 == i2c_hal_data[bus].ref_ct) {
            i2c_hal_data[bus].ref_ct = 1;
            i2c_hal_data[bus].port_num = bus;
            i2c_hal_data[bus].speed = 100000; // Standard 100kHz for ATECC608A
            i2c_hal_data[bus].initialized = false;

            // Configure I2C master bus
            i2c_master_bus_config_t bus_config = {
                .i2c_port = bus,
                .sda_io_num = I2C_SDA_PIN,
                .scl_io_num = I2C_SCL_PIN,
                .clk_source = I2C_CLK_SRC_DEFAULT,
                .glitch_ignore_cnt = 7,
                .intr_priority = 0,
                .trans_queue_depth = 0,  // 0 = synchronous mode
                .flags.enable_internal_pullup = true,
            };

            rc = i2c_new_master_bus(&bus_config, &i2c_hal_data[bus].bus_handle);
            if (rc != ESP_OK) {
                return ATCA_COMM_FAIL;
            }

            // Get device address
#ifdef ATCA_ENABLE_DEPRECATED
            i2c_hal_data[bus].device_address = ATCA_IFACECFG_VALUE(cfg, atcai2c.slave_address) >> 1;
#else
            i2c_hal_data[bus].device_address = ATCA_IFACECFG_VALUE(cfg, atcai2c.address) >> 1;
#endif

            // Configure I2C device
            i2c_device_config_t dev_cfg = {
                .dev_addr_length = I2C_ADDR_BIT_LEN_7,
                .device_address = i2c_hal_data[bus].device_address,
                .scl_speed_hz = i2c_hal_data[bus].speed,
                .scl_wait_us = 0,
                .flags.disable_ack_check = true,
            };

            rc = i2c_master_bus_add_device(i2c_hal_data[bus].bus_handle, &dev_cfg, &i2c_hal_data[bus].dev_handle);
            if (rc != ESP_OK) {
                i2c_del_master_bus(i2c_hal_data[bus].bus_handle);
                return ATCA_COMM_FAIL;
            }

            i2c_hal_data[bus].initialized = true;
        } else {
            i2c_hal_data[bus].ref_ct++;
        }

        iface->hal_data = &i2c_hal_data[bus];
    }

    if (ESP_OK == rc) {
        return ATCA_SUCCESS;
    } else {
        return ATCA_COMM_FAIL;
    }
}

/** \brief HAL implementation of I2C post init
 * \param[in] iface  instance
 * \return ATCA_SUCCESS
 */
ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
    return ATCA_SUCCESS;
}

/** \brief HAL implementation of I2C send
 * \param[in] iface         instance
 * \param[in] word_address  device transaction type
 * \param[in] txdata        pointer to space to bytes to send
 * \param[in] txlength      number of bytes to send
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t word_address, uint8_t *txdata, int txlength)
{
    ATCAIfaceCfg *cfg = iface->mIfaceCFG;
    esp_err_t rc;
    ATCAI2CMaster_t *hal_data;

    if (!cfg) {
        return ATCA_BAD_PARAM;
    }

    hal_data = (ATCAI2CMaster_t*)iface->hal_data;
    if (!hal_data || !hal_data->dev_handle || !hal_data->initialized) {
        return ATCA_BAD_PARAM;
    }

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
    // For ESP-IDF v5.4+, use multi-buffer transmit
    i2c_master_transmit_multi_buffer_info_t buffer_info[2] = {
        {.write_buffer = (uint8_t*) &word_address, .buffer_size = 1},
        {.write_buffer = (uint8_t*)txdata, .buffer_size = txlength},
    };

    rc = i2c_master_multi_buffer_transmit(hal_data->dev_handle, buffer_info, 2, 200);
#else
    // For ESP-IDF v5.2 and v5.3, use dynamic allocation for the write buffer
    // Prepare write buffer: word_address + txdata
    size_t write_size = 1;
    if (NULL != txdata && 0u < txlength) {
        write_size += txlength;
    }

    uint8_t *write_buffer = malloc(write_size);
    if (!write_buffer) {
        return ATCA_COMM_FAIL;
    }

    write_buffer[0] = word_address;
    if (NULL != txdata && 0u < txlength) {
        memcpy(write_buffer + 1, txdata, txlength);
    }

    rc = i2c_master_transmit(hal_data->dev_handle, write_buffer, write_size, 200);
    free(write_buffer);
#endif // ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)

    if (rc != ESP_OK) {
        return ATCA_COMM_FAIL;
    } else {
        return ATCA_SUCCESS;
    }
}

/** \brief HAL implementation of I2C receive function
 * \param[in]    iface          Device to interact with.
 * \param[in]    address        Device address
 * \param[out]   rxdata         Data received will be returned here.
 * \param[in,out] rxlength      As input, the size of the rxdata buffer.
 *                              As output, the number of bytes received.
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t address, uint8_t *rxdata, uint16_t *rxlength)
{
    ATCAIfaceCfg *cfg = iface->mIfaceCFG;
    esp_err_t rc;
    ATCA_STATUS status = ATCA_COMM_FAIL;

    if ((NULL == cfg) || (NULL == rxlength) || (NULL == rxdata)) {
        return ATCA_TRACE(ATCA_BAD_PARAM, "NULL pointer encountered");
    }

    ATCAI2CMaster_t *hal_data;

    hal_data = (ATCAI2CMaster_t*)iface->hal_data;
    if (!hal_data || !hal_data->dev_handle || !hal_data->initialized) {
        return ATCA_BAD_PARAM;
    }

    int retries = 6;

    while (retries--) {
        rc = i2c_master_receive(hal_data->dev_handle, rxdata, *rxlength, 200);

        // If we read 0xFF from the I2C device, typically it means that device is currently busy in
        // some operation. We will retry with a small delay.
        // This behaviour is introduced as the delay before a receive command doesnt seem to be handled by the CryptoAuthlib.

        if (*rxlength == 1 && rxdata[0] == 0xFF) {
            // Device is busy/not ready, retry or continue
            // This is normal behavior, not an error
            esp_rom_delay_us(25000);
            continue;
        } else {
            break;
        }
    }

    if (ESP_OK == rc) {
        status = ATCA_SUCCESS;
    }

    return status;
}

/** \brief manages reference count on given bus and releases resource if no more references exist
 * \param[in] hal_data - opaque pointer to hal data structure - known only to the HAL implementation
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_release(void *hal_data)
{
    ATCAI2CMaster_t *hal = (ATCAI2CMaster_t*)hal_data;

    if (hal && --(hal->ref_ct) <= 0) {
        if (hal->dev_handle) {
            i2c_master_bus_rm_device(hal->dev_handle);
            hal->dev_handle = NULL;
        }
        if (hal->bus_handle) {
            i2c_del_master_bus(hal->bus_handle);
            hal->bus_handle = NULL;
        }
        hal->initialized = false;
    }
    return ATCA_SUCCESS;
}

/** \brief Perform control operations for the kit protocol
 * \param[in]     iface          Interface to interact with.
 * \param[in]     option         Control parameter identifier
 * \param[in]     param          Optional pointer to parameter value
 * \param[in]     paramlen       Length of the parameter
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_control(ATCAIface iface, uint8_t option, void* param, size_t paramlen)
{
    (void)param;
    (void)paramlen;

    if (iface && iface->mIfaceCFG) {
        if (ATCA_HAL_CHANGE_BAUD == option) {
            return hal_i2c_change_baud(iface, *(uint32_t*)param);
        } else {
            return ATCA_UNIMPLEMENTED;
        }
    }
    return ATCA_BAD_PARAM;
}

#endif // ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 2, 0) || defined(CONFIG_ATCA_I2C_USE_LEGACY_DRIVER)
