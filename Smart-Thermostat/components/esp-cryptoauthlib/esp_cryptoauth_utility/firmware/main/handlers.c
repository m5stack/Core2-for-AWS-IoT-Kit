/*
 * Copyright 2021 Espressif Systems (Shanghai) CO LTD
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
#ifdef CONFIG_ECU_DEBUGGING
#define ECU_DEBUG_LOG ESP_LOGI
#else
#define ECU_DEBUG_LOG(...)
#endif /* MFG_DEBUG */

#include <string.h>
#include "stdio.h"
#include "mbedtls/base64.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_rom_crc.h"
#include "esp_rom_sys.h"
#include "esp_rom_gpio.h"
#include "hal/gpio_ll.h"
#include "soc/gpio_sig_map.h"
#include "esp_partition.h"
#include "esp_flash_partitions.h"
#include "spi_flash_mmap.h"

#include "handlers.h"

/* Cryptoauthlib includes */
#include "cryptoauthlib.h"
#include "cert_def_3_device_csr.h"
#include "cert_def_2_device.h"
#include "cert_def_1_signer.h"
#include "atcacert/atcacert_client.h"
#include "atcacert/atcacert_pem.h"
#include "tng_atcacert_client.h"

#include "mbedtls/atca_mbedtls_wrap.h"
#include "ecu_console_interface.h"

static const char *TAG = "secure_element";
static bool is_atcab_init = false;

static atcacert_def_t g_cert_def_common;
uint8_t *g_cert_template_device;

static const uint8_t public_key_x509_header[] = {
    0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A,
    0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04
};


int convert_pem_to_der( const unsigned char *input, size_t ilen,
                        unsigned char *output, size_t *olen )
{
    int ret;
    const unsigned char *s1, *s2, *end = input + ilen;
    size_t len = 0;

    s1 = (unsigned char *) strstr( (const char *) input, "-----BEGIN" );
    if ( s1 == NULL ) {
        return ( -1 );
    }

    s2 = (unsigned char *) strstr( (const char *) input, "-----END" );
    if ( s2 == NULL ) {
        return ( -1 );
    }

    s1 += 10;
    while ( s1 < end && *s1 != '-' ) {
        s1++;
    }
    while ( s1 < end && *s1 == '-' ) {
        s1++;
    }
    if ( *s1 == '\r' ) {
        s1++;
    }
    if ( *s1 == '\n' ) {
        s1++;
    }

    if ( s2 <= s1 || s2 > end ) {
        return ( -1 );
    }
    ret = mbedtls_base64_decode( NULL, 0, &len, (const unsigned char *) s1, s2 - s1 );
    if ( ret == MBEDTLS_ERR_BASE64_INVALID_CHARACTER ) {
        return ( ret );
    }

    if ( len > *olen ) {
        return ( -1 );
    }
    if ( ( ret = mbedtls_base64_decode( output, len, &len, (const unsigned char *) s1,
                                        s2 - s1 ) ) != 0 ) {
        return ( ret );
    }

    *olen = len;

    return ( 0 );
}

extern void hal_esp32_i2c_set_pin_config(uint8_t i2c_sda_pin, uint8_t i2c_scl_pin);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0) && !defined(CONFIG_ATCA_I2C_USE_LEGACY_DRIVER)
#include "driver/i2c_master.h"
#include "soc/soc_caps.h"

// Static bus handle for probing - initialized once and reused
static i2c_master_bus_handle_t s_i2c_bus_handle = NULL;

// Static variables to track bus initialization
static uint8_t s_initialized_sda_pin = 0;
static uint8_t s_initialized_scl_pin = 0;

