/**
 * @file disp_spi.c
 *
 */

#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "lvgl/lvgl.h"

#include "disp_spi.h"
#include "disp_driver.h"

SemaphoreHandle_t spi_mutex;

static void IRAM_ATTR spi_ready (spi_transaction_t *trans);

static spi_host_device_t spi_host;
static spi_device_handle_t spi;
static volatile uint8_t spi_pending_trans = 0;
static transaction_cb_t chained_post_cb;

static uint8_t tft_used_spi_dma = 0;

#define CONFIG_LV_DISP_SPI_CS   5

void spi_poll() {
    if (!tft_used_spi_dma) {
        return ;
    }

    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .rxlength = 0,
        .tx_data = {0xff}
    };
    spi_device_polling_transmit(spi, &t);
    tft_used_spi_dma = 0;
}

void disp_spi_add_device_config(spi_host_device_t host, spi_device_interface_config_t *devcfg) {
    spi_host=host;
    chained_post_cb=devcfg->post_cb;
    devcfg->post_cb=spi_ready;
    esp_err_t ret=spi_bus_add_device(host, devcfg, &spi);
    assert(ret==ESP_OK);
}

void disp_spi_add_device(spi_host_device_t host) {
    gpio_pad_select_gpio(CONFIG_LV_DISP_SPI_CS);
	gpio_set_direction(CONFIG_LV_DISP_SPI_CS, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(CONFIG_LV_DISP_SPI_CS, GPIO_PULLUP_ONLY);
    gpio_set_level(CONFIG_LV_DISP_SPI_CS, 1);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 40 * 1000 * 1000,
        .mode = 0,
        .spics_io_num=CONFIG_LV_DISP_SPI_CS,              // CS pin
        .input_delay_ns=0,
        .queue_size=1,
        .pre_cb=NULL,
        .post_cb=NULL,
        .flags = SPI_DEVICE_NO_DUMMY,
    };

    disp_spi_add_device_config(host, &devcfg);
}

void disp_spi_transaction(const uint8_t *data, size_t length,
    disp_spi_send_flag_t flags, disp_spi_read_data *out,
    uint64_t addr) {
    if (0 == length) {
        return;
    }

    /* Wait for previous pending transaction results */
    disp_wait_for_pending_transactions();

    spi_transaction_ext_t t = {0};

    /* transaction length is in bits */
    t.base.length = length * 8;

    if (length <= 4 && data != NULL) {
        t.base.flags = SPI_TRANS_USE_TXDATA;
        memcpy(t.base.tx_data, data, length);
    } else {
        t.base.tx_buffer = data;
    }

    if (flags & DISP_SPI_RECEIVE) {
        assert(out != NULL && (flags & (DISP_SPI_SEND_POLLING | DISP_SPI_SEND_SYNCHRONOUS)));
        t.base.rx_buffer = out;
        t.base.rxlength = 0; /* default, same as tx length */
    }

    if (flags & DISP_SPI_ADDRESS_8) {
        t.address_bits = 8;
    } else if (flags & DISP_SPI_ADDRESS_16) {
        t.address_bits = 16;
    } else if (flags & DISP_SPI_ADDRESS_24) {
        t.address_bits = 24;
    } else if (flags & DISP_SPI_ADDRESS_32) {
        t.address_bits = 32;
    }
    if (t.address_bits) {
        t.base.addr = addr;
        t.base.flags |= SPI_TRANS_VARIABLE_ADDR;
    }

    /* Save flags for pre/post transaction processing */
    t.base.user = (void *) flags;

    xSemaphoreTake(spi_mutex, portMAX_DELAY);
    spi_device_acquire_bus(spi, portMAX_DELAY);
    gpio_set_level(CONFIG_LV_DISP_SPI_CS, 0);

    /* Poll/Complete/Queue transaction */
    if (flags & DISP_SPI_SEND_POLLING) {
        spi_device_polling_transmit(spi, (spi_transaction_t *) &t);
        gpio_set_level(CONFIG_LV_DISP_SPI_CS, 1);
        spi_device_release_bus(spi);
        xSemaphoreGive(spi_mutex);
    } else if (flags & DISP_SPI_SEND_SYNCHRONOUS) {
        spi_device_transmit(spi, (spi_transaction_t *) &t);
        gpio_set_level(CONFIG_LV_DISP_SPI_CS, 1);
        spi_device_release_bus(spi);
        xSemaphoreGive(spi_mutex);
    } else {
        static spi_transaction_ext_t queuedt;
        memcpy(&queuedt, &t, sizeof t);
        spi_pending_trans++;
        if (spi_device_queue_trans(spi, (spi_transaction_t *) &queuedt, portMAX_DELAY) != ESP_OK) {
            spi_pending_trans--; /* Clear wait state */
        }
    }
}

void disp_wait_for_pending_transactions(void) {
    spi_transaction_t *presult;

    while (spi_pending_trans) {
        if (spi_device_get_trans_result(spi, &presult, portMAX_DELAY) == ESP_OK) {
            spi_pending_trans--;
        }
    }
}

static void IRAM_ATTR spi_ready(spi_transaction_t *trans) {
    disp_spi_send_flag_t flags = (disp_spi_send_flag_t) trans->user;
    int higher_priority_task_awoken = pdFALSE;

    if (flags & DISP_SPI_SIGNAL_FLUSH) {
        lv_disp_t * disp = NULL;
        disp = _lv_refr_get_disp_refreshing();
        lv_disp_flush_ready(&disp->driver);

    }

    if (chained_post_cb) {
        chained_post_cb(trans);
    }

    if (flags & DISP_SPI_SIGNAL_FLUSH) {
        tft_used_spi_dma = 1;
        gpio_set_level(CONFIG_LV_DISP_SPI_CS, 1);
        spi_device_release_bus(spi);
        xSemaphoreGiveFromISR(spi_mutex, &higher_priority_task_awoken);
    }

    if (higher_priority_task_awoken) portYIELD_FROM_ISR();
}
