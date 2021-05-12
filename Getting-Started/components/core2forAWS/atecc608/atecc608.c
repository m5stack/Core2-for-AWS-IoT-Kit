#include <stdbool.h>
#include "mbedtls/config.h"

#include "cryptoauthlib.h"
#include "mbedtls/atca_mbedtls_wrap.h"
#include "esp_log.h"

#include "i2c_device.h"
#include "atecc608.h"

/* mbedTLS includes */
#include "mbedtls/platform.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/pk.h"

static const char *TAG = "atecc608";

static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;

static int configure_mbedtls_rng(void) {
    int ret;
    mbedtls_ctr_drbg_init(&ctr_drbg);

    ESP_LOGI(TAG, "Seeding the random number generator...");

    mbedtls_entropy_init(&entropy);
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
        (const unsigned char *) TAG, strlen(TAG));
    if (ret != 0) {
        ESP_LOGI(TAG, "failed  ! mbedtls_ctr_drbg_seed returned %d", ret);
    } else {
        ESP_LOGI(TAG, "ok");
    }
    return ret;
}

static void close_mbedtls_rng(void) {
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
}

// static const uint8_t public_key_x509_header[] = {
//     0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A,
//     0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04
// };

static void handleErr(){
    fflush(stdout);
    close_mbedtls_rng();
}

ATCA_STATUS Atecc608_GetSerialString(char * sn) {
    int ret;
    uint8_t serial[ATCA_SERIAL_NUM_SIZE];
    
    i2c_take_port(ATECC608_I2C_PORT, portMAX_DELAY);    
    ret = atcab_read_serial_number(serial);
    i2c_free_port(ATECC608_I2C_PORT);
    
    if (ret != ATCA_SUCCESS) {
        ESP_LOGI(TAG, "*FAILED* atcab_read_serial_number returned %02x", ret);
        handleErr();
    }

    for (size_t i = 0; i < ATCA_SERIAL_NUM_SIZE; i++)
        sprintf(sn + i * 2, "%02x", serial[i]);
    return ret;
}

ATCA_STATUS Atecc608_Init() {
    int ret = ATCA_SUCCESS;
    bool lock;
    uint8_t buf[ATCA_ECC_CONFIG_SIZE];
    // uint8_t pubkey[ATCA_PUB_KEY_SIZE];
    ret = configure_mbedtls_rng();
    ret = atcab_init(&cfg_ateccx08a_i2c_default);
    ESP_LOGI(TAG, "Initializing ATECC608 secure element");
    if (ret != ATCA_SUCCESS) {
        ESP_LOGI(TAG, "*FAILED* atcab_init returned %02x", ret);
        handleErr();
        return ret;
    }
    else {
        ESP_LOGI(TAG, "ok");
        lock = 0;
        ESP_LOGI(TAG, "Checking data zone lock status...");
        ret = atcab_is_locked(LOCK_ZONE_DATA, &lock);
        if (ret != ATCA_SUCCESS) {
            ESP_LOGI(TAG, "*FAILED* atcab_is_locked returned %02x", ret);
            handleErr();
        }
        else {
            if (lock) {
                ESP_LOGI(TAG, "ok: locked");
            }
            else {
                ESP_LOGE(TAG, "*FAILED* ATECC608 is unlocked. Lock with esp_cryptoauth_utility and try again");
                handleErr();
                return ATCA_NOT_LOCKED;
            }
            ESP_LOGI(TAG, "Get the device info (type)...");
            ret = atcab_info(buf);
            if (ret != ATCA_SUCCESS) {
                ESP_LOGI(TAG, "*FAILED* atcab_info returned %02x", ret);
                handleErr();
            }
            ESP_LOGI(TAG, "ok: %02x %02x", buf[2], buf[3]);
        }
    }

    return ret;
}