// Helper function to initialize I2C bus for probing (if not already initialized)
static esp_err_t ensure_i2c_bus_initialized(uint8_t sda_pin, uint8_t scl_pin)
{
    if (s_i2c_bus_handle != NULL) {
        // If pins changed, reinitialize the bus
        if (s_initialized_sda_pin != sda_pin || s_initialized_scl_pin != scl_pin) {
            i2c_del_master_bus(s_i2c_bus_handle);
            s_i2c_bus_handle = NULL;
        } else {
            return ESP_OK;  // Bus already initialized with correct pins
        }
    }

    // Configure I2C master bus
    i2c_master_bus_config_t bus_config = {
        .i2c_port = cfg_ateccx08a_i2c_default.atcai2c.bus,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_config, &s_i2c_bus_handle);
    if (ret != ESP_OK) {
        return ret;
    }

    s_initialized_sda_pin = sda_pin;
    s_initialized_scl_pin = scl_pin;

    return ESP_OK;
}

#endif

esp_err_t init_atecc608_device(char *device_type, uint8_t i2c_sda_pin, uint8_t i2c_scl_pin)
{
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0) && !defined(CONFIG_ATCA_I2C_USE_LEGACY_DRIVER)
    // Two-phase detection:
    // Phase 1: Wake the ATECC chip by driving SDA low via GPIO. The chip
    //          ignores I2C probes while in sleep mode but wakes when SDA is
    //          held low for at least tWLO (60us).
    // Phase 2: Now that the chip is awake, use i2c_master_probe to quickly
    //          find the correct address, then atcab_init at that address.
    int ret;
    static const uint8_t addresses[] = {0x6A, 0x6C, 0xC0, 0x70};
    static const char *names[] = {"Trust&Go", "TrustFlex", "TrustCustom", "TrustManager"};

    // Phase 1: Wake the ATECC chip by toggling SDA low via GPIO.
    // Must be done BEFORE I2C bus init so we can control the pin directly.
    // Uses ROM + LL (inline) functions to avoid pulling in the GPIO driver.
    esp_rom_gpio_pad_select_gpio(i2c_sda_pin);
    esp_rom_gpio_connect_out_signal(i2c_sda_pin, SIG_GPIO_OUT_IDX, false, false);
    gpio_ll_od_enable(&GPIO, i2c_sda_pin);
    gpio_ll_output_enable(&GPIO, i2c_sda_pin);
    gpio_ll_set_level(&GPIO, i2c_sda_pin, 0);
    esp_rom_delay_us(100);   // tWLO: hold SDA low for 100us (min 60us)
    gpio_ll_set_level(&GPIO, i2c_sda_pin, 1);
    esp_rom_delay_us(1500);  // tWHI: wait 1.5ms for chip to wake
    gpio_ll_output_disable(&GPIO, i2c_sda_pin);
    gpio_ll_od_disable(&GPIO, i2c_sda_pin);

    // Phase 2: Chip is awake, init I2C bus and probe each known address.
    esp_err_t esp_ret = ensure_i2c_bus_initialized(i2c_sda_pin, i2c_scl_pin);
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus for probing");
        return ESP_FAIL;
    }

    uint8_t found_address = 0;
    const char *found_name = NULL;
    for (int i = 0; i < 4; i++) {
        uint16_t probe_address = addresses[i] >> 1;
        if (i2c_master_probe(s_i2c_bus_handle, probe_address, 50) == ESP_OK) {
            found_address = addresses[i];
            found_name = names[i];
            break;
        }
    }

    // Clean up probe bus before atcab_init (HAL creates its own)
    if (s_i2c_bus_handle != NULL) {
        i2c_del_master_bus(s_i2c_bus_handle);
        s_i2c_bus_handle = NULL;
        s_initialized_sda_pin = 0;
        s_initialized_scl_pin = 0;
    }

    if (found_name == NULL) {
        ESP_LOGE(TAG, "No ATECC device found at any known address");
        return ESP_FAIL;
    }

    // Initialize at the detected address
    cfg_ateccx08a_i2c_default.atcai2c.address = found_address;
    ret = atcab_init(&cfg_ateccx08a_i2c_default);
    if (ret == ATCA_SUCCESS) {
        ESP_LOGI(TAG, "Device is of type %s", found_name);
        sprintf(device_type, "%s", found_name);
        return ESP_OK;
    }
    atcab_release();
