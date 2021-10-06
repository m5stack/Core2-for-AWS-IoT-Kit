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
#include "esp_err.h"
#include "esp_log.h"
#include "cryptoauthlib.h"
#include "i2c_device.h"

#define I2C1_SDA_PIN                       CONFIG_ATCA_I2C_SDA_PIN
#define I2C1_SCL_PIN                       CONFIG_ATCA_I2C_SCL_PIN
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL                    ESP_LOG_INFO
#endif

#define MAX_I2C_BUSES 2  //ESP32 has 2 I2C bus

I2CDevice_t atecc608_device = NULL;

const char* TAG = "HAL_I2C";

ATCA_STATUS status;

/** \brief method to change the bus speec of I2C
 * \param[in] iface  interface on which to change bus speed
 * \param[in] speed  baud rate (typically 100000 or 400000)
 */
ATCA_STATUS hal_i2c_change_baud(ATCAIface iface, uint32_t speed)
{
    esp_err_t rc;
    rc = i2c_device_change_freq(atecc608_device, speed);
    if (rc == ESP_OK)
    {
        //ESP_LOGD(TAG, "Baudrate Changed");
        return ATCA_SUCCESS;
    }
    else
    {
        //ESP_LOGW(TAG, "Baudrate Change Failed");
        return ATCA_COMM_FAIL;
    }
}

/** \brief
    - this HAL implementation assumes you've included the START Twi libraries in your project, otherwise,
    the HAL layer will not compile because the START TWI drivers are a dependency *
 */

/** \brief hal_i2c_init manages requests to initialize a physical interface.  it manages use counts so when an interface
 * has released the physical layer, it will disable the interface for some other use.
 * You can have multiple ATCAIFace instances using the same bus, and you can have multiple ATCAIFace instances on
 * multiple i2c buses, so hal_i2c_init manages these things and ATCAIFace is abstracted from the physical details.
 */

/** \brief initialize an I2C interface using given config
 * \param[in] hal - opaque ptr to HAL data
 * \param[in] cfg - interface configuration
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_init(ATCAIface iface, ATCAIfaceCfg *cfg)
{
    int bus = cfg->atcai2c.bus;
    atecc608_device = i2c_malloc_device(bus, I2C1_SDA_PIN, I2C1_SCL_PIN, cfg->atcai2c.baud, cfg->atcai2c.address >> 1);

    if (atecc608_device == NULL) {
        return ATCA_COMM_FAIL;
    } else {
        return ATCA_SUCCESS;
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
ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t address, uint8_t *txdata, int txlength)
{
    ATCAIfaceCfg *cfg = iface->mIfaceCFG;
    esp_err_t rc;

    if (!cfg)
    {
        return ATCA_BAD_PARAM;
    }

    //ESP_LOGD(TAG, "txdata: %p , txlength: %d", txdata, txlength);
    //ESP_LOG_BUFFER_HEXDUMP(TAG, txdata, txlength, 3);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    (void)i2c_master_start(cmd);
    (void)i2c_master_write_byte(cmd, address | I2C_MASTER_WRITE, ACK_CHECK_EN);
    (void)i2c_master_write(cmd, txdata, txlength, ACK_CHECK_EN);
    (void)i2c_master_stop(cmd);
    i2c_apply_bus(atecc608_device);
    rc = i2c_master_cmd_begin(cfg->atcai2c.bus, cmd, 10);
    i2c_free_bus(atecc608_device);
    (void)i2c_cmd_link_delete(cmd);

    if (ESP_OK != rc)
    {
        return ATCA_COMM_FAIL;
    }
    else
    {
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
 
    if ((NULL == cfg) || (NULL == rxlength) || (NULL == rxdata))
    {
        return ATCA_TRACE(ATCA_INVALID_POINTER, "NULL pointer encountered");
    }

    cmd = i2c_cmd_link_create();
    (void)i2c_master_start(cmd);
    (void)i2c_master_write_byte(cmd, address | I2C_MASTER_READ, ACK_CHECK_EN);
    if (*rxlength > 1)
    {
        (void)i2c_master_read(cmd, rxdata, *rxlength - 1, ACK_VAL);
    }
    (void)i2c_master_read_byte(cmd, rxdata + (size_t)*rxlength - 1, NACK_VAL);
    (void)i2c_master_stop(cmd);
    i2c_apply_bus(atecc608_device);
    rc = i2c_master_cmd_begin(cfg->atcai2c.bus, cmd, 10);
    i2c_free_bus(atecc608_device);
    (void)i2c_cmd_link_delete(cmd);

    //ESP_LOG_BUFFER_HEXDUMP(TAG, rxdata, *rxlength, 3);

    if (ESP_OK == rc)
    {
        status = ATCA_SUCCESS;
    }

    return status;

}

/** \brief manages reference count on given bus and releases resource if no more refences exist
 * \param[in] hal_data - opaque pointer to hal data structure - known only to the HAL implementation
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_release(void *hal_data)
{
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

    if (iface && iface->mIfaceCFG)
    {
        if (ATCA_HAL_CHANGE_BAUD == option)
        {
            return hal_i2c_change_baud(iface, *(uint32_t*)param);
        }
        else
        {
            return ATCA_UNIMPLEMENTED;
        }
    }
    return ATCA_BAD_PARAM;
}
