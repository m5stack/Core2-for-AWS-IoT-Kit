/**
 * @file disp_spi.h
 *
 */

#ifndef DISP_SPI_H
#define DISP_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <driver/spi_master.h>

typedef enum _disp_spi_send_flag_t {
    DISP_SPI_SEND_QUEUED        = 0x00000000,
    DISP_SPI_SEND_POLLING       = 0x00000001,
    DISP_SPI_SEND_SYNCHRONOUS   = 0x00000002,
    DISP_SPI_SIGNAL_FLUSH       = 0x00000004,
    DISP_SPI_RECEIVE            = 0x00000008,
    DISP_SPI_CMD_8              = 0x00000010, /* Reserved */
    DISP_SPI_CMD_16             = 0x00000020, /* Reserved */
    DISP_SPI_ADDRESS_8          = 0x00000040, /* Reserved */
    DISP_SPI_ADDRESS_16         = 0x00000080, /* Reserved */
    DISP_SPI_ADDRESS_24         = 0x00000100, /* Reserved */
    DISP_SPI_ADDRESS_32         = 0x00000200, /* Reserved */
    DISP_SPI_MODE_DIO           = 0x00000400, /* Reserved */
    DISP_SPI_MODE_QIO           = 0x00000800, /* Reserved */
    DISP_SPI_MODE_DIOQIO_ADDR   = 0x00001000, /* Reserved */
} disp_spi_send_flag_t;

typedef struct _disp_spi_read_data {
    uint8_t _dummy_byte;
    union {
        uint8_t byte;
        uint16_t word;
        uint32_t dword;
    } __attribute__((packed));
} disp_spi_read_data __attribute__((aligned(4)));

/**
 * @brief Semaphore for the shared SPI bus.
 * 
 * This semaphore helps protect the usage of the SPI bus. The implementation of the 
 * ILI9342C library uses this mutex to write to the display controller.
 */
/* @[declare_spi_mutex] */
extern SemaphoreHandle_t spi_mutex;
/* @[declare_spi_mutex] */

void disp_spi_add_device(spi_host_device_t host);
void disp_spi_add_device_config(spi_host_device_t host, spi_device_interface_config_t *devcfg);
void disp_spi_transaction(const uint8_t *data, size_t length,
    disp_spi_send_flag_t flags, disp_spi_read_data *out, uint64_t addr);
void disp_wait_for_pending_transactions(void);

static inline void disp_spi_send_data(uint8_t *data, size_t length) {
    disp_spi_transaction(data, length, DISP_SPI_SEND_POLLING, NULL, 0);
}

static inline void disp_spi_send_colors(uint8_t *data, size_t length) {
    disp_spi_transaction(data, length,
        DISP_SPI_SEND_QUEUED | DISP_SPI_SIGNAL_FLUSH,
        NULL, 0);
}

/**
 * @brief Polls the SPI bus for a pending SPI transaction to complete.
 * 
 * This function blocks until a pending SPI transaction has finished.
 * Once that transaction is complete, execution of code continues.
 * Useful for using more than one peripheral on the same SPI bus without
 * conflicts.
 * 
 * @note This function is necessary when accessing the SD card on the shared SPI bus. 
 * Uses [spi_device_polling_transmit](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/peripherals/spi_master.html#_CPPv427spi_device_polling_transmit19spi_device_handle_tP17spi_transaction_t).
 * to repeatedly poll the SPI bus until the transaction completes.
 */
/* @[declare_spi_poll] */
void spi_poll();
/* @[declare_spi_poll] */


/* @brief Receive data helpers */
#define member_size(type, member)   sizeof(((type *)0)->member)

#define SPI_READ_DUMMY_LEN  member_size(disp_spi_read_data, _dummy_byte)
#define SPI_READ_BYTE_LEN   (SPI_READ_DUMMY_LEN + member_size(disp_spi_read_data, byte))
#define SPI_READ_WORD_LEN   (SPI_READ_DUMMY_LEN + member_size(disp_spi_read_data, word))
#define SPI_READ_DWORD_LEN   (SPI_READ_DUMMY_LEN + member_size(disp_spi_read_data, dword))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DISP_SPI_H*/