#else
    // Fallback to original method for legacy driver or older ESP-IDF versions
    int ret;
    cfg_ateccx08a_i2c_default.atcai2c.address = 0xC0;
    ret = atcab_init(&cfg_ateccx08a_i2c_default);
    if (ret == ATCA_SUCCESS) {
        ESP_LOGI(TAG, "Device is of type TrustCustom");
        sprintf(device_type, "%s", "TrustCustom");
        return ESP_OK;
    }
    atcab_release();

    cfg_ateccx08a_i2c_default.atcai2c.address = 0x6A;
    ret = atcab_init(&cfg_ateccx08a_i2c_default);
    if (ret == ATCA_SUCCESS) {
        ESP_LOGI(TAG, "Device is of type TrustnGo");
        sprintf(device_type, "%s", "Trust&Go");
        return ESP_OK;
    }
    atcab_release();

    cfg_ateccx08a_i2c_default.atcai2c.address = 0x6C;
    ret = atcab_init(&cfg_ateccx08a_i2c_default);
    if (ret == ATCA_SUCCESS) {
        ESP_LOGI(TAG, "Device is of type TrusFlex");
        sprintf(device_type, "%s", "TrustFlex");
        return ESP_OK;
    }
    atcab_release();

    cfg_ateccx08a_i2c_default.atcai2c.address = 0x70;
    ret = atcab_init(&cfg_ateccx08a_i2c_default);
    if (ret == ATCA_SUCCESS) {
        ESP_LOGI(TAG, "Device is of type TrustManager");
        sprintf(device_type, "%s", "TrustManager");
        return ESP_OK;
    }
    atcab_release();
#endif

    return ESP_FAIL;
}

esp_err_t init_atecc608a(char *device_type, uint8_t i2c_sda_pin, uint8_t i2c_scl_pin, int *err_ret)
{
    int ret = 0;
    bool is_zone_locked = false;
    ECU_DEBUG_LOG(TAG, "Initialize the ATECC interface...");
    hal_esp32_i2c_set_pin_config(i2c_sda_pin,i2c_scl_pin);
    ESP_LOGI(TAG, "I2C pins selected are SDA = %d, SCL = %d", i2c_sda_pin, i2c_scl_pin);

    esp_err_t esp_ret = init_atecc608_device(device_type, i2c_sda_pin, i2c_scl_pin);
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize atca device");
    }

    ECU_DEBUG_LOG(TAG, "\t\t OK");

    if (ATCA_SUCCESS != (ret = atcab_is_locked(LOCK_ZONE_CONFIG, &is_zone_locked))) {
        ESP_LOGE(TAG, " failed\n  ! atcab_is_locked returned %02x", ret);
        goto exit;
    }

    if (!is_zone_locked) {
        ret = atcab_lock_config_zone();
        if (ret != ATCA_SUCCESS) {
            ESP_LOGE(TAG, "error in locking config zone, ret = %02x", ret);
            goto exit;
        }
        ECU_DEBUG_LOG(TAG, "success in locking config zone");
    } else {
        ECU_DEBUG_LOG(TAG, "config zone is Locked ..\tOK");
    }

    is_zone_locked = false;

    if (ATCA_SUCCESS != (ret = atcab_is_locked(LOCK_ZONE_DATA, &is_zone_locked))) {
        ESP_LOGE(TAG, " failed\n  ! atcab_is_locked returned %02x", ret);
        goto exit;
    }

    if (!is_zone_locked) {
        ret = atcab_lock_data_zone();
        if (ret != ATCA_SUCCESS) {
            ESP_LOGE(TAG, "\nerror in locking data zone, ret = %02x", ret);
            goto exit;
        }
        ECU_DEBUG_LOG(TAG, "success in locking data zone");
    } else {
        ECU_DEBUG_LOG(TAG, "data zone is Locked ..\tOK");
    }
    is_atcab_init = true;
    *err_ret = ret;
    return ESP_OK;
exit:
    *err_ret = ret;
    return ESP_FAIL;
}

esp_err_t atecc_print_info(uint8_t *serial_no, int *err_ret)
{
    uint8_t rev_info[4] = {};
    int ret = -1;
    if (ATCA_SUCCESS != (ret = atcab_info(rev_info))) {
        ESP_LOGE(TAG, "Error in reading revision information, ret is %02x", ret);
        goto exit;
    }
    ESP_LOG_BUFFER_HEX("ATECC CHIP REVISION", rev_info, 4);
    if (rev_info[3] == 0x03) {
        ESP_LOGI(TAG, "Since the last byte of chip revision is 0x03. This is an ATECC608B chip");
    } else if (rev_info[3] == 0x02) {
        ESP_LOGI(TAG, "Since the last byte of chip revision is 0x02. This is a ATECC608A chip");
    }

    if (ATCA_SUCCESS != (ret = atcab_read_serial_number(serial_no))) {
        ESP_LOGE(TAG, "Error in reading serial number, ret is %02x", ret);
        goto exit;
    }
    ESP_LOG_BUFFER_HEX("ATECC CHIP SERIAL NUMBER", serial_no, 9);
    *err_ret = ret;
    return ESP_OK;
exit:
    *err_ret = ret;
    return ESP_FAIL;
}

static void print_public_key(uint8_t pubkey[ATCA_PUB_KEY_SIZE])
{
    uint8_t buf[128];
    uint8_t *tmp;
    size_t buf_len = sizeof(buf);

    /* Calculate where the raw data will fit into the buffer */
    tmp = buf + sizeof(buf) - ATCA_PUB_KEY_SIZE - sizeof(public_key_x509_header);

    /* Copy the header */
    memcpy(tmp, public_key_x509_header, sizeof(public_key_x509_header));

    /* Copy the key bytes */
    memcpy(tmp + sizeof(public_key_x509_header), pubkey, ATCA_PUB_KEY_SIZE);

    /* Convert to base 64 */
    (void)atcab_base64encode(tmp, ATCA_PUB_KEY_SIZE + sizeof(public_key_x509_header), (char *)buf, &buf_len);

    /* Add a null terminator */
    buf[buf_len] = 0;

    /* Print out the key */
    ECU_DEBUG_LOG(TAG, "\r\n-----BEGIN PUBLIC KEY-----\r\n%s\r\n-----END PUBLIC KEY-----\r\n", buf);
}

esp_err_t atecc_keygen(int slot, unsigned char *pub_key_buf, int pub_key_buf_len, int *err_ret)
{
    int ret = 0;
    bzero(pub_key_buf, pub_key_buf_len);
    if (!is_atcab_init) {
        ESP_LOGE(TAG, "gevice is not initialized");
        goto exit;
    }
    ECU_DEBUG_LOG(TAG, "generating priv key ..");

    if (ATCA_SUCCESS != (ret = atcab_genkey(slot, pub_key_buf))) {
        ESP_LOGE(TAG, "failed\n !atcab_genkey returned -0x%02x", -ret);
        goto exit;
    }
    ECU_DEBUG_LOG(TAG, "\t\t OK");
    print_public_key(pub_key_buf);
    *err_ret = ret;
    return ESP_OK;

exit:
    ESP_LOGE(TAG, "failure in generating Key");
    *err_ret = ret;
    return ESP_FAIL;
}

esp_err_t atecc_gen_pubkey(int slot, unsigned char *pub_key_buf, int pub_key_buf_len, int *err_ret)
{
    int ret = -1;
    if (!is_atcab_init) {
        ESP_LOGE(TAG, "\ndevice is not initialized");
        goto exit;
    }
    bzero(pub_key_buf, pub_key_buf_len);
    ECU_DEBUG_LOG(TAG, "Get the public key...");
    if (0 != (ret = atcab_get_pubkey(slot, pub_key_buf))) {
        ESP_LOGE(TAG, " failed\n  ! atcab_get_pubkey returned %02x", ret);
        goto exit;
    }
    ECU_DEBUG_LOG("\t\t OK\n");
    print_public_key(pub_key_buf);
    *err_ret = ret;
    return ESP_OK;

exit:
    *err_ret = ret;
    ESP_LOGE(TAG, "\ngenerate public key failed");
    return ESP_FAIL;
}

esp_err_t atecc_csr_gen(unsigned char *csr_buf, size_t csr_buf_len, int *err_ret)
{
    int ret = 0;
    if (!is_atcab_init) {
        ESP_LOGE(TAG, "device is not initialized");
        goto exit;
    }
    bzero(csr_buf, csr_buf_len);
    ECU_DEBUG_LOG(TAG, "generating csr ..");
    ret = atcacert_create_csr_pem(&g_csr_def_3_device, (char *)csr_buf, &csr_buf_len);
    if (ret != ATCA_SUCCESS) {
        ESP_LOGE(TAG, "create csr pem failed, returned %02x", ret);
        goto exit;
    }
    ECU_DEBUG_LOG(TAG, "\t\t OK");
    *err_ret = ret;
    return ESP_OK;

exit:
    ESP_LOGE(TAG, "Failure, Exiting , ret is %02x", ret);
    *err_ret = ret;
    return ESP_FAIL;

}

esp_err_t get_cert_def(unsigned char *cert_def_array, size_t data_len, cert_type_t cert_type)
{
    if (cert_type == CERT_TYPE_DEVICE) {
        g_cert_def_common = g_cert_def_2_device;
    } else if (cert_type == CERT_TYPE_SIGNER) {
        g_cert_def_common = g_cert_def_1_signer;
    }
    ecu_console_interface_t *console_interface = get_console_interface();
    if (console_interface == NULL) {
        ESP_LOGE(TAG, "Console interface is NULL");
        return ESP_FAIL;
    }
    int i = 0;
    memset(cert_def_array, 0xff, data_len);
    do {
        esp_err_t ret;
        ret = console_interface->read_bytes((uint8_t *) &cert_def_array[i], 1, portMAX_DELAY);
        if (ret > 0) {
            if (cert_def_array[i] == '\0') {
                break;
            }
            i++;
        }
    } while (i < data_len - 1 && cert_def_array[i] != '\0');

    char str[4] = {};
    int count = 0;
    /* converting the offsets and counts to int, 4 bytes at a time */
    for (count = 0 ; count < 8; count++) {
        memcpy(str, &cert_def_array[4 * ((2 * count) + 0)], 4);
        g_cert_def_common.std_cert_elements[count].offset = (uint16_t)atoi(str);
        memcpy(str, &cert_def_array[4 * ((2 * count) + 1)], 4);
        g_cert_def_common.std_cert_elements[count].count = (uint16_t)atoi(str);
    }

    memcpy(str, &cert_def_array[4 * ((2 * count) + 0)], 4);
    g_cert_def_common.tbs_cert_loc.offset = (uint16_t)atoi(str);

    memcpy(str, &cert_def_array[4 * ((2 * count) + 1)], 4);
    g_cert_def_common.tbs_cert_loc.count = (uint16_t)atoi(str);

    count = count + 1;
    /* converting to total number of bytes used */
    count = count * 8;
    int template_size = ((strlen((const char *)&cert_def_array[0]) - count ) / 2);
    int pos = 0;
    char temp[2];
    g_cert_template_device = (uint8_t *)calloc(template_size, sizeof(uint8_t));
    /* Converting the templates from string to hex, 2 bytes at a time */
    for (int i = 0; i < template_size; i++) {
        memcpy(temp, &cert_def_array[count], 2);
        g_cert_template_device[pos] = strtol((const char *)temp, NULL, 16);
        pos ++;
        count = count + 2;
    }
    atcacert_cert_element_t *cert_element;

    cert_element = (atcacert_cert_element_t *)calloc(2, sizeof(atcacert_cert_element_t));

    if (cert_type == CERT_TYPE_SIGNER) {
        cert_element[0].device_loc.offset = 35 - g_cert_def_common.std_cert_elements[STDCERT_ISSUE_DATE].offset;
        cert_element[0].device_loc.count = g_cert_def_common.std_cert_elements[STDCERT_ISSUE_DATE].count;
        cert_element[0].cert_loc.offset = g_cert_def_common.std_cert_elements[STDCERT_ISSUE_DATE].offset;
        cert_element[0].cert_loc.count = g_cert_def_common.std_cert_elements[STDCERT_ISSUE_DATE].count;

        cert_element[1].device_loc.offset = 50 - g_cert_def_common.std_cert_elements[STDCERT_EXPIRE_DATE].offset;
        cert_element[1].device_loc.count = g_cert_def_common.std_cert_elements[STDCERT_EXPIRE_DATE].count;
        cert_element[1].cert_loc.offset = g_cert_def_common.std_cert_elements[STDCERT_EXPIRE_DATE].offset;
        cert_element[1].cert_loc.count = g_cert_def_common.std_cert_elements[STDCERT_EXPIRE_DATE].count;

        g_cert_def_common.cert_elements = cert_element;
        g_cert_def_common.cert_elements_count = 2;
    }

    g_cert_def_common.cert_template = g_cert_template_device;
    g_cert_def_common.cert_template_size = template_size;

    return ESP_OK;
}

#define ATECC608A_DEVICE_CERT_SLOT 10
#define ATECC608A_SIGNER_CERT_SLOT 12
esp_err_t atecc_input_cert(unsigned char *cert_buf, size_t cert_len, cert_type_t cert_type, bool lock, int *err_ret, uint32_t expected_crc)
{
    int ret = -1;

    if (!is_atcab_init) {
        ESP_LOGE(TAG, "\ndevice is not initialized");
        goto exit;
    }
    int i = 0;
    ecu_console_interface_t *console_interface = get_console_interface();
    if (console_interface == NULL) {
        ESP_LOGE(TAG, "Console interface is NULL");
        return ESP_FAIL;
    }
    memset(cert_buf, 0xff, cert_len);
    do {
        esp_err_t esp_ret;
        esp_ret = console_interface->read_bytes((uint8_t *) &cert_buf[i], 1, portMAX_DELAY);
        if (esp_ret > 0) {
            if (cert_buf[i] == '\0') {
                break;
            }
            i++;
        }
    } while (i < cert_len - 1 && cert_buf[i] != '\0');

    // Compute CRC32
    uint32_t calculated_crc = esp_rom_crc32_le(0, cert_buf, i);
    if (calculated_crc != expected_crc) {
        ESP_LOGE(TAG, "CRC32 mismatch! Expected: %ld, Received: %ld", expected_crc, calculated_crc);
        return ESP_ERR_INVALID_CRC;
    }

    uint8_t der_cert[800];
    size_t der_cert_size = 800;
    if (convert_pem_to_der(cert_buf, cert_len, (unsigned char *)der_cert, &der_cert_size) != 0) {
        ESP_LOGE(TAG, "error in converting to der");
        return ESP_FAIL;
    }

    der_cert[der_cert_size] = 0;
    der_cert_size += 1;

    if (cert_type == CERT_TYPE_DEVICE) {
        ECU_DEBUG_LOG(TAG, "writing device cert ..");
        if (ATCA_SUCCESS != (ret = atcacert_write_cert((const atcacert_def_t *)&g_cert_def_common, der_cert, der_cert_size + 1))) {
            ESP_LOGE(TAG, "writecert failed , ret is %02x\nPlease make sure that the device cert slot is not locked", ret);
            goto exit;
        }
        if (lock) {
            ret = atcab_lock_data_slot(ATECC608A_DEVICE_CERT_SLOT);
            if (ret != ATCA_SUCCESS) {
                ESP_LOGE(TAG, "Failed to lock slot 10 (device certificate)\nThis action is supposed to fail if the slot is already locked");
                goto exit;
            }
            ESP_LOGI(TAG, "Slot %d has been locked, and cannot be used again", ATECC608A_DEVICE_CERT_SLOT);
        }

    } else if (cert_type == CERT_TYPE_SIGNER) {
        ECU_DEBUG_LOG(TAG, "writing signer cert ..");
        if (ATCA_SUCCESS != (ret = atcacert_write_cert(&g_cert_def_common, der_cert, der_cert_size + 1))) {
            ESP_LOGE(TAG, "writecert failed , ret is %02x\nPlease make sure that the signer cert slot is not locked", ret);
            goto exit;
        }
        if (lock) {
            ret = atcab_lock_data_slot(ATECC608A_SIGNER_CERT_SLOT);
            if (ret != ATCA_SUCCESS) {
                ESP_LOGE(TAG, "Failed to lock slot 12 (signer certificate),\nThis action is supposed to fail if the slot is already locked");
                goto exit;
            }
            ESP_LOGI(TAG, "Slot %d has been locked, and cannot be used again", ATECC608A_DEVICE_CERT_SLOT);
        }
    } else {
        ESP_LOGE(TAG, "wrong cert type");
        goto exit;
    }
    ECU_DEBUG_LOG(TAG, "\t\t OK");
    *err_ret = ret;
    return ESP_OK;
exit:
    ESP_LOGE(TAG, "failure, exiting");
    *err_ret = ret;
    return ESP_FAIL;
}

esp_err_t atecc_get_tngtls_root_cert(unsigned char *cert_buf, size_t *cert_len, int *err_ret)
{
    int ret;
    ECU_DEBUG_LOG(TAG, "atecc_get_tngtls_root_cert start");
    if (ATCA_SUCCESS != (ret = tng_atcacert_root_cert_size(cert_len))) {
        ESP_LOGE(TAG, "failed to get tng_atcacert_root_cert_size, returned 0x%02x", ret);
        goto exit;
    }
    if (ATCA_SUCCESS != (ret = tng_atcacert_root_cert(cert_buf, cert_len))) {
        ESP_LOGE(TAG, "failed to read tng_atcacert_root_cert, returned 0x%02x", ret);
        goto exit;
    }
    ECU_DEBUG_LOG(TAG, "atecc_get_tngtls_root_cert end");
    *err_ret = ret;
    return ESP_OK;

exit:
    *err_ret = ret;
    return ESP_FAIL;
}

esp_err_t atecc_get_tngtls_signer_cert(unsigned char *cert_buf, size_t *cert_len, int *err_ret)
{
    int ret;
    ECU_DEBUG_LOG(TAG, "atecc_get_tngtls_signer_cert start");
    if (ATCA_SUCCESS != (ret = tng_atcacert_max_signer_cert_size(cert_len))) {
        ESP_LOGE(TAG, "failed to get tng_atcacert_signer_cert_size, returned 0x%02x", ret);
        goto exit;
    }
    if (ATCA_SUCCESS != (ret = tng_atcacert_read_signer_cert(cert_buf, cert_len))) {
        ESP_LOGE(TAG, "failed to read tng_atcacert_signer_cert, returned 0x%02x", ret);
        goto exit;
    }
    ECU_DEBUG_LOG(TAG, "atecc_get_tngtls_signer_cert end");
    *err_ret = ret;
    return ESP_OK;

exit:
    *err_ret = ret;
    return ESP_FAIL;
}

esp_err_t atecc_get_tngtls_device_cert(unsigned char *cert_buf, size_t *cert_len, int *err_ret)
{
    int ret;
    ECU_DEBUG_LOG(TAG, "atecc_get_tngtls_signer_cert start");
    if (ATCA_SUCCESS != (ret = tng_atcacert_max_device_cert_size(cert_len))) {
        ESP_LOGE(TAG, "Failed to get tng_atcacert_device_cert_size, returned 0x%02x", ret);
        goto exit;
    }
    if (ATCA_SUCCESS != (ret = tng_atcacert_read_device_cert(cert_buf, cert_len, NULL))) {
        ESP_LOGE(TAG, "failed to read tng_atcacert_device_cert, returned 0x%02x", ret);
        goto exit;
    }
    ECU_DEBUG_LOG(TAG, "atecc_get_tngtls_signer_cert end");

    *err_ret = ret;
    return ESP_OK;

exit:
    *err_ret = ret;
    return ESP_FAIL;
}
